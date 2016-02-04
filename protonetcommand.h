#ifndef PROTONETCOMMAND_H
#define PROTONETCOMMAND_H

#include <QByteArray>
#include <QString>

class ProtoNetPeer;

class cProtonetCommand
{
public:
    cProtonetCommand(ProtoNetPeer* peer, bool hasClientId, bool withOutput, QByteArray clientid, quint32 messagenr ,QString input, quint8 scpitype);
    cProtonetCommand(const cProtonetCommand* protoCmd);
    ProtoNetPeer* m_pPeer;
    bool m_bhasClientId;
    bool m_bwithOutput;
    QByteArray m_clientId;
    quint32 m_nmessageNr;
    QString m_sInput;
    QString m_sOutput;
    quint8 m_nSCPIType;
};

#endif // PROTONETCOMMAND_H
