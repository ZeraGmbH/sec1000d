#include <QFile>
#include <QByteArray>
#include <QTextCodec>
#include <QList>
#include <QString>
#include <QTcpSocket>
#include <protonetpeer.h>
#include <xmlconfigreader.h>
#include <protonetserver.h>
#include <scpi.h>
#include <fcntl.h>
#include <unistd.h>
#include <netmessages.pb.h>
#include <QtDebug>

#include "protonetcommand.h"
#include "resource.h"
#include "scpiconnection.h"
#include "pcbserver.h"
#include "sec1000dglobal.h"

cPCBServer::cPCBServer(QObject *parent)
    : cSCPIConnection(parent)
{
    m_nMsgNr = 0;
    m_sServerName = ServerName;
    m_sServerVersion = ServerVersion;
    myXMLConfigReader = new Zera::XMLConfig::cReader();
}


void cPCBServer::initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface)
{
    cSCPIDelegate* delegate;

    if (leadingNodes != "")
        leadingNodes += ":";

    delegate = new cSCPIDelegate(QString("%1SERVER").arg(leadingNodes),"REGISTER",SCPI::isCmdwP,scpiInterface, PCBServer::cmdRegister);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
    delegate = new cSCPIDelegate(QString("%1SERVER").arg(leadingNodes),"UNREGISTER",SCPI::isQuery | SCPI::isCmd ,scpiInterface, PCBServer::cmdUnregister);
    m_DelegateList.append(delegate);
    connect(delegate, SIGNAL(execute(int, cProtonetCommand*)), this, SLOT(executeCommand(int, cProtonetCommand*)));
}


quint32 cPCBServer::getMsgNr()
{
    m_nMsgNr++;
    if (m_nMsgNr == 0)
        m_nMsgNr++;
    return m_nMsgNr;
}


QString &cPCBServer::getName()
{
    return m_sServerName;
}


QString &cPCBServer::getVersion()
{
    return m_sServerVersion;
}


cSCPI *cPCBServer::getSCPIInterface()
{
    return m_pSCPInterface;
}


void cPCBServer::setupServer()
{
    m_pSCPInterface = new cSCPI(m_sServerName); // our scpi interface
    myServer = new ProtoNetServer(this); // our working (talking) horse
    myServer->setDefaultWrapper(&m_ProtobufWrapper);
    connect(myServer,SIGNAL(sigClientConnected(ProtoNetPeer*)),this,SLOT(establishNewConnection(ProtoNetPeer*)));
}


void cPCBServer::executeCommand(int cmdCode, cProtonetCommand *protoCmd)
{
    switch (cmdCode)
    {
    case PCBServer::cmdRegister:
        m_RegisterNotifier(protoCmd);
        break;
    case PCBServer::cmdUnregister:
        m_UnregisterNotifier(protoCmd);
        break;
    }

    if (protoCmd->m_bwithOutput)
        emit cmdExecutionDone(protoCmd);
}


void cPCBServer::sendAnswer(cProtonetCommand *protoCmd)
{
    if (protoCmd->m_bhasClientId)
    {
        ProtobufMessage::NetMessage protobufAnswer;
        ProtobufMessage::NetMessage::NetReply *Answer = protobufAnswer.mutable_reply();

        // dependent on rtype caller can see ack, nak, error
        // in case of error the body has to be analyzed for details

        QString output = protoCmd->m_sOutput;

        if (output.contains(SCPI::scpiAnswer[SCPI::ack]))
            Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ACK);
        else
            if (output.contains(SCPI::scpiAnswer[SCPI::nak]))
                Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_NACK);
            else
                if (output.contains(SCPI::scpiAnswer[SCPI::busy]))
                    Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                else
                    if (output.contains(SCPI::scpiAnswer[SCPI::errcon]))
                        Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                    else
                        if (output.contains(SCPI::scpiAnswer[SCPI::erraut]))
                            Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                        else
                            if (output.contains(SCPI::scpiAnswer[SCPI::errval]))
                                Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                            else
                                if (output.contains(SCPI::scpiAnswer[SCPI::errxml]))
                                    Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                                else
                                    if (output.contains(SCPI::scpiAnswer[SCPI::errmmem]))
                                        Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                                    else
                                        if (output.contains(SCPI::scpiAnswer[SCPI::errpath]))
                                            Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                                        else
                                            if (output.contains(SCPI::scpiAnswer[SCPI::errexec]))
                                                Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                                            else
                                                if (output.contains(SCPI::scpiAnswer[SCPI::errtimo]))
                                                    Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ERROR);
                                                else
                                                    Answer->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ACK);

        Answer->set_body(output.toStdString()); // in any case we set the body

        protobufAnswer.set_clientid(protoCmd->m_clientId, protoCmd->m_clientId.count());
        protobufAnswer.set_messagenr(protoCmd->m_nmessageNr);

        protoCmd->m_pPeer->sendMessage(&protobufAnswer);
    }

    else

    {
        QByteArray block;

        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (qint32)0;

        out << protoCmd->m_sOutput.toUtf8();
        out.device()->seek(0);
        out << (qint32)(block.size() - sizeof(qint32));

        protoCmd->m_pPeer->getTcpSocket()->write(block);
    }

    delete protoCmd;
}


