#include "hrhttpserver.h"
#include "identitycontroller.h"
#include "annotation.h"
#include <QJsonDocument>
#include "appglobal.h"
#include <QThread>
#include <QDateTime>

bool checkTrialPeriod() {
    QSettings settings("sdgs", "ProcessingSoftWareServer");

    // 检查是否是首次运行
    if (!settings.contains("firstRunDate"))  {
        settings.setValue("firstRunDate",  QDateTime::currentDateTime().toString(Qt::ISODate));
        return true;
    }

    // 获取首次运行日期
    QDateTime firstRunDate = QDateTime::fromString(settings.value("firstRunDate").toString(),  Qt::ISODate);
    QDateTime currentDate = QDateTime::currentDateTime();

    // 计算试用期剩余天数
    qint64 daysElapsed = firstRunDate.daysTo(currentDate);
    qint64 trialDays = 30;
    if (daysElapsed >= trialDays) {
        return false;
    } else {
        return true;
    }
}

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

    if(!checkTrialPeriod())
    {
        QJsonObject retObj;
        retObj.insert("message","授权到期");
        retObj.insert("code",500);
        QJsonObject dataObj;
        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
        return ;
    }

    //////////////////////////////////////
    if(path.contains("/api/v1/system/info/percent"))
    {
        QJsonObject retObj;
        retObj.insert("status","sucess");
        retObj.insert("message","操作成功");
        retObj.insert("code",200);
        QJsonObject dataObj;
        dataObj.insert("cpu",50);
        dataObj.insert("memory",40.8);
        dataObj.insert("disk",100);
        retObj.insert("data",dataObj);
        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
        return ;
    }
    ////////////////////////////////////////////


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
    else if("identity" == urlList.at(2) && "authentication" == urlList.at(3)) //身份验证
    {
        IdentityController().service(request,response);
    }
    else if("annotation" == urlList.at(2) && "record" == urlList.at(3)) //标注记录相关
    {
        Annotition().service(request,response);
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
