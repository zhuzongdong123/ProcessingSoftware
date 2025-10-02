/*
 * @file hrhttpclient.h
 * @brief 数据库访问客户端
 * @author jason
 * @date 2023-10-14
 */
#ifndef HRHTTPCLIENT_H
#define HRHTTPCLIENT_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

//*****************身份信息
//登录验证密码
#define IDENTITY_AUTHENTICATION_QUERYPWD "/controller/identity/authentication/queryPwd"
//身份信息分页查询
#define IDENTITY_AUTHENTICATION_PAGELIST "/controller/identity/authentication/pagelist"
//身份信息删除
#define IDENTITY_AUTHENTICATION_DEL "/controller/identity/authentication/del"
//身份信息更新插入
#define IDENTITY_AUTHENTICATION_ADD "/controller/identity/authentication/add"

class hrHttpClient : public QObject
{
   Q_OBJECT
public:
    hrHttpClient();
    ~hrHttpClient();
    /*
    * @Function  httpPost
    * @Description 数据库访问同步方法
    * @param 入参(in) QString url  http的访问地址
    * @param 入参(in) QJsonObject body  参数
    * @param 入参(in) int timeout  超时时间
    * @return  返回值(return)  QJsonObject 返回值
    */
    static QJsonObject httpPost(QString url, QJsonObject body = QJsonObject(),int timeout = 3000,QString addx = "");

    /*
    * @Function  httpGet
    * @Description htttp get method
    * @param 入参(in) QString url  http的访问地址
    * @param 入参(in) int timeout  超时时间
    * @return  返回值(return)  QJsonObject 返回值
    */
    static QJsonObject httpGet(QString url, int timeout = 3000);

    static QNetworkAccessManager m_accessManager;
    /*
    * @Function  httpPostAnsy
    * @Description 数据库访问异步方法
    * @param 入参(in) QString url  http的访问地址
    * @param 入参(in) QJsonObject body  参数
    * @param 入参(in) int timeout  超时时间
    * @return  返回值(return)  QJsonObject 返回值
    */
    bool httpPostAnsy(QString url, QJsonObject body = QJsonObject(),int timeout = 5000,QString addx = "");

signals:
    void sig_visitFinish(QString url,QJsonObject byte);
};

#endif // HRHTTPCLIENT_H
