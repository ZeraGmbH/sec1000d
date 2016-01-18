#include <unistd.h>
#include <fcntl.h>

#include <QList>
#include <QString>

#include <scpi.h>
#include <scpicommand.h>

#include "sec1000d.h"
#include "scpiconnection.h"
#include "ecalcchannel.h"
#include "fpgasettings.h"
#include "inputsettings.h"
#include "protonetcommand.h"

extern void SigHandler(int);

cECalculatorChannel::cECalculatorChannel(cSEC1000dServer* server, cECalculatorSettings* esettings, cFPGASettings* fsettings, cInputSettings *inpsettings, quint16 nr)
    :m_pMyServer(server), m_pecalcsettings(esettings), m_pfpgasettings(fsettings), m_pInputSettings(inpsettings), m_nNr(nr)
{
    m_nBaseAdress = m_pecalcsettings->getBaseAdress();
    m_nMyAdress = m_nBaseAdress + (nr << 5);
    m_sName = QString("%1%2").arg(baseChnName).arg(nr);
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

    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"REGISTER", SCPI::isCmdwP, scpiInterface, ECalcChannel::cmdRegister);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"SYNC", SCPI::isCmdwP, scpiInterface, ECalcChannel::setSync);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"MUX", SCPI::isCmdwP, scpiInterface, ECalcChannel::setMux);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"CMDID", SCPI::isCmdwP, scpiInterface, ECalcChannel::setCmdid);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"START", SCPI::isCmdwP, scpiInterface, ECalcChannel::start);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1%2").arg(leadingNodes).arg(m_sName),"STOP", SCPI::isCmdwP, scpiInterface, ECalcChannel::stop);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
}


