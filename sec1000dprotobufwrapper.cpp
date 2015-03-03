#include <qdebug.h>
#include <netmessages.pb.h>

#include "sec1000dprotobufwrapper.h"


google::protobuf::Message *cSec1000dProtobufWrapper::byteArrayToProtobuf(QByteArray bA)
{
    ProtobufMessage::NetMessage *proto = new ProtobufMessage::NetMessage();
    if(!proto->ParseFromArray(bA, bA.size()))
    {
        ProtobufMessage::NetMessage::ScpiCommand *cmd = proto->mutable_scpi();
        cmd->set_command(bA.data(), bA.size() );
    }
    return proto;
}


QByteArray cSec1000dProtobufWrapper::protobufToByteArray(google::protobuf::Message *pMessage)
{
    return QByteArray(pMessage->SerializeAsString().c_str(), pMessage->ByteSize());
}


