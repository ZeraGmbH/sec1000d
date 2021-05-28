#ifndef SEC1000DPROTOBUFWRAPPER_H
#define SEC1000DPROTOBUFWRAPPER_H

#include <xiqnetwrapper.h>

class cSec1000dProtobufWrapper : public XiQNetWrapper
{
public:
    cSec1000dProtobufWrapper();
    std::shared_ptr<google::protobuf::Message> byteArrayToProtobuf(QByteArray bA) override;
};

#endif // SEC1000DPROTOBUFWRAPPER_H
