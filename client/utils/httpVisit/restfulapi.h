#ifndef RESTFULLAPI_H
#define RESTFULLAPI_H
#include <QUrlQuery>
#include <QObject>
#include <qnetworkaccessmanager.h>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>

//*****************身份信息
//登录验证密码
#define IDENTITY_AUTHENTICATION_QUERYPWD "/controller/identity/authentication/queryPwd"
//身份信息分页查询
#define IDENTITY_AUTHENTICATION_PAGELIST "/controller/identity/authentication/pagelist"
//身份信息删除
#define IDENTITY_AUTHENTICATION_DEL "/controller/identity/authentication/del"
//身份信息更新插入
#define IDENTITY_AUTHENTICATION_ADD "/controller/identity/authentication/add"

/*
 *提供restfull便携接口类外使用
 * RestFullApi *myApi=new RestFullApi();
 *
 */
enum ReplyType
{
    LOGIN,//登录
};

enum class VisitType
{
    GET,
    POST,
    PUT,
    DELETE
};

extern QMap<QNetworkReply *, int> replyTypeMap;
class RestFulApi:public QObject
{
    Q_OBJECT
public:
    RestFulApi();
    void downloadFileFromUrl(QByteArray byte, QString strFilePath);

    static QNetworkAccessManager& getAccessManager(){return m_accessManager;}
    QUrlQuery& getPostData(){return m_postData;}
    bool replyResultCheck(QJsonObject obj, QNetworkReply *networkReply, bool outLog = true);

    QNetworkReply * visitUrl(QString api,VisitType visitType, ReplyType replyType, QString hearderInfo = "application/x-www-form-urlencoded",QByteArray byte = nullptr, bool outLog = true, int timeout = 15000);

    QNetworkReply *visitUrlByFormData(QString url, int replyType, QMap<QString, QString> textMap, QMap<QString, QString> fileMap, int timeout);
private:
    QUrlQuery m_postData;
    static QNetworkAccessManager m_accessManager;
};

#endif // RESTFULLAPI_H
