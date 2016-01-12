#include "inputinterface.h"
#include "inputsettings.h"
#include "protonetcommand.h"


cInputInterface::cInputInterface(cInputSettings *iSettings)
    :m_pInputSettings(iSettings)
{
}


void cInputInterface::initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface)
{
    cSCPIDelegate* delegate;

    if (leadingNodes != "")
        leadingNodes += ":";

    delegate = new cSCPIDelegate(QString("%1INPUT:CHANNEL").arg(leadingNodes),"CATALOG", SCPI::isQuery, scpiInterface, InputSystem::cmdChannelCat);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));

    for (int i = 0; i < m_pInputSettings->count(); i++ )
    {
        delegate = new cSCPIDelegate(QString("%1INPUT:%2").arg(leadingNodes).arg(m_pInputSettings->getList().at(i).m_sName),"MUX", SCPI::isQuery, scpiInterface, InputSystem::cmdChannelMux + i);
        m_DelegateList.append(delegate);
        connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    }
}


void cInputInterface::executeCommand(int cmdCode, cProtonetCommand *protoCmd)
{
    switch (cmdCode)
    {
    case InputSystem::cmdChannelCat:
        protoCmd->m_sOutput = m_ReadInputChannelCatalog(protoCmd->m_sInput);
        if (protoCmd->m_bwithOutput)
            emit cmdExecutionDone(protoCmd);
        break;
    default:
        if (cmdCode >= InputSystem::cmdChannelMux && cmdCode < InputSystem::cmdChannelMux + m_pInputSettings->count() )
        {
            protoCmd->m_sOutput = m_ReadInputChannelMux(cmdCode-InputSystem::cmdChannelMux, protoCmd->m_sInput);
            if (protoCmd->m_bwithOutput)
                emit cmdExecutionDone(protoCmd);
            break;
        }
    }
}


QString cInputInterface::m_ReadInputChannelCatalog(QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
        return m_pInputSettings->nameList();
    else
        return SCPI::scpiAnswer[SCPI::nak];
}


QString cInputInterface::m_ReadInputChannelMux(quint8 pos, QString &sInput)
{
    cSCPICommand cmd = sInput;

    if (cmd.isQuery())
        return QString("%1").arg(m_pInputSettings->getList().at(pos).m_nMux);
    else
        return SCPI::scpiAnswer[SCPI::nak];
}
