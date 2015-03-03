#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QStringList>
#include <QDebug>
#include <xmlconfigreader.h>
#include <QCoreApplication>
#include <protonetserver.h>
#include <unistd.h>
#include <fcntl.h>

#include "sec1000dglobal.h"
#include "sec1000d.h"
#include "pcbserver.h"
#include "debugsettings.h"
#include "ethsettings.h"
#include "fpgasettings.h"
#include "ecalcsettings.h"
#include "statusinterface.h"
#include "systeminterface.h"
#include "systeminfo.h"
#include "ecalcinterface.h"
#include "rmconnection.h"


cSEC1000dServer::cSEC1000dServer(QObject *parent)
    :cPCBServer(parent)
{

    m_pDebugSettings = 0;
    m_pETHSettings = 0;
    m_pFPGAsettings = 0;
    m_pECalcSettings = 0;
    m_pStatusInterface = 0;
    m_pSystemInterface = 0;
    m_pECalculatorInterface = 0;
    m_pSystemInfo = 0;
    m_pRMConnection = 0;

    m_pInitializationMachine = new QStateMachine(this);

    QState* stateCONF = new QState(); // we start from here
    QFinalState* stateFINISH = new QFinalState(); // and here we finish

    stateCONF->addTransition(this, SIGNAL(abortInit()),stateFINISH); // from anywhere we arrive here if some error

    QState* statexmlConfiguration = new QState(stateCONF); // we configure our server with xml file
    QState* statesetupServer = new QState(stateCONF); // we setup our server now
    stateconnect2RM = new QState(stateCONF); // we connect to resource manager
    stateconnect2RMError = new QState(stateCONF);
    stateSendRMIdentandRegister = new QState(stateCONF); // we send ident. to rm and register our resources
    stateCONF->setInitialState(statexmlConfiguration);

    statexmlConfiguration->addTransition(myXMLConfigReader, SIGNAL(finishedParsingXML(bool)), statesetupServer);
    statesetupServer->addTransition(this, SIGNAL(serverSetup()), stateconnect2RM);
    m_pInitializationMachine->addState(stateCONF);
    m_pInitializationMachine->addState(stateFINISH);
    m_pInitializationMachine->setInitialState(stateCONF);

    QObject::connect(statexmlConfiguration, SIGNAL(entered()), this, SLOT(doConfiguration()));
    QObject::connect(statesetupServer, SIGNAL(entered()), this, SLOT(doSetupServer()));
    QObject::connect(stateconnect2RM, SIGNAL(entered()), this, SLOT(doConnect2RM()));
    QObject::connect(stateconnect2RMError, SIGNAL(entered()), this, SLOT(connect2RMError()));
    QObject::connect(stateSendRMIdentandRegister, SIGNAL(entered()), this, SLOT(doIdentAndRegister()));
    QObject::connect(stateFINISH, SIGNAL(entered()), this, SLOT(doCloseServer()));

    m_pInitializationMachine->start();
}


cSEC1000dServer::~cSEC1000dServer()
{
    if (m_pDebugSettings) delete m_pDebugSettings;
    if (m_pETHSettings) delete m_pETHSettings;
    if (m_pFPGAsettings) delete m_pFPGAsettings;
    if (m_pECalcSettings) delete m_pECalcSettings;
    if (m_pStatusInterface) delete m_pStatusInterface;
    if (m_pSystemInterface) delete m_pSystemInterface;
    if (m_pECalculatorInterface) delete m_pECalculatorInterface;
    if (m_pSystemInfo) delete m_pSystemInfo;
    if (m_pRMConnection) delete m_pRMConnection;
}


