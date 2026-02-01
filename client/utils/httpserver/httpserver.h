/**
 * @file HttpServer.h
 * @brief
 * httpserver服务端
 * 参考https://github.com/188080501/JQHttpServer.git
 * @author 朱宗冬
 * @date 2023-04-13
 */
#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTimer>
#include <JQHttpServer>
#include <QJsonObject>

class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);

    void initializeHttpServer();

    void initializeHttpsServer();

    static HttpServer *getInstance();

signals:
    void sig_sendRcvmsg(QJsonObject obj);

};

#endif // HTTPSERVER_H
