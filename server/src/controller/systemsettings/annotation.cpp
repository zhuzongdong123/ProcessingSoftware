#include "annotation.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
Annotition::Annotition(QObject *parent) : HttpRequestHandler(parent)
{

}

void Annotition::service(HttpRequest &request, HttpResponse &response)
{
    QByteArray path=request.getPath();
    QByteArray body = request.getBody();
    QJsonObject retObj;
    QJsonArray arr;

    QJsonObject obj = QJsonDocument::fromJson(body).object();
    QString msg = "成功";
    bool result = false;
    int code = 200;

    if(!dao.openConnection(response.getThreadId()))
    {
        retObj.insert("success",false);
        retObj.insert("message",dao.m_lastError);
        retObj.insert("code",502);
        QJsonDocument doc;
        doc.setObject(retObj);
        response.write(doc.toJson());
        qDebug() << "url: " << path << "params: " << obj << "result: " << retObj;
        return ;
    }
    QList<QByteArray> urlList = path.split('/');
    QString strAction = "";
    if(urlList.size() > 4)
    {
        strAction = QString(urlList.at(4));
    }
    if("pagelist" == strAction) //分页查询
    {
        result = dao.queryIdentityResource(arr, obj);
        if(!result)
        {
            msg = dao.getLastError();
            code = 500;
        }
        else
        {
             retObj.insert("data",arr);
        }
    }
    else if("add" == strAction || "update" == strAction) //增加
    {
        result = dao.addOrUpdateIdentity(obj);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
        }
    }
    else if("addEvent" == strAction) //增加
    {
        result = dao.addEvents(obj);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
        }
    }
    else if("queryEvent" == strAction) //增加
    {
        result = dao.getAllEvents(obj,arr);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
        }
        else
        {
             retObj.insert("data",arr);
        }
    }
    else if("del" == strAction) //删除
    {
        result = dao.deleteIdentityResource(obj);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
        }
    }
    else if("query" == strAction) //查询
    {
        result = dao.queryResource(obj,arr);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
        }
        else
        {
             retObj.insert("data",arr);
        }
    }
    else
    {
        msg = "路由不存在";
        code = 500;
    }
    retObj.insert("success",result);
    retObj.insert("message",msg);
    retObj.insert("code",code);
    QJsonDocument doc;
    doc.setObject(retObj);
    response.write(doc.toJson());
}
