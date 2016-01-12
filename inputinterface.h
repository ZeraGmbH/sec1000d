#ifndef INPUTINTERFACE
#define INPUTINTERFACE

#include <QObject>
#include <scpi.h>

#include "scpiconnection.h"

namespace InputSystem
{

enum SystemCommands
{
    cmdVersionServer,
    cmdChannelCat,
    cmdChannelMux = 16
};
}

class cInputSettings;

class cInputInterface: public cSCPIConnection
{
    Q_OBJECT

public:
    cInputInterface(cInputSettings* iSettings);
    virtual void initSCPIConnection(QString leadingNodes, cSCPI* scpiInterface);

protected slots:
    virtual void executeCommand(int cmdCode, cProtonetCommand* protoCmd);

private:
    cInputSettings* m_pInputSettings;
    QString m_ReadInputChannelCatalog(QString& sInput);
    QString m_ReadInputChannelMux(quint8 pos, QString& sInput);
};

#endif // INPUTINTERFACE

