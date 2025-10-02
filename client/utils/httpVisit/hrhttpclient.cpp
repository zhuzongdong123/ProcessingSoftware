#include "hrhttpclient.h"
QNetworkAccessManager hrHttpClient::m_accessManager(0);
hrHttpClient::hrHttpClient()
{

}

hrHttpClient::~hrHttpClient()
{
    qDebug() << "release";
}


QJsonObject hrHttpClient::httpPost(QString url, QJsonObject body, int timeout,QString addx )
{
    QJsonObject ret ;
    QNetworkAccessManager manage;
    QNetworkRequest requset;
    requset.setRawHeader("Content-Type","application/json");
    QUrl qurl;
    if(addx.isEmpty())
    {
        return QJsonObject();
    }
    qurl.setUrl(addx + url);
    requset.setUrl(qurl);
    QByteArray baBody = QJsonDocument(body).toJson();
    QNetworkReply *replayEx = manage.post(requset,baBody);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeout);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(replayEx, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();
    if (timer.isActive())
    {  // 处理响应
        timer.stop();
        if (replayEx->error() == QNetworkReply::NoError) {
            QByteArray byRet = replayEx->readAll();
            ret = QJsonDocument::fromJson(byRet).object();
        }
        else
        {
            ret.insert("code",502);
        }
    }
    else
    {  // 处理超时
        disconnect(replayEx, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        if(replayEx->isRunning())
        {
            replayEx->abort();
        }
        ret.insert("code",302);
    }
    replayEx->deleteLater();
    return ret;
}

bool hrHttpClient::httpPostAnsy(QString url, QJsonObject body,int timeout,QString addx)
{
    bool ret = true;
    QNetworkRequest requset;
    requset.setRawHeader("Content-Type","application/json");
    QUrl qurl;
    if(addx.isEmpty())
    {
        return false;
    }
    qurl.setUrl(addx + url);
    requset.setUrl(qurl);
    QByteArray baBody = QJsonDocument(body).toJson();
    QNetworkReply *replayEx = hrHttpClient::m_accessManager.post(requset,baBody);

    QTimer* timer = new QTimer();
    timer->setInterval(timeout);  // 设置超时时间 3 秒
    timer->setSingleShot(true);  // 单次触发
    connect(timer, &QTimer::timeout, this,[=]()
    {
        disconnect(replayEx,&QNetworkReply::finished,this,nullptr);
        QString url = replayEx->url().toString();
        QJsonObject obj;
        obj.insert("code",302);
        obj.insert("message","超时");
        obj.insert("sender",body);
        if (replayEx->error() == QNetworkReply::NoError)
        {
            replayEx->abort();
        }
        emit sig_visitFinish(url,obj);
        replayEx->deleteLater();
        timer->deleteLater();
    });
    timer->start();

    connect(replayEx,&QNetworkReply::finished,this,[=]()
    {
        if(timer->isActive())
        {
            timer->stop();
            timer->deleteLater();
        }
        QByteArray ret ;
        QString url = replayEx->url().toString();
        QJsonObject obj;
        obj.insert("code",200);
        if (replayEx->error() == QNetworkReply::NoError)
        {
            ret = replayEx->readAll();
            if(!ret.isEmpty())
            {
                obj = QJsonDocument::fromJson(ret).object();
            }
        }
        else
        {
            obj.insert("code",502);
        }
        obj.insert("sender",body);
        emit sig_visitFinish(url,obj);
        replayEx->deleteLater();
    });

    return ret;
}

QJsonObject hrHttpClient::httpGet(QString url, int timeout)
{
    QJsonObject ret ;
    QNetworkAccessManager manage;
    QNetworkRequest requset;
    requset.setRawHeader("Accept","*/*");
    QUrl qurl;
    qurl.setUrl(url);
    requset.setUrl(qurl);
    QNetworkReply *replayEx = manage.get(requset);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeout);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(replayEx, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();
    if (timer.isActive())
    {
        // 处理响应
        timer.stop();
        if (replayEx->error() == QNetworkReply::NoError)
        {
            QByteArray byRet = replayEx->readAll();
            ret = QJsonDocument::fromJson(byRet).object();
        }
        else
        {
            qDebug() << replayEx->error();
            ret.insert("code",502);
        }
    }
    else
    {  // 处理超时
        disconnect(replayEx, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        if(replayEx->isRunning())
        {
            replayEx->abort();
        }
        ret.insert("code",302);
    }
    replayEx->deleteLater();
    return ret;
}

