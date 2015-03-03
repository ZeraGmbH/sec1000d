#ifndef ECALCINTERFACE_H
#define ECALCINTERFACE_H

#include <QObject>
#include <QList>
#include <QHash>

#include "resource.h"
#include "scpiconnection.h"
#include "ecalcchannel.h"
#include "notificationstring.h"

namespace ECalcSystem
{

const QString Version = "V1.00";
const QString sECalculatorDescription = "Standard error calculator base unit";

enum Commands
{
    cmdVersion,
    cmdChannelCat,
    cmdSetChannels,
    cmdFreeChannels
};

}

class cSEC1000dServer;
class cETHSettings;
class cECalculatorChannel;
class cECalculatorSettings;
class cFPGASettings;
class cProtonetCommand;


class cECalculatorInterface : public cResource
{
    Q_OBJECT

public:
    cECalculatorInterface(cSEC1000dServer* server, cETHSettings* ethsettings, cECalculatorSettings* ecalcSettings, cFPGASettings* fpgasettings);
    ~cECalculatorInterface();
    virtual void initSCPIConnection(QString leadingNodes, cSCPI* scpiInterface);
    virtual void registerResource(cRMConnection *rmConnection, quint16 port);
    virtual void unregisterResource(cRMConnection *rmConnection);

protected slots:
    virtual void executeCommand(int cmdCode, cProtonetCommand* protoCmd);

private:
    cSEC1000dServer* m_pMyServer;
    cETHSettings* m_pETHsettings;
    cECalculatorSettings* m_pecalcsettings;
    cFPGASettings* m_pfpgasettings;

    QString m_sVersion;

    QHash<QString,cECalculatorChannel*> m_ECalculatorChannelHash;
    QHash<QByteArray, QString> m_ClientECalcHash; // we hold the set ecalculators by clientid

    QString m_ReadVersion(QString& sInput);
    QString m_ReadECalculatorChannelCatalog(QString& sInput);
    void m_SetChannels(cProtonetCommand *protoCmd);
    void m_FreeChannels(cProtonetCommand *protoCmd);
};

#endif // ECALCINTERFACE_H