void cPCBServer::m_RegisterNotifier(cProtonetCommand *protoCmd)
{
    QString dummy;
    cSCPICommand cmd = protoCmd->m_sInput;

    if (cmd.isCommand(1)) // we only expect 1 parameter
    {
        cSCPIObject* scpiObject;
        QString query = cmd.getParam(0);

        if ( (scpiObject =  m_pSCPInterface->getSCPIObject(query, dummy)) != 0)
        {
            cNotificationData notData;

            notData.netPeer = protoCmd->m_pPeer;
            notData.clientID = protoCmd->m_clientId;

            notifierRegisterNext.append(notData); // we wait for a notifier signal

            cSCPIDelegate* scpiDelegate = static_cast<cSCPIDelegate*>(scpiObject);
            cProtonetCommand* procmd = new cProtonetCommand(protoCmd);
            procmd->m_nSCPIType = SCPI::isQuery; // we need to set query type for proper execution
            procmd->m_bwithOutput = false;
            procmd->m_sInput = query;

            if (!scpiDelegate->executeSCPI(procmd))
            {
                protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
                notifierRegisterNext.pop_back();
            }
            else
                protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack]; // we overwrite the query's output here
        }
        else
            protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];

    }
}


void cPCBServer::m_UnregisterNotifier(cProtonetCommand *protoCmd)
{
    cSCPICommand cmd = protoCmd->m_sInput;

    if (cmd.isCommand(1) && (cmd.getParam(0) == "") )
    {
        doUnregisterNotifier(protoCmd);
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::ack];
    }
    else
        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
}


void cPCBServer::doUnregisterNotifier(cProtonetCommand *protoCmd)
{
    if (notifierRegisterList.count() > 0)
    {
        QList<int> posList;
        // we have to remove all notifiers for this client and or clientId
        for (int i = 0; i < notifierRegisterList.count(); i++)
        {
            cNotificationData notData = notifierRegisterList.at(i);
            if (protoCmd->m_pPeer == notData.netPeer)
            { // we found the client
                if (notData.clientID.isEmpty() or (notData.clientID == protoCmd->m_clientId))
                {
                    posList.append(i);
                }
            }
        }

        if (posList.count() > 0)
        {
            for (int i = 0; i < posList.count(); i++)
                notifierRegisterList.removeAt(posList.at(i));
        }
    }
}

void cPCBServer::establishNewConnection(ProtoNetPeer *newClient)
{
    connect(newClient,SIGNAL(sigMessageReceived(google::protobuf::Message*)),this,SLOT(executeCommand(google::protobuf::Message*)));
    // later ... connect(newClient,SIGNAL(sigMessageReceived(QByteArray*)),this,SLOT(executeCommand(QByteArray*)));
}


