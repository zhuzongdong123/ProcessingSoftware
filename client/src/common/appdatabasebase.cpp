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
