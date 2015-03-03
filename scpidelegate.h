#ifndef SCPIDELEGATE_H
#define SCPIDELEGATE_H

#include <QObject>

#include "scpiobject.h"

class QString;
class cSCPI;
class cProtonetCommand;

class cSCPIDelegate: public QObject, public cSCPIObject
{
   Q_OBJECT

public:
    cSCPIDelegate(QString cmdParent, QString cmd, quint8 type, cSCPI *scpiInterface, quint16 cmdCode);
    virtual bool executeSCPI(const QString& sInput, QString& sOutput);
    virtual bool executeSCPI(cProtonetCommand* protoCmd);

signals:
    void execute(int cmdCode, QString& sInput, QString& sOutput);
    void execute(int cmdCode, cProtonetCommand* protoCmd);

private:
    quint16 m_nCmdCode;
};


#endif // SCPIDELEGATE_H
