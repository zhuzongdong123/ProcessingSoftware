#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "appconfigbase.h"

AppConfigBase::AppConfigBase(QObject *parent) : QObject(parent)
{

}

//获取单例类的实例
AppConfigBase *AppConfigBase::getInstance()
{
    AppConfigBase* pAppConfigBase = nullptr;
    static AppConfigBase appConfigBase;
    pAppConfigBase = &appConfigBase;
    return pAppConfigBase;
}

//读取某一个项值
QString AppConfigBase::readConfigSettings(QString section, QString key, QString default_value)
{
    if(nullptr == m_configIniRead)
    {
        return "";
    }

    QString value = m_configIniRead->value("/" + section + "/" + key).toString();
    if(value == "") {
        value = default_value;
    }
    return value;
}

QString AppConfigBase::readCameraSettings(QString section, QString key, QString default_value)
{
    if(nullptr == m_cameraIniRead)
    {
        return "";
    }

    QString value = m_cameraIniRead->value("/" + section + "/" + key).toString();
    if(value == "") {
        value = default_value;
    }
    return value;
}

//读取配置文件
void AppConfigBase::readConfig(QString path)
{
    QString fileName = path;
    if("" == fileName)
        fileName = QCoreApplication::applicationDirPath() + QString("/%1.ini").arg(QCoreApplication::applicationName());

    //判断文件是否存在
    if(nullptr != m_configIniRead)
        m_configIniRead->deleteLater();
    m_configIniRead = new QSettings(fileName, QSettings::IniFormat,this);
    m_configIniRead->setIniCodec(QTextCodec::codecForName("utf-8"));
}

void AppConfigBase::readCameraConfig(QString path)
{
    QString fileName = path;
    if("" == fileName)
        fileName = QCoreApplication::applicationDirPath() + QString("/%1.ini").arg("camera");

    //判断文件是否存在
    if(nullptr != m_cameraIniRead)
        m_cameraIniRead->deleteLater();
    m_cameraIniRead = new QSettings(fileName, QSettings::IniFormat,this);
    m_cameraIniRead->setIniCodec(QTextCodec::codecForName("utf-8"));
}

//更新某一个项值
void AppConfigBase::updateConfigSetting(QSettings* setting, QString section, QString key, const char* value)
{
    QString returnValue = setting->value("/" + section + "/" + key).toString();
    if(returnValue == "")
        setting->setValue("/" + section + "/" + key, value);
}
void AppConfigBase::updateConfigSetting(QString section, QString key, QString value)
{
    QString returnValue = m_configIniRead->value("/" + section + "/" + key).toString();
    if(returnValue == "" || returnValue != QString(value))
        m_configIniRead->setValue("/" + section + "/" + key, value);
    m_configIniRead->sync();//写入配置文件
}
