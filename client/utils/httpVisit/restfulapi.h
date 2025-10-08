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

//*****************系统信息相关
//获取全部系统信息(简化版)
#define API_PERCENT_INFO_GET "/api/v1/system/info/percent"

//*****************bag文件相关
//获取bag列表
#define API_BAG_LIST_GET "/api/v1/bags/"
//处理bag文件
#define API_BAG_FILE_HANDLE "/api/v1/bags/process/%1"
//获取bag文件详情
#define API_BAG_FILE_DETIAL "/api/v1/bags/%1"
//删除bag文件
#define API_BAG_FILE_DEL "/api/v1/bags/delete/%1"
//获取所有文件的标注状态
#define API_BAG_ANNOTATION_STATUS_GET_ALL "/api/v1/bags/annotation_status/"
//设置文件的标注状态
#define API_BAG_ANNOTATION_STATUS_SET "/api/v1/bags/%1/annotation_status"
//获取文件的标注状态
#define API_BAG_ANNOTATION_STATUS_GET "/api/v1/bags/%1/annotation_status"

//图片处理相关
//获取bag文件对于的图片列表
#define API_IMAGES_LIST_GET "/api/v1/subdata/%1/images"
//获取图片详情
#define API_IMAGE_DETIAL_GET       "/api/v1/subdata/%1/%2/images"
#define API_POINT_IMAGE_DETIAL_GET "/api/v1/subdata/%1/%2/pointclouds"
#define API_EVENT_IMAGE_DETIAL_GET "/api/v1/subdata/%1/%2/event"

/*
 *提供restfull便携接口类外使用
 * RestFullApi *myApi=new RestFullApi();
 *
 */
enum ReplyType
{
    LOGIN,//登录
    PERCENT_INFO_GET,
    BAG_LIST_GET,
    BAG_FILE_HANDLE,
    BAG_FILE_DEL,
    BAG_ANNOTATION_STATUS_GET_ALL,
    BAG_ANNOTATION_STATUS_SET,
    BAG_ANNOTATION_STATUS_GET,
    BAG_FILE_DETIAL,
    IMAGES_LIST_GET,
    IMAGE_DETIAL_GET,
    CURRENT_IMAGE_DETIAL_GET,
    POINT_IMAGE_DETIAL_GET,
    EVENT_IMAGE_DETIAL_GET
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

    QNetworkReply * visitUrl(QString api,VisitType visitType, ReplyType replyType, QString hearderInfo = "application/x-www-form-urlencoded",
                             QByteArray byte = nullptr, bool outLog = true, int timeout = 15000,QNetworkRequest::Priority priority = QNetworkRequest::Priority::NormalPriority);

    QNetworkReply *visitUrlByFormData(QString url, int replyType, QMap<QString, QString> textMap, QMap<QString, QString> fileMap, int timeout);
private:
    QUrlQuery m_postData;
    static QNetworkAccessManager m_accessManager;
};

#endif // RESTFULLAPI_H
