#ifndef SEC1000DPROTOBUFWRAPPER_H
#define SEC1000DPROTOBUFWRAPPER_H

#include <protonetwrapper.h>

class cSec1000dProtobufWrapper : public ProtoNetWrapper
{
public:
  cSec1000dProtobufWrapper();
  google::protobuf::Message *byteArrayToProtobuf(QByteArray bA);
  QByteArray protobufToByteArray(google::protobuf::Message *pMessage);
};

#endif // SEC1000DPROTOBUFWRAPPER_H
