
/**
 * @file identitycontroller.h
 * @brief   身份信息相关数据库操作
 * @author 马存亮
 * @date 2023-10-20
 */

#ifndef IDENTITYCONTROLLER_H
#define IDENTITYCONTROLLER_H

#include "httprequesthandler.h"
#include "identitydao.h"
using namespace stefanfrings;
class IdentityController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(IdentityController)
public:
    explicit IdentityController(QObject *parent = nullptr);
    void service(HttpRequest& request, HttpResponse& response);
private:
    IdentityDao dao;
signals:

};

#endif // IDENTITYCONTROLLER_H
