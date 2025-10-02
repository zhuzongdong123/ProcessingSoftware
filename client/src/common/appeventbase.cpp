#include "appeventbase.h"

//获取单例类的实例
AppEventBase *AppEventBase::getInstance()
{
    AppEventBase* pAppEventBase = nullptr;
    static AppEventBase appConfigBase;
    pAppEventBase = &appConfigBase;
    return pAppEventBase;
}