void cECalculatorChannel::executeCommand(int cmdCode, cProtonetCommand *protoCmd)
{
    switch (cmdCode)
    {
    case ECalcChannel::cmdRegister:
        m_ReadWriteRegister(protoCmd);
        break;
    case ECalcChannel::setSync:
        m_setSync(protoCmd);
        break;
    case ECalcChannel::setMux:
        m_setMux(protoCmd);
        break;
    case ECalcChannel::setCmdid:
        m_setCmdId(protoCmd);
        break;
    case ECalcChannel::start:
        m_start(protoCmd);
        break;
    case ECalcChannel::stop:
        m_stop(protoCmd);
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


void cECalculatorChannel::setIntReg(quint8 reg)
{
    notifierECalcChannelIntReg = reg;
}


void cECalculatorChannel::m_ReadWriteRegister(cProtonetCommand *protoCmd)
{
    bool ok1,ok2;
    cSCPICommand cmd = protoCmd->m_sInput;
    QString par;
    quint8 regInd;
    quint32 reg;

    if (cmd.isQuery(1))
    {
        par = cmd.getParam(0);
        regInd = par.toInt(&ok1);
        if (ok1)
        {
            switch (regInd)
            {
                case ECALCREG::CMD:
                case ECALCREG::CONF:
                case ECALCREG::STATUS:
                case ECALCREG::INTMASK:
                case ECALCREG::MTCNTin:
                case ECALCREG::MTCNTact:
                case ECALCREG::MTPULSin:
                case ECALCREG::MTPAUSEin:
                case ECALCREG::MTPULS:
                case ECALCREG::MTPAUSE:
                    lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (regInd << 2), 0);
                    read(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                    protoCmd->m_sOutput =  QString("%1").arg(reg);
                    break;
                case ECALCREG::INTREG:
                    emit notifier(&notifierECalcChannelIntReg);
                    protoCmd->m_sOutput =  QString("%1").arg(notifierECalcChannelIntReg.getValue()); // we only return the notifiers value
                    break;
                default:
                    protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
            }
        }
    }
    else
        if (cmd.isCommand(2))
        {
            if (protoCmd->m_clientId == m_ClientId) // authorized ?
            {
                par = cmd.getParam(0);
                regInd = par.toInt(&ok1);
                par = cmd.getParam(1);
                reg = par.toULong(&ok2);
                if (ok1 && ok2)
                {
                    switch (regInd)
                    {
                        case ECALCREG::CMD:
                        case ECALCREG::CONF:
                        case ECALCREG::INTMASK:
                        case ECALCREG::INTREG:
                        case ECALCREG::MTCNTin:
                        case ECALCREG::MTCNTact:
                        case ECALCREG::MTPULSin:
                        case ECALCREG::MTPAUSEin:
                        case ECALCREG::MTPULS:
                        case ECALCREG::MTPAUSE:
                            lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (regInd << 2), 0);
                            write(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                            if (regInd == ECALCREG::INTREG)
                            {
                                notifierECalcChannelIntReg.setValue(reg);
                                SigHandler(0); // we do so as if the interrupt handler had seen another edge
                            }
                            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
                            break;
                        default:
                            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
                    }
                }
                else
                    protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
            }
            else
                protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::erraut];
        }
        else
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorChannel::m_setSync(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    if (cmd.isCommand(1))
    {
        if (protoCmd->m_clientId == m_ClientId) // authorized ?
        {
            QString par;
            par = cmd.getParam(0);
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::errval]; // preset
            if (par.contains("ec"))
            {
                bool ok;
                quint32 chnIndex;
                par.remove(baseChnName);
                chnIndex = par.toULong(&ok);
                if (ok && (chnIndex <= m_pecalcsettings->getNumber()) )
                {
                    quint32 reg;
                    lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (ECALCREG::CONF << 2), 0);
                    read(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                    reg = (reg & 0xFFFFFF00) | chnIndex;
                    write(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                    protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
                }
            }
        }
        else
           protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::erraut];
    }
    else
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorChannel::m_setMux(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    if (cmd.isCommand(1))
    {
        if (protoCmd->m_clientId == m_ClientId) // authorized ?
        {
            bool ok;
            QString par;
            par = cmd.getParam(0);
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::errval]; // preset

            if (m_pInputSettings->hasInput(par))
            {
                quint32 reg;
                lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (ECALCREG::CONF << 2), 0);
                read(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                reg = (reg & 0xFFFF83FF) | m_pInputSettings->mux(par);
                write(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
            }
        }
        else
           protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::erraut];
    }
    else
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorChannel::m_setCmdId(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    if (cmd.isCommand(1))
    {
        if (protoCmd->m_clientId == m_ClientId) // authorized ?
        {
            bool ok;
            quint32 cmdId;
            QString par;
            par = cmd.getParam(0);
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::errval]; // preset
            cmdId = par.toULong(&ok);
            if (ok && (cmdId < 64) )
            {
                quint32 reg;
                lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (ECALCREG::CMD << 2), 0);
                read(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                reg = (reg & 0xFFFFFFC0) | cmdId;
                write(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
                protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
            }
        }
        else
           protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::erraut];
    }
    else
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorChannel::m_start(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    if (cmd.isCommand(0))
    {
        if (protoCmd->m_clientId == m_ClientId) // authorized ?
        {
            quint32 reg;
            lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (ECALCREG::CMD << 2), 0);
            read(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
            reg |= 0x80;
            write(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
        }
        else
           protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::erraut];
    }
    else
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorChannel::m_stop(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;
    if (cmd.isCommand(0))
    {
        if (protoCmd->m_clientId == m_ClientId) // authorized ?
        {
            quint32 reg;
            lseek(m_pMyServer->DevFileDescriptor, m_nMyAdress + (ECALCREG::CMD << 2), 0);
            read(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
            reg |= 0x40;
            write(m_pMyServer->DevFileDescriptor,(char*) &reg, 4);
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
        }
        else
           protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::erraut];
    }
    else
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cECalculatorChannel::setNotifierECalcChannelIntReg()
{
    notifierECalcChannelIntReg = 0; // we have no interrupt yet
}


