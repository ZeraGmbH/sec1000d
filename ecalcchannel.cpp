#include <unistd.h>
#include <fcntl.h>

#include <QList>
#include <QString>

#include <scpi.h>
#include <scpicommand.h>

#include "scpiconnection.h"
#include "ecalcchannel.h"
#include "fpgasettings.h"
#include "protonetcommand.h"


cECalculatorChannel::cECalculatorChannel(cECalculatorSettings* esettings, cFPGASettings* fsettings, quint16 nr)
    :m_pecalcsettings(esettings), m_pfpgasettings(fsettings), m_nNr(nr)
{
    m_nBaseAdress = m_pecalcsettings->getBaseAdress();
    m_nMyAdress = m_nBaseAdress + (nr << 4);
    m_sName = QString("ec%1").arg(nr);
    m_bSet = false;
    setNotifierECalcChannelIntReg();
}


cECalculatorChannel::~cECalculatorChannel()
{
}


void cECalculatorChannel::initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface)
{
    cSCPIDelegate* delegate;

    if (leadingNodes != "")
        leadingNodes += ":";

    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"CMD", SCPI::isQuery | SCPI::isCmdwP, scpiInterface, ECalcChannel::cmdCmd);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"CONF", SCPI::isQuery | SCPI::isCmdwP, scpiInterface, ECalcChannel::cmdConf);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"STATUS", SCPI::isQuery, scpiInterface, ECalcChannel::cmdStatus);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"INTMASK", SCPI::isQuery | SCPI::isCmdwP, scpiInterface, ECalcChannel::cmdIntmask);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"INT", SCPI::isQuery | SCPI::isCmdwP, scpiInterface, ECalcChannel::cmdIntreg);
}


void cECalculatorChannel::executeCommand(int cmdCode, cProtonetCommand *protoCmd)
{
    switch (cmdCode)
    {
    case ECalcChannel::cmdCmd:
        m_ReadWriteCMD(protoCmd);
        break;
    case ECalcChannel::cmdConf:
        m_ReadWriteCONF(protoCmd);
        break;
    case ECalcChannel::cmdStatus:
        m_ReadSTATUS(protoCmd);
        break;
    case ECalcChannel::cmdIntmask:
        m_ReadWriteINTMASK(protoCmd);
        break;
    case ECalcChannel::cmdIntreg:
        m_ReadWriteINTREG(protoCmd);
        break;
    }

    if (protoCmd->m_bwithOutput)
        emit cmdExecutionDone(protoCmd);

}


QString &cECalculatorChannel::getName()
{
    return m_sName;
}


bool cECalculatorChannel::isfree()
{
    return !m_bSet;
}


bool cECalculatorChannel::set(QByteArray id)
{
    bool ret = !m_bSet;
    if (ret)
        m_ClientId = id;
    m_bSet = true;
    return ret;
}


void cECalculatorChannel::free()
{
    m_bSet = false;
}


void cECalculatorChannel::m_ReadWriteCMD(cProtonetCommand *protoCmd)
{
    m_ReadWriteRegister(protoCmd, CMDREG);
}


void cECalculatorChannel::m_ReadWriteCONF(cProtonetCommand *protoCmd)
{
    m_ReadWriteRegister(protoCmd, CONFREG);
}


void cECalculatorChannel::m_ReadSTATUS(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    QString answ;
    quint32 reg;
    int m_nFPGAfd;

    m_nFPGAfd = open(m_pfpgasettings->getDeviceNode().toLatin1().data() ,O_RDWR);
    lseek(m_nFPGAfd,m_nMyAdress+STATUSREG,0);
    if (cmd.isQuery())
    {
        read(m_nFPGAfd,(char*) &reg, 4);
        answ =  QString("%1").arg(reg);
    }
    else
        answ = SCPI::scpiAnswer[SCPI::nak];

    close(m_nFPGAfd);
    protoCmd->m_sOutput = answ;
}


void cECalculatorChannel::m_ReadWriteINTMASK(cProtonetCommand *protoCmd)
{
    return m_ReadWriteRegister(protoCmd, INTMASKREG);
}


void cECalculatorChannel::m_ReadWriteINTREG(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    QString answ;
    quint32 reg;
    int m_nFPGAfd;

    m_nFPGAfd = open(m_pfpgasettings->getDeviceNode().toLatin1().data() ,O_RDWR);
    lseek(m_nFPGAfd,m_nMyAdress+INTREG,0);
    if (cmd.isQuery())
    {
        emit notifier(&notifierECalcChannelIntReg);
        answ =  notifierECalcChannelIntReg.getString(); // we only return the notifiers value
    }
    else
    {
        if (cmd.isCommand(1))
        {
            if (protoCmd->m_clientId == m_ClientId)
            {
                bool ok;
                reg = cmd.getParam(0).toULong(&ok);
                write(m_nFPGAfd,(char*) &reg, 4);
                answ = SCPI::scpiAnswer[SCPI::ack];
            }
            else
                answ = SCPI::scpiAnswer[SCPI::erraut];
        }
        else
            answ = SCPI::scpiAnswer[SCPI::nak];
    }

    close(m_nFPGAfd);
    protoCmd->m_sOutput = answ;
}


void cECalculatorChannel::m_ReadWriteRegister(cProtonetCommand *protoCmd, quint32 adroffs)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    QString answ;
    quint32 reg;
    int m_nFPGAfd;

    m_nFPGAfd = open(m_pfpgasettings->getDeviceNode().toLatin1().data() ,O_RDWR);
    lseek(m_nFPGAfd,m_nMyAdress+adroffs,0);
    if (cmd.isQuery())
    {
        read(m_nFPGAfd,(char*) &reg, 4);
        answ =  QString("%1").arg(reg);
    }
    else
    {
        if (cmd.isCommand(1))
        {
            if (protoCmd->m_clientId == m_ClientId)
            {
                bool ok;
                reg = cmd.getParam(0).toULong(&ok);
                write(m_nFPGAfd,(char*) &reg, 4);
                answ = SCPI::scpiAnswer[SCPI::ack];
            }
            else
                answ = SCPI::scpiAnswer[SCPI::erraut];
        }
        else
            answ = SCPI::scpiAnswer[SCPI::nak];
    }

    close(m_nFPGAfd);
    protoCmd->m_sOutput = answ;
}


void cECalculatorChannel::setNotifierECalcChannelIntReg()
{
    notifierECalcChannelIntReg="0"; // we have no interrupt yet
}


