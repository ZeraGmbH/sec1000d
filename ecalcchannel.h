#ifndef ECALCCHANNEL_H
#define ECALCCHANNEL_H


#include <QObject>
#include <QList>
#include <QByteArray>

#include "scpiconnection.h"
#include "ecalcsettings.h"
#include "notificationvalue.h"


#define baseChnName "ec"

namespace ECALCREG {
    enum { CMD, CONF, STATUS, INTMASK, INTREG, MTCNTin, MTCNTfin, MTCNTact, MTPULSin = 12, MTPAUSEin, MTPULS, MTPAUSE};
}

namespace ECALCCMDID {
    enum { COUNTEDGE = 1, COUNTRESET, ERRORMEASMASTER, ERRORMEASSLAVE};
}

namespace ECalcChannel
{
enum Commands
{
    cmdRegister,
    setSync,
    setMux,
    setCmdid,
    start,
    stop
};

}

class cSEC1000dServer;
class cECalculatorSettings;
class cFPGASettings;
class cProtonetCommand;

class cECalculatorChannel : public cSCPIConnection
{
    Q_OBJECT

public:
    cECalculatorChannel(cSEC1000dServer* server, cECalculatorSettings* esettings, cFPGASettings* fsettings, quint16 nr);
    ~cECalculatorChannel();
    virtual void initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface);

    QString& getName();
    bool isfree();
    bool set(QByteArray id);
    void free();
    void setIntReg(quint8 reg);
    void clrIntReg(quint8 reg);

protected slots:
    virtual void executeCommand(int cmdCode, cProtonetCommand* protoCmd);

private:
    cSEC1000dServer* m_pMyServer;
    cECalculatorSettings* m_pecalcsettings;
    cFPGASettings* m_pfpgasettings;
    quint16 m_nNr;
    quint32 m_nBaseAdress;
    quint32 m_nMyAdress;

    QString m_sName; // the channels name ec0...
    bool m_bSet; // we mark if the channel is occupied
    QByteArray m_ClientId; // we remark the clientid for arbitration purpose
    void m_ReadWriteRegister(cProtonetCommand* protoCmd);
    void m_setSync(cProtonetCommand* protoCmd);
    void m_setMux(cProtonetCommand* protoCmd);
    void m_setCmdId(cProtonetCommand* protoCmd);
    void m_start(cProtonetCommand* protoCmd);
    void m_stop(cProtonetCommand* protoCmd);

    cNotificationValue notifierECalcChannelIntReg;
    void setNotifierECalcChannelIntReg();
};

#endif // ECALCCHANNEL_H