void cSEC1000dServer::doConfiguration()
{
    QStringList args;

    args = QCoreApplication::instance()->arguments();
    if (args.count() != 2) // we want exactly 1 parameter
    {
        m_nerror = parameterError;
        emit abortInit();
    }
    else
    {

        if (myXMLConfigReader->loadSchema(defaultXSDFile))
        {
            // we want to initialize all settings first
            m_pDebugSettings = new cDebugSettings(myXMLConfigReader);
            connect(myXMLConfigReader,SIGNAL(valueChanged(const QString&)),m_pDebugSettings,SLOT(configXMLInfo(const QString&)));
            m_pETHSettings = new cETHSettings(myXMLConfigReader);
            connect(myXMLConfigReader,SIGNAL(valueChanged(const QString&)),m_pETHSettings,SLOT(configXMLInfo(const QString&)));
            m_pFPGAsettings = new cFPGASettings(myXMLConfigReader);
            connect(myXMLConfigReader,SIGNAL(valueChanged(const QString&)),m_pFPGAsettings,SLOT(configXMLInfo(const QString&)));
            m_pECalcSettings = new cECalculatorSettings(myXMLConfigReader);
            connect(myXMLConfigReader,SIGNAL(valueChanged(const QString&)),m_pECalcSettings,SLOT(configXMLInfo(const QString&)));

            QString s = args.at(1);
            qDebug() << s;

            if (myXMLConfigReader->loadXML(s)) // the first parameter should be the filename
            {
                // xmlfile ok -> nothing to do .. the configreader will emit all configuration
                // signals and after this the finishedparsingXML signal
            }
            else
            {
                m_nerror = xmlfileError;
                emit abortInit();
            }
        }
        else
        {
            m_nerror = xsdfileError;
            emit abortInit();
        }

    }
}


void cSEC1000dServer::doSetupServer()
{
    m_pSystemInfo = new cSystemInfo();

    cPCBServer::setupServer(); // here our scpi interface gets instanciated, we need this for further steps

    scpiConnectionList.append(this); // the server itself has some commands
    scpiConnectionList.append(m_pStatusInterface = new cStatusInterface());
    scpiConnectionList.append(m_pSystemInterface = new cSystemInterface(this, m_pSystemInfo));
    scpiConnectionList.append(m_pECalculatorInterface = new cECalculatorInterface(this, m_pETHSettings, m_pECalcSettings, m_pFPGAsettings));

    resourceList.append(m_pECalculatorInterface); // all our resources

    initSCPIConnections();

    myServer->startServer(m_pETHSettings->getPort(server)); // and can start the server now

    // our resource mananager connection must be opened after configuration is done
    m_pRMConnection = new cRMConnection(m_pETHSettings->getRMIPadr(), m_pETHSettings->getPort(resourcemanager), m_pDebugSettings->getDebugLevel());
    //connect(m_pRMConnection, SIGNAL(connectionRMError()), this, SIGNAL(abortInit()));
    // so we must complete our state machine here
    m_nRetryRMConnect = 100;
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, SIGNAL(timeout()), this, SIGNAL(serverSetup()));

    stateconnect2RM->addTransition(m_pRMConnection, SIGNAL(connected()), stateSendRMIdentandRegister);
    stateconnect2RM->addTransition(m_pRMConnection, SIGNAL(connectionRMError()), stateconnect2RMError);
    stateconnect2RMError->addTransition(this, SIGNAL(serverSetup()), stateconnect2RM);

    emit serverSetup(); // so we enter state machine's next state
}


void cSEC1000dServer::doCloseServer()
{
    QCoreApplication::instance()->exit(m_nerror);
}


void cSEC1000dServer::doConnect2RM()
{
    m_nerror = rmConnectionError; // preset error condition
    m_pRMConnection->connect2RM();
}


void cSEC1000dServer::connect2RMError()
{
    m_nRetryRMConnect--;
    if (m_nRetryRMConnect == 0)
        emit abortInit();
    else
        m_retryTimer.start(200);
}


void cSEC1000dServer::doIdentAndRegister()
{
    m_pRMConnection->SendIdent(getName());

    for (int i = 0; i < resourceList.count(); i++)
    {
        cResource *res = resourceList.at(i);
        connect(m_pRMConnection, SIGNAL(rmAck(quint32)), res, SLOT(resourceManagerAck(quint32)) );
        res->registerResource(m_pRMConnection, m_pETHSettings->getPort(server));
    }
}







