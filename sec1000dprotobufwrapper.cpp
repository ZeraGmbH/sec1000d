#include <qdebug.h>
#include <netmessages.pb.h>

#include "sec1000dprotobufwrapper.h"


cSec1000dProtobufWrapper::cSec1000dProtobufWrapper()
{
}

std::shared_ptr<google::protobuf::Message> cSec1000dProtobufWrapper::byteArrayToProtobuf(QByteArray bA)
{
    ProtobufMessage::NetMessage *intermediate = new ProtobufMessage::NetMessage();
    if(!intermediate->ParseFromArray(bA, bA.size()))
    {
        ProtobufMessage::NetMessage::ScpiCommand *cmd = intermediate->mutable_scpi();
        cmd->set_command(bA.data(), bA.size() );
    }
    std::shared_ptr<google::protobuf::Message> proto {intermediate};
    return proto;
}


QByteArray cSec1000dProtobufWrapper::protobufToByteArray(const google::protobuf::Message &pMessage)
{
    return QByteArray(pMessage.SerializeAsString().c_str(), pMessage.ByteSize());
}


