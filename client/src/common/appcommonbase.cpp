#include <QApplication>
#include <QWidget>
#include <QDir>
#include "appcommonbase.h"

//获取单例类的实例
AppCommonBase* AppCommonBase::getInstance()
{
    AppCommonBase* pAppCommonBase = nullptr;
    static AppCommonBase appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}