
/**
 * @file Annotition.h
 * @brief   标注类
 * @author zzd
 * @date 2023-10-20
 */

#ifndef Annotition_H
#define Annotition_H

#include "httprequesthandler.h"
#include "annotitiondao.h"
using namespace stefanfrings;
class Annotition : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(Annotition)
public:
    explicit Annotition(QObject *parent = nullptr);
    void service(HttpRequest& request, HttpResponse& response);
private:
    AnnotitionDao dao;
signals:

};

#endif // Annotition_H
