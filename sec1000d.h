// header datei com5003.h
// globale definitionen

#ifndef SEC1000D_H
#define SEC1000D_H

#include <QTimer>

#include "pcbserver.h"

class QStateMachine;
class QState;
class cDebugSettings;
class cFPGASettings;
class cETHSettings;
class cECalculatorSettings;
class cStatusInterface;
class cSystemInterface;
class cECalculatorInterface;
class cSystemInfo;
class cRMConnection;


class cSEC1000dServer: public cPCBServer
{
    Q_OBJECT

public:
    explicit cSEC1000dServer(QObject* parent=0);
    ~cSEC1000dServer();

    cDebugSettings* m_pDebugSettings;
    cFPGASettings* m_pFPGAsettings;
    cETHSettings* m_pETHSettings;
    cECalculatorSettings* m_pECalcSettings;

    cStatusInterface* m_pStatusInterface;
    cSystemInterface* m_pSystemInterface;
    cECalculatorInterface* m_pECalculatorInterface;
    cSystemInfo* m_pSystemInfo;
    cRMConnection* m_pRMConnection;


signals:
    void abortInit();
    void confStarting();
    void confFinished();
    void serverSetup();

private:
    QStateMachine* m_pInitializationMachine;
    QState* stateconnect2RM;
    QState* stateconnect2RMError;
    QState* stateSendRMIdentandRegister;
    quint8 m_nerror;
    int m_nRetryRMConnect;
    QTimer m_retryTimer;

private slots:
    void doConfiguration();
    void doSetupServer();
    void doCloseServer();
    void doConnect2RM();
    void connect2RMError();
    void doIdentAndRegister();
};


#endif
