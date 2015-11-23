#include <QList>
#include <QStringList>
#include <QDebug>
#include <scpi.h>

#include "xmlsettings.h"
#include "scpiconnection.h"
#include "resource.h"

#include "sec1000d.h"
#include "ecalcsettings.h"
#include "fpgasettings.h"
#include "ethsettings.h"
#include "ecalcinterface.h"
#include "ecalcchannel.h"
#include "protonetcommand.h"


cECalculatorInterface::cECalculatorInterface(cSEC1000dServer* server, cETHSettings* ethsettings, cECalculatorSettings* ecalcSettings, cFPGASettings* fpgasettings)
    : m_pMyServer(server), m_pETHsettings(ethsettings), m_pecalcsettings(ecalcSettings), m_pfpgasettings(fpgasettings)
{
    m_sVersion = ECalcSystem::Version;

    // first we create the configured number of error calculators and attach them into a hash table for better access
    int n = m_pecalcsettings->getNumber();
    for (int i = 0; i < n; i++ )
    {
        cECalculatorChannel* eChan = new cECalculatorChannel(m_pMyServer, m_pecalcsettings, m_pfpgasettings, i);
        m_ECalculatorChannelList.append(eChan); // we have a list for seq. access
        m_ECalculatorChannelHash[eChan->getName()] = eChan; // and a hash for access by channel nam3
    }
}


cECalculatorInterface::~cECalculatorInterface()
{
    QList<cECalculatorChannel*> ecList;
    ecList = m_ECalculatorChannelHash.values();
    cECalculatorChannel* cptr;

    int n = ecList.count();
    for (int i = 0; n; i++)
    {
        cptr = ecList.at(i);
        delete cptr;
    }
}


void cECalculatorInterface::initSCPIConnection(QString leadingNodes, cSCPI* scpiInterface)
{
    cSCPIDelegate* delegate;

    if (leadingNodes != "")
        leadingNodes += ":";

    delegate = new cSCPIDelegate(QString("%1ECALCULATOR").arg(leadingNodes),"VERSION",SCPI::isQuery,scpiInterface, ECalcSystem::cmdVersion);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1ECALCULATOR:CHANNEL").arg(leadingNodes),"CATALOG", SCPI::isQuery, scpiInterface, ECalcSystem::cmdChannelCat);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1ECALCULATOR").arg(leadingNodes),"SET",SCPI::CmdwP,scpiInterface, ECalcSystem::cmdSetChannels);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1ECALCULATOR").arg(leadingNodes),"FREE",SCPI::CmdwP,scpiInterface, ECalcSystem::cmdFreeChannels);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));

    QList<cECalculatorChannel*> ecList;
    ecList = m_ECalculatorChannelHash.values();
    int n = ecList.count();
    for (int i = 0; i < n; i++)
    {
        // we also must connect the signals for notification and for output
        connect(ecList.at(i), SIGNAL(notifier(cNotificationValue*)), this, SIGNAL(notifier(cNotificationValue*)));
        connect(ecList.at(i), SIGNAL(cmdExecutionDone(cProtonetCommand*)), this, SIGNAL(cmdExecutionDone(cProtonetCommand*)));

        ecList.at(i)->initSCPIConnection(QString("%1ECALCULATOR").arg(leadingNodes),scpiInterface);
    }
}


void cECalculatorInterface::executeCommand(int cmdCode, cProtonetCommand *protoCmd)
{
    switch (cmdCode)
    {
    case ECalcSystem::cmdVersion:
        protoCmd->m_sOutput = m_ReadVersion(protoCmd->m_sInput);
        if (protoCmd->m_bwithOutput)
            emit cmdExecutionDone(protoCmd);
        break;
    case ECalcSystem::cmdChannelCat:
        protoCmd->m_sOutput = m_ReadECalculatorChannelCatalog(protoCmd->m_sInput);
        if (protoCmd->m_bwithOutput)
            if (protoCmd->m_bwithOutput)
                emit cmdExecutionDone(protoCmd);
        break;
    case ECalcSystem::cmdSetChannels:
        m_SetChannels(protoCmd);
        if (protoCmd->m_bwithOutput)
            emit cmdExecutionDone(protoCmd);
        break;
    case ECalcSystem::cmdFreeChannels:
        m_FreeChannels(protoCmd);
        if (protoCmd->m_bwithOutput)
            emit cmdExecutionDone(protoCmd);
        break;
    }
}


void cECalculatorInterface::registerResource(cRMConnection *rmConnection, quint16 port)
{
    // we register all our error calculator units as resources
    register1Resource(rmConnection, m_pMyServer->getMsgNr(), QString("SEC1;ECALCULATOR;%1;%2;%3;")
                      .arg(m_pecalcsettings->getNumber())
                      .arg(ECalcSystem::sECalculatorDescription)
                      .arg(port));

}


void cECalculatorInterface::unregisterResource(cRMConnection *rmConnection)
{
    unregister1Resource(rmConnection, m_pMyServer->getMsgNr(), QString("SEC1;ECALCULATOR;"));
}


QList<cECalculatorChannel *> cECalculatorInterface::getECalcChannelList()
{
    return m_ECalculatorChannelList;
}


QString cECalculatorInterface::m_ReadVersion(QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
        return m_sVersion;
    else
        return SCPI::scpiAnswer[SCPI::nak];
}


QString cECalculatorInterface::m_ReadECalculatorChannelCatalog(QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        QString s;
        QList<cECalculatorChannel*> ecList;
        ecList = m_ECalculatorChannelHash.values();
        int n = ecList.count();
        for (int i = 0; i < n; i++)
            s += QString("%1;").arg(ecList.at(i)->getName());

        return s;
    }
    else
        return SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorInterface::m_SetChannels(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    QString answ;

    if (cmd.isCommand(1))
    {
       bool ok;
       int n = cmd.getParam(0).toInt(&ok);
       if (ok && (n > 0) && (n < 5)) // we accept 1 .. 4 ecalc requests
       {
           QString s;
           QList<cECalculatorChannel*> ecList;
           QList<cECalculatorChannel*> selEChannels;
           ecList = m_ECalculatorChannelHash.values();
           int m = ecList.count();
           for (int i = 0; i < m; i++)
           {
               if (ecList.at(i)->isfree())
               {
                   selEChannels.append(ecList.at(i));
                   s += QString("%1;").arg(ecList.at(i)->getName());
                   n--;
               }
               if (n == 0)
                   break;
           }
           if (n == 0)
           {
               answ = s;
               QByteArray id = protoCmd->m_clientId;
               m_ClientECalcHash[id] = s;
               for (int i = 0; i < selEChannels.count(); i++)
                   selEChannels.at(i)->set(id);
           }
           else
               answ = SCPI::scpiAnswer[SCPI::busy];
       }
       else
           answ = SCPI::scpiAnswer[SCPI::nak];
    }
    else
        answ = SCPI::scpiAnswer[SCPI::nak];

    protoCmd->m_sOutput = answ;
}


void cECalculatorInterface::m_FreeChannels(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    QString answ;

    if (cmd.isCommand(0))
    {
       if (m_ClientECalcHash.contains(protoCmd->m_clientId))
       {
           QStringList sl = m_ClientECalcHash[protoCmd->m_clientId].split(";");
           for (int i = 0; i < sl.count(); i++)
                m_ECalculatorChannelHash[sl.at(i)]->free();
       }
       else
           answ = SCPI::scpiAnswer[SCPI::nak];
    }
    else
        answ = SCPI::scpiAnswer[SCPI::nak];

    protoCmd->m_sOutput = answ;
}






