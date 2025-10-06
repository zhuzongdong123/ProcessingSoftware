#include "appdatabasebase.h"

//获取单例类的实例
AppDatabaseBase *AppDatabaseBase::getInstance()
{
    //qDebug() << "begin";
    AppDatabaseBase* pAppDatabaseBase = nullptr;
    static AppDatabaseBase appConfigBase;
    pAppDatabaseBase = &appConfigBase;
    //qDebug() << "end " << pAppDatabaseBase;
    return pAppDatabaseBase;
}

QString AppDatabaseBase::getBusinessServerUrl()
{
    return QString("http://%1:%2").arg(m_serverIp).arg(m_businessPort);
}

QString AppDatabaseBase::getBagServerUrl()
{
    return QString("http://%1:%2").arg(m_serverIp).arg(m_bagPort);
}
