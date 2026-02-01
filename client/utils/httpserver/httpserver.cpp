#include "httpserver.h"
#include <JQHttpServer>
#include <QJsonObject>
#include <QJsonDocument>
#include "appconfigbase.h"

HttpServer::HttpServer(QObject *parent) : QObject(parent)
{
}

HttpServer *HttpServer::getInstance()
{
    static HttpServer server;
    return &server;
}

void onHttpAccepted(const QPointer< JQHttpServer::Session > &session)
{
    // 回调发生在专用的处理线程内，不是主线程，也不是socket的生存线程，请注意线程安全
    // 若阻塞了此回调，那么新的连接将被阻塞（默认情况下有2个线程可以阻塞2次，第3个连接将被阻塞）
    //第三方推送访客通行结果
     if(session->requestUrl().contains("/pull_result"))
     {
         QString requestUrlPath =  session->requestUrlPath();
         QString requestSourceIp =  session->requestSourceIp();

        // QByteArray转换成QJsonObject
        QJsonDocument document2=QJsonDocument::fromJson(session->requestBody());
        QJsonObject object2 = document2.object();
        session->replyJsonObject( {{ "status", "0" }} );
        //qDebug() << object2;
        emit HttpServer::getInstance()->sig_sendRcvmsg(object2);
     }
     else
     {
         QJsonDocument document2=QJsonDocument::fromJson(session->requestBody());
         QJsonObject object2 = document2.object();
         session->replyJsonObject(object2);
         qDebug() << "http收到信息："<< object2;
     }
}

void HttpServer::initializeHttpServer()
{
    static JQHttpServer::TcpServerManage tcpServerManage( 8 ); // 设置最大处理线程数，默认2个
    tcpServerManage.setHttpAcceptedCallback( std::bind( onHttpAccepted, std::placeholders::_1 ) );

    int port = AppConfigBase::getInstance()->readConfigSettings("yoloListen","port","8899").toInt();
    const auto listenSucceed = tcpServerManage.listen(QHostAddress::LocalHost, port );
    qDebug() << "HTTP server listen:" << listenSucceed;
}

void HttpServer::initializeHttpsServer()
{
#ifndef QT_NO_SSL
    static JQHttpServer::SslServerManage sslServerManage( 2 ); // 设置最大处理线程数，默认2个
    sslServerManage.setHttpAcceptedCallback( std::bind( onHttpAccepted, std::placeholders::_1 ) );

    const auto listenSucceed = sslServerManage.listen( QHostAddress::Any, 23413, ":/server.crt", ":/server.key" );
    qDebug() << "HTTPS server listen:" << listenSucceed;

    // 这是我在一个实际项目中用的配置（用认证的证书，访问时带绿色小锁），其中涉及到隐私的细节已经被替换，但是任然能够看出整体用法
    /*
    qDebug() << "listen:" << sslServerManage.listen(
                    QHostAddress::Any,
                    24684,
                    "xxx/__xxx_com.crt",
                    "xxx/__xxx_com.key",
                    {
                        { "xxx/__xxx_com.ca-bundle", QSsl::Pem },
                        { "xxx/COMODO RSA Certification Authority.crt", QSsl::Pem },
                        { "xxx/COMODO RSA Domain Validation Secure Server CA.cer", QSsl::Der }
                    }
                );
    */
#else
    qDebug() << "SSL not support"
#endif
}
