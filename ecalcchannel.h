#ifndef ECALCCHANNEL_H
#define ECALCCHANNEL_H


#include <QObject>
#include <QList>
#include <QByteArray>

#include "scpiconnection.h"
#include "ecalcsettings.h"
#include "notificationvalue.h"


#define baseChnName "ec"

// some bit definitions
#define en_n 5
#define en_p 6
#define direction 12
#define single 13
#define sssto 14
#define ssarm 16
#define imp_en 18
#define impout_en 19
#define cnt2carry 20

// some cmdid definitions
const quint32 conf[]=
{
    // mrate counter for error measurement or energy comparison in single mode
    1<<en_n + 1<<single + 1<<cnt2carry,
    // mrate counter for error measurement or energy comparison in continous mode
    1<<en_n + 1<<cnt2carry,
    // vi counter for error measurement or energy comparison
    1<<en_n + 1<<direction + 1<<single + 3<<sssto + 2<<ssarm
};

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
    stop,
    intAcknowledge
};

}

class cSEC1000dServer;
class cECalculatorSettings;
class cFPGASettings;
class cInputSettings;
class cProtonetCommand;

class cECalculatorChannel : public cSCPIConnection
{
    Q_OBJECT

public:
    cECalculatorChannel(cSEC1000dServer* server, cECalculatorSettings* esettings, cFPGASettings* fsettings, cInputSettings* inpsettings, quint16 nr);
    ~cECalculatorChannel();
    virtual void initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface);

    QString& getName();
    bool isfree();
    bool set(QByteArray id);
    void free();
    void setIntReg(quint8 reg);
    //void clrIntReg(quint8 reg);

    void m_StopErrorCalculator();
    void m_resetInterrupt(quint8 interrupt);

protected slots:
    virtual void executeCommand(int cmdCode, cProtonetCommand* protoCmd);

private:
    cSEC1000dServer* m_pMyServer;
    cECalculatorSettings* m_pecalcsettings;
    cFPGASettings* m_pfpgasettings;
    cInputSettings* m_pInputSettings;
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
    void m_resetInt(cProtonetCommand* protoCmd);

    cNotificationValue notifierECalcChannelIntReg;
};

#endif // ECALCCHANNEL_H
