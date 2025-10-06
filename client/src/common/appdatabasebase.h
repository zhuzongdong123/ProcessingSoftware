/**
 * @file appdatabasebase.h
 * @brief 数据库管理单类基类
 * @author 朱宗冬
 * @date 2023-09-21
 */
#ifndef APPDATABASEBASE_H
#define APPDATABASEBASE_H

#include <QObject>
#include <QDebug>
class AppDatabaseBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief getInstance 获取单例类的实例
     * @return AppDatabaseBase* 返回类的实例指针
     */
    static AppDatabaseBase* getInstance();

    //获取业务服务器的http请求路径
    QString getBusinessServerUrl();

    //获取bag处理服务器的请求路径
    QString getBagServerUrl();

    //用户信息
    QString m_userName;
    QString m_userId;
    QString m_userType;
    QString m_serverIp;
    QString m_businessPort;
    QString m_bagPort;
};

#endif // APPDATABASEBASE_H
