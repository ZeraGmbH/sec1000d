#ifndef ECALCCHANNEL_H
#define ECALCCHANNEL_H


#include <QObject>
#include <QList>
#include <QByteArray>

#include "scpiconnection.h"
#include "ecalcsettings.h"
#include "notificationvalue.h"


#define CMDREG 0x0
#define CONFREG 0x4
#define STATUSREG 0x8
#define INTMASKREG 0xC
#define INTREG 0x10
#define COUNTPRESET 0x14
#define COUNTLATCH 0x18
#define COUNTACTUAL 0x1C

namespace ECalcChannel
{
enum Commands
{
    cmdCmd,
    cmdConf,
    cmdStatus,
    cmdIntmask,
    cmdIntreg,
    cmdCountPreset,
    cmdCountLatch,
    cmdCountActual
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
    void m_ReadWriteCMD(cProtonetCommand* protoCmd);
    void m_ReadWriteCONF(cProtonetCommand* protoCmd);
    void m_ReadSTATUS(cProtonetCommand* protoCmd);
    void m_ReadWriteINTMASK(cProtonetCommand* protoCmd);
    void m_ReadWriteINTREG(cProtonetCommand* protoCmd);
    void m_ReadWriteCOUNTPRESET(cProtonetCommand* protoCmd);
    void m_ReadCOUNTLATCH(cProtonetCommand* protoCmd);
    void m_ReadCOUNTACTUAL(cProtonetCommand* protoCmd);
    void m_ReadWriteRegister(cProtonetCommand* protoCmd, quint32 adroffs);
    void m_ReadRegister(cProtonetCommand* protoCmd, quint32 adroffs);

    cNotificationValue notifierECalcChannelIntReg;
    void setNotifierECalcChannelIntReg();
};

#endif // ECALCCHANNEL_H
