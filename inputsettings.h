#ifndef INPUTSETTINGS
#define INPUTSETTINGS


#include <QObject>
#include <QList>
#include <QHash>

#include "xmlsettings.h"

namespace InputSettings
{
enum configstate
{
    setnumber,
    setinputname1 = 16,
    setinputmuxer1 = 48
};
}

namespace Zera
{
namespace XMLConfig
{
    class XMLConfigReader;
}
}


struct cInputInfo
{
    QString m_sName;
    quint8 m_nMux;
};


class cInputSettings : public cXMLSettings
{
    Q_OBJECT

public:
    cInputSettings(Zera::XMLConfig::cReader *xmlread);

    quint16 count();
    QList<cInputInfo>& getList();
    QString nameList();
    qint8 mux(QString name);

public slots:
    virtual void configXMLInfo(QString key);

private:
    quint16 m_nCount;
    QList<cInputInfo> inputInfoList;
};

#endif // INPUTSETTINGS