void cPCBServer::executeCommand(google::protobuf::Message* cmd)
{
    ProtobufMessage::NetMessage *protobufCommand;
    cSCPIObject* scpiObject;
    QString dummy;

    //ProtoNetPeer* client = qobject_cast<ProtoNetPeer*>(sender());

    ProtoNetPeer* peer = qobject_cast<ProtoNetPeer*>(sender());
    protobufCommand = static_cast<ProtobufMessage::NetMessage*>(cmd);

    if ( (protobufCommand != 0) && (peer != 0))
    {
        if (protobufCommand->has_clientid())
        {
            QByteArray clientId = QByteArray(protobufCommand->clientid().data(), protobufCommand->clientid().size());

            if (protobufCommand->has_netcommand())
            {
                // in case of "lost" clients we delete registration for notification
                cProtonetCommand* protoCmd = new cProtonetCommand(peer, true, true, clientId, 0, "", 0);
                doUnregisterNotifier(protoCmd);
            }

            else

            if (protobufCommand->has_messagenr())
            {
                quint32 messageNr = protobufCommand->messagenr();
                ProtobufMessage::NetMessage::ScpiCommand scpiCmd = protobufCommand->scpi();
                m_sInput = QString::fromStdString(scpiCmd.command()) +  " " + QString::fromStdString(scpiCmd.parameter());

                cProtonetCommand* protoCmd;

                if ( (scpiObject =  m_pSCPInterface->getSCPIObject(m_sInput, dummy)) != 0)
                {
                    protoCmd = new cProtonetCommand(peer, true, true, clientId, messageNr, m_sInput, scpiObject->getType());
                    cSCPIDelegate* scpiDelegate = static_cast<cSCPIDelegate*>(scpiObject);
                    if (!scpiDelegate->executeSCPI(protoCmd))
                    {
                        protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
                        emit cmdExecutionDone(protoCmd);
                    }
                }
                else
                {
                    protoCmd = new cProtonetCommand(peer, true, true, clientId, messageNr, m_sInput, 0);
                    protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak];
                    emit cmdExecutionDone(protoCmd);
                }

                // we get a signal when a command is finished and send answer then
            }
        }

        else

        {
            m_sInput =  QString::fromStdString(protobufCommand->scpi().command());
            QByteArray clientId = QByteArray(); // we set an empty byte array
            cProtonetCommand* protoCmd;
            if ( (scpiObject =  m_pSCPInterface->getSCPIObject(m_sInput, dummy)) != 0)
            {
                protoCmd = new cProtonetCommand(peer, false, true, clientId, 0, m_sInput, scpiObject->getType());
                cSCPIDelegate* scpiDelegate = static_cast<cSCPIDelegate*>(scpiObject);

                if (!scpiDelegate->executeSCPI(protoCmd))
                {
                    protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak]+";";
                    emit cmdExecutionDone(protoCmd);
                }

            }
            else
            {
                protoCmd = new cProtonetCommand(peer, false, true, clientId, 0, m_sInput, 0);
                protoCmd->m_sOutput = SCPI::scpiAnswer[SCPI::nak]+";";
                emit cmdExecutionDone(protoCmd);
            }
        }
    }
}


void cPCBServer::establishNewNotifier(cNotificationValue *notifier)
{
    if (notifierRegisterNext.count() > 0) // if we're waiting for notifier
    {
        disconnect(notifier, 0, 0, 0); // we disconnect first because we only want 1 signal
        cNotificationData notData = notifierRegisterNext.takeFirst(); // we pick the notification data
        notData.notValue = notifier;
        notifierRegisterList.append(notData); //
        connect(notifier, SIGNAL(risingEdge(quint32)), this, SLOT(asyncHandler(quint32)));
    }
}


void cPCBServer::asyncHandler(quint32 irqreg)
{
    cNotificationValue* notifier = qobject_cast<cNotificationValue*>(sender());

    if (notifierRegisterList.count() > 0)
    {
        ProtobufMessage::NetMessage protobufIntMessage;

        if (notifierRegisterList.count() > 0)
            for (int i = 0; i < notifierRegisterList.count(); i++)
            {
                cNotificationData notData = notifierRegisterList.at(i);
                if (notData.notValue == notifier)
                {
                    ProtobufMessage::NetMessage::NetReply *intMessage = protobufIntMessage.mutable_reply();
                    QString s = QString("IRQ:%1").arg(irqreg);
                    if (notData.clientID.isEmpty()) // old style communication
                    {
                        QByteArray block;

                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_0);
                        out << (qint32)0;

                        out << s.toUtf8();
                        out.device()->seek(0);
                        out << (qint32)(block.size() - sizeof(qint32));

                        notData.netPeer->getTcpSocket()->write(block);
                    }
                    else
                    {
                        intMessage->set_body(s.toStdString());
                        intMessage->set_rtype(ProtobufMessage::NetMessage_NetReply_ReplyType_ACK);
                        QByteArray id = notData.clientID;
                        protobufIntMessage.set_clientid(id, id.count());
                        protobufIntMessage.set_messagenr(0); // interrupt

                        notData.netPeer->sendMessage(&protobufIntMessage);
                    }
                }
            }
    }
}


void cPCBServer::initSCPIConnections()
{
    for (int i = 0; i < scpiConnectionList.count(); i++)
    {
        scpiConnectionList.at(i)->initSCPIConnection("",m_pSCPInterface); // we have our interface
        connect(scpiConnectionList.at(i), SIGNAL(notifier(cNotificationValue*)), this, SLOT(establishNewNotifier(cNotificationValue*)));
        connect(scpiConnectionList.at(i), SIGNAL(cmdExecutionDone(cProtonetCommand*)), this, SLOT(sendAnswer(cProtonetCommand*)));
    }
}


