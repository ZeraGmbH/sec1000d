#include <scpi.h>
#include <scpicommand.h>

#include "sec1000d.h"
#include "scpidelegate.h"
#include "systeminfo.h"
#include "systeminterface.h"
#include "protonetcommand.h"


cSystemInterface::cSystemInterface(cSEC1000dServer *server, cSystemInfo *sInfo)
    :m_pMyServer(server), m_pSystemInfo(sInfo)
{
}


void cSystemInterface::initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface)
{
    cSCPIDelegate* delegate;

    if (leadingNodes != "")
        leadingNodes += ":";

    delegate = new cSCPIDelegate(QString("%1SYSTEM:VERSION").arg(leadingNodes),"SERVER", SCPI::isQuery, scpiInterface, SystemSystem::cmdVersionServer);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1SYSTEM:VERSION").arg(leadingNodes),"DEVICE", SCPI::isQuery, scpiInterface, SystemSystem::cmdVersionDevice);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1SYSTEM:VERSION").arg(leadingNodes), "PCB", SCPI::isQuery | SCPI::isCmdwP, scpiInterface, SystemSystem::cmdVersionPCB);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1SYSTEM:VERSION").arg(leadingNodes), "FPGA", SCPI::isQuery, scpiInterface, SystemSystem::cmdVersionFPGA);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1SYSTEM").arg(leadingNodes), "SERIAL", SCPI::isQuery | SCPI::isCmdwP , scpiInterface, SystemSystem::cmdSerialNumber);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1SYSTEM:INTERFACE").arg(leadingNodes), "READ", SCPI::isQuery, scpiInterface, SystemSystem::cmdInterfaceRead);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
}


void cSystemInterface::executeCommand(int cmdCode, cProtonetCommand *protoCmd)
{
    switch (cmdCode)
    {
    case SystemSystem::cmdVersionServer:
        protoCmd->m_sOutput = m_ReadServerVersion(protoCmd->m_sInput);
        break;
    case SystemSystem::cmdVersionDevice:
        protoCmd->m_sOutput = m_ReadDeviceVersion(protoCmd->m_sInput);
        break;
    case SystemSystem::cmdVersionPCB:
        protoCmd->m_sOutput = m_ReadWritePCBVersion(protoCmd->m_sInput);
        break;
    case SystemSystem::cmdVersionFPGA:
        protoCmd->m_sOutput = m_ReadFPGAVersion(protoCmd->m_sInput);
        break;
    case SystemSystem::cmdSerialNumber:
        protoCmd->m_sOutput = m_ReadWriteSerialNumber(protoCmd->m_sInput);
        break;
    case SystemSystem::cmdInterfaceRead:
        protoCmd->m_sOutput = m_InterfaceRead(protoCmd->m_sInput);
        break;
    }

    if (protoCmd->m_bwithOutput)
        emit cmdExecutionDone(protoCmd);
}


QString cSystemInterface::m_ReadServerVersion(QString &sInput)
{
    QString s;
    cSCPICommand cmd = sInput;

    if ( cmd.isQuery() )
    {
        s = m_pMyServer->getVersion();
    }
    else
        s = SCPI::scpiAnswer[SCPI::nak];

    return s;
}


QString cSystemInterface::m_ReadDeviceVersion(QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        if (m_pSystemInfo->dataRead())
            return m_pSystemInfo->getDeviceVersion();
        else
            return SCPI::scpiAnswer[SCPI::errexec];
    }
    else
        return SCPI::scpiAnswer[SCPI::nak];
}


QString cSystemInterface::m_ReadDeviceName(QString& sInput)
{
    QString s;
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        if (m_pSystemInfo->dataRead())
            return m_pSystemInfo->getDeviceName();
        else
            return SCPI::scpiAnswer[SCPI::errexec];
    }
    else
        return SCPI::scpiAnswer[SCPI::nak];
}


QString cSystemInterface::m_ReadWritePCBVersion(QString &sInput)
{
    QString s;
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        if (m_pSystemInfo->dataRead())
            s = m_pSystemInfo->getPCBVersion();
        else
            s = SCPI::scpiAnswer[SCPI::errexec];
    }
    else
    {
        if (cmd.isCommand(1))
        {
            QString Version = cmd.getParam(0);
            // todo write here the pcb version
            m_pSystemInfo->getSystemInfo(); // read back info
            s = SCPI::scpiAnswer[SCPI::ack];
        }
        else
            s = SCPI::scpiAnswer[SCPI::nak];
    }

    return s;
}


QString cSystemInterface::m_ReadFPGAVersion(QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        if (m_pSystemInfo->dataRead())
            return m_pSystemInfo->getLCAVersion();
        else
            return SCPI::scpiAnswer[SCPI::errexec];
    }

    else
        return SCPI::scpiAnswer[SCPI::nak];
}


QString cSystemInterface::m_ReadWriteSerialNumber(QString &sInput)
{
    QString s;
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        {
            if (m_pSystemInfo->dataRead())
                s = m_pSystemInfo->getSerialNumber();
            else
                s = SCPI::scpiAnswer[SCPI::errexec];
        }
    }
    else
    {
        if (cmd.isCommand(1))
        {
            QString Serial = cmd.getParam(0);
            // todo write the serial number
            m_pSystemInfo->getSystemInfo(); // read back info
            s = SCPI::scpiAnswer[SCPI::ack];
        }
        else
            s = SCPI::scpiAnswer[SCPI::nak];
    }

    return s;
}


QString cSystemInterface::m_InterfaceRead(QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
    {
        QString s;
        m_pMyServer->getSCPIInterface()->exportSCPIModelXML(s);
        return s;
    }
    else
        return SCPI::scpiAnswer[SCPI::nak];
}




