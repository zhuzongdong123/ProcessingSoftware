#include "identitycontroller.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
IdentityController::IdentityController(QObject *parent) : HttpRequestHandler(parent)
{

}

void IdentityController::service(HttpRequest &request, HttpResponse &response)
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
    else if("del" == strAction) //删除
    {

        result = dao.deleteIdentityResource(obj);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
        }

    }
    else if("queryPwd" == strAction)  //验证密码
    {

        result = dao.queryPwdIdentityResource(arr, obj);
        if(!result)
        {
            msg = dao.m_lastError;
            code = 500;
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
    retObj.insert("data",arr);
    QJsonDocument doc;
    doc.setObject(retObj);
    response.write(doc.toJson());
}
