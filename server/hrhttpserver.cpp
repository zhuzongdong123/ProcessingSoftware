#include "hrhttpserver.h"
#include "identitycontroller.h"
#include <QJsonDocument>
#include "appglobal.h"
#include <QThread>
hrHttpserver::hrHttpserver(QObject *parent)
    :HttpRequestHandler(parent)
{

}

hrHttpserver::~hrHttpserver()
{

}
//控制模块分发
void hrHttpserver::service(HttpRequest &request, HttpResponse &response)
{
    QByteArray path=request.getPath();
    QList<QByteArray> urlList = path.split('/');
    int iUrlSize = urlList.size();
    if(iUrlSize < 5)
    {
        QJsonObject retObj;
        retObj.insert("success",false);
        retObj.insert("message",QString("url length error: %1" ).arg(QString(path)));
        retObj.insert("code",201);
        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
        return ;
    }
    if(urlList.at(1) != "controller")
    {
        QJsonObject retObj;
        retObj.insert("success",false);
        retObj.insert("message",QString("url need startwith controller: %1" ).arg(QString(path)));
        retObj.insert("code",201);
        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
        return ;
    }
    if("login" == urlList.at(2) && "dbstatus" == urlList.at(3) && "status" == urlList.at(4)) //数据库联通性
    {
        QJsonObject retObj;
        bool dbStatus = AppGlobal::Instance().dbOpenStatus();
        if(dbStatus)
        {
            retObj.insert("success",true);
            retObj.insert("message","success");
            retObj.insert("code",200);
        }
        else
        {
            retObj.insert("success",false);
            retObj.insert("message","数据库连接失败");
            retObj.insert("code",502);
        }


        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
    }
    else if("identity" == urlList.at(2) && "authentication" == urlList.at(3)) //身份验证
    {
        IdentityController().service(request,response);
    }
    else
    {
        QJsonObject retObj;
        retObj.insert("success",false);
        retObj.insert("message","no this api");
        retObj.insert("code",201);
        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
    }
}
