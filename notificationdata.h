#ifndef NOTIFICATIONDATA_H
#define NOTIFICATIONDATA_H

#include <QByteArray>
#include <protonetpeer.h>

#include "notificationvalue.h"

struct cNotificationData
{
    ProtoNetPeer *netPeer;
    QByteArray clientID;
    cNotificationValue *notValue;
};

#endif // NOTIFICATIONDATA_H
