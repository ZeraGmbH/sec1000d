#include <xmlconfigreader.h>

#include "inputsettings.h"


cInputSettings::cInputSettings(Zera::XMLConfig::cReader *xmlread)
{
    m_pXMLReader = xmlread;

    m_ConfigXMLMap[QString("sec1000dconfig:connectivity:inputs:n")] = InputSettings::setnumber;
    // for the inputs we generate new entries dynamically
}


quint16 cInputSettings::count()
{
    return m_nCount;
}


QList<cInputInfo> &cInputSettings::getList()
{
    return inputInfoList;
}


QString cInputSettings::nameList()
{
    QString s;

    for (int i = 0; i < inputInfoList.count(); i++)
    {
        s += (inputInfoList.at(i).m_sName + ";");
    }

    return s;
}


void cInputSettings::configXMLInfo(QString key)
{
    bool ok;

    if (m_ConfigXMLMap.contains(key))
    {
        int cmd = m_ConfigXMLMap[key];
        switch (cmd)
        {
        case InputSettings::setnumber:
        {
            cInputInfo iInfo;

            m_nCount = m_pXMLReader->getValue(key).toInt(&ok);
            for (int i = 0; i < m_nCount; i++)
            {
                m_ConfigXMLMap[QString("sec1000dconfig:connectivity:inputs:inp%1:name").arg(i+1)] = InputSettings::setinputname1+i;
                m_ConfigXMLMap[QString("sec1000dconfig:connectivity:inputs:inp%1:muxer").arg(i+1)] = InputSettings::setinputname1+i;
                inputInfoList.append(iInfo);
            }
        }
        default:
        {
            cInputInfo iInfo;

            if (cmd >= InputSettings::setinputname1 && cmd < InputSettings::setinputname1 + 32)
            {
                cmd -= InputSettings::setinputname1;
                iInfo = inputInfoList.at(cmd);
                iInfo.m_sName = m_pXMLReader->getValue(key);
                inputInfoList.replace(cmd, iInfo);
            }
            else
                if (cmd >= InputSettings::setinputmuxer1 && cmd < InputSettings::setinputmuxer1 + 32)
                {
                    cmd -= InputSettings::setinputmuxer1;
                    iInfo = inputInfoList.at(cmd);
                    iInfo.m_nMux = m_pXMLReader->getValue(key).toInt(&ok);
                    inputInfoList.replace(cmd, iInfo);
                }
        }
        }
    }
}
