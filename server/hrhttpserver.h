#ifndef HRHTTPSERVER_H
#define HRHTTPSERVER_H
/*
 * @file hrhttpserver.h
 * @brief 总体控制模块，分发处理模块
 * @author jason
 * @date 2023-10-17
 */
#include "httprequesthandler.h"
using namespace stefanfrings;
class hrHttpserver : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(hrHttpserver)
public:
    hrHttpserver(QObject* parent=0);
    ~hrHttpserver();
    /*
    * @Function  service
    * @Description 具体业务处理模块
    * @param 入参(in) HttpRequest request  调用请求类，包含请求信息
    * @param 入参(in) HttpResponse response  返回数据类，通过此对象返回数据
    */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // HRHTTPSERVER_H
