#include <syslog.h>
#include <QString>
#include <protonetpeer.h>
#include <netmessages.pb.h>
#include <QtDebug>

#include "sec1000dglobal.h"
#include "rmconnection.h"


cRMConnection::cRMConnection(QString ipadr, quint16 port, quint8 dlevel)
    :m_sIPAdr(ipadr), m_nPort(port), m_nDebugLevel(dlevel)
{
    m_pResourceManagerClient = 0;
    // qDebug() << "IP=" << m_sIPAdr << " PORT=" << m_nPort;
}


void cRMConnection::connect2RM()
{
    if (m_pResourceManagerClient) // in case we try to
        delete m_pResourceManagerClient;
    m_pResourceManagerClient = new ProtoNetPeer(this);
    m_pResourceManagerClient->setWrapper(&m_ProtobufWrapper);
    connect(m_pResourceManagerClient, SIGNAL(sigSocketError(QAbstractSocket::SocketError)), this, SLOT(tcpErrorHandler(QAbstractSocket::SocketError)));
    connect(m_pResourceManagerClient, SIGNAL(sigConnectionEstablished()), this, SIGNAL(connected()));
    connect(m_pResourceManagerClient, SIGNAL(sigConnectionClosed()), this, SIGNAL(connectionRMError()));
    connect(m_pResourceManagerClient, SIGNAL(sigMessageReceived(google::protobuf::Message*)), this, SLOT(responseHandler(google::protobuf::Message*)));
    m_pResourceManagerClient->startConnection(m_sIPAdr, m_nPort);
}


void cRMConnection::SendCommand(QString &cmd, QString &par, quint32 msgnr)
{
    m_sCommand = cmd;
    ProtobufMessage::NetMessage message;
    ProtobufMessage::NetMessage::ScpiCommand* scpiCmd = message.mutable_scpi();
    scpiCmd->set_command(cmd.toStdString());
    scpiCmd->set_parameter(par.toStdString());
    message.set_messagenr(msgnr);

    m_pResourceManagerClient->sendMessage(&message);
}


void cRMConnection::tcpErrorHandler(QAbstractSocket::SocketError errorCode)
{
    if (DEBUG1) syslog(LOG_ERR,"tcp socket error resource manager port: %d\n",errorCode);
    emit connectionRMError();
}


void cRMConnection::responseHandler(google::protobuf::Message *response)
{
    ProtobufMessage::NetMessage *answer;
    answer = static_cast<ProtobufMessage::NetMessage*>(response);
    if (answer != 0)
    {
        if ( !(answer->has_reply() && answer->reply().rtype() == answer->reply().ACK))
        {
            if (DEBUG1)
            {
                QByteArray ba = m_sCommand.toLocal8Bit();
                syslog(LOG_ERR,"command: %s, was not acknowledged\n", ba.data() );
            }
            emit connectionRMError();
        }

        else
        {
            emit rmAck(answer->messagenr());
        }
    }
    else
    {
        if (DEBUG1) syslog(LOG_ERR,"answer from resource manager not protobuf \n");
        emit connectionRMError();
    }
}


void cRMConnection::SendIdent(QString ident)
{
    ProtobufMessage::NetMessage envelope;
    ProtobufMessage::NetMessage::NetReply* message = envelope.mutable_reply();
    message->set_rtype(ProtobufMessage::NetMessage::NetReply::IDENT);
    message->set_body(ident.toStdString());

    m_pResourceManagerClient->sendMessage(&envelope);
}
