#ifndef RESOURCE_H
#define RESOURCE_H

#include <QObject>
#include <QList>

#include "scpiconnection.h"

class cRMConnection;
class QString;
class cCOM5003dServer;

// pure virtual base class for resources to register themselves anywhere

class cResource: public cSCPIConnection
{
    Q_OBJECT

public:
    cResource(QObject* parent=0);
    virtual ~cResource();
    virtual void registerResource(cRMConnection *rmConnection, quint16 port) = 0;
    virtual void unregisterResource(cRMConnection *rmConnection) = 0;
    virtual void initSCPIConnection(QString leadingNodes, cSCPI* scpiInterface) = 0;

signals:
    void registerRdy(); // we emit signal when all register or unregister action is done

protected:
    QList<quint32> msgNrList;
    cCOM5003dServer* m_pMyServer;
    void register1Resource(cRMConnection *rmConnection, quint32 msgnr, QString registerParameter);
    void unregister1Resource(cRMConnection *rmConnection, quint32 msgnr, QString unregisterParameter);

protected slots:
    void resourceManagerAck(quint32 msgnr);

};

#endif // RESOURCE_H
