#ifndef ECALCCHANNEL_H
#define ECALCCHANNEL_H

#include <QObject>
#include <QList>
#include <QByteArray>

#include "scpiconnection.h"
#include "ecalcsettings.h"
#include "notificationstring.h"


#define CMDREG 0x0
#define CONFREG 0x4
#define STATUSREG 0x8
#define INTMASKREG 0xC
#define INTREG 0x10


namespace ECalcChannel
{
enum Commands
{
    cmdCmd,
    cmdConf,
    cmdStatus,
    cmdIntmask,
    cmdIntreg
};

}

class cECalculatorSettings;
class cFPGASettings;
class cProtonetCommand;

class cECalculatorChannel : public cSCPIConnection
{
    Q_OBJECT

public:
    cECalculatorChannel(cECalculatorSettings* esettings, cFPGASettings* fsettings, quint16 nr);
    ~cECalculatorChannel();
    virtual void initSCPIConnection(QString leadingNodes, cSCPI *scpiInterface);

    QString& getName();
    bool isfree();
    bool set(QByteArray id);
    void free();

protected slots:
    virtual void executeCommand(int cmdCode, cProtonetCommand* protoCmd);

private:
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
    void m_ReadWriteRegister(cProtonetCommand* protoCmd, quint32 adroffs);

    cNotificationString notifierECalcChannelIntReg;
    void setNotifierECalcChannelIntReg();
};

#endif // ECALCCHANNEL_H
