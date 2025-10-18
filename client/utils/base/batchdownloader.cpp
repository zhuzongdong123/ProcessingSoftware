#include "batchdownloader.h"
#include <QPointF>
#include <QDebug>

QJsonArray DownloadTask::handleEvents()
{
    qDebug() << "DownloadTask::handleEvents() start" << m_bagId << m_imageId;
    //从业务数据库中获取所有的事件
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    QUrlQuery m_postData;
    m_postData.clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",m_bagId);
    post_data.insert("image_id",m_imageId);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    QNetworkRequest request(requestUrl + API_EVENT_IMAGE_DETIAL_GET);
    // 设置SSL认证方式
    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //传输层安全协议，一般设为TlsV1_0 通用性高，具体要看服务器配置；
    sslconfig.setProtocol(QSsl::TlsV1_2);
    //配置SSL
    request.setSslConfiguration(sslconfig);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_manager->post(request,m_postData.toString(QUrl::FullyEncoded).toUtf8());
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(3000);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    QJsonObject returnObj;
    QJsonArray returnArray;
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
        QJsonArray dataArray = obj.value("data").toArray();
        if(dataArray.size() > 0)
        {
            for(auto data : dataArray)
            {
                QString bagId = data.toObject().value("bag_id").toString();
                QString imageId = data.toObject().value("image_id").toString();
                QString strData = data.toObject().value("data").toString();
                QJsonDocument doc = QJsonDocument::fromJson(strData.toUtf8());
                auto eventArray = doc.array();
                for(auto event : eventArray)
                {
                    double x1 = event.toObject().value("x1").toDouble();
                    double y1 = event.toObject().value("x2").toDouble();
                    double x2 = event.toObject().value("y1").toDouble();
                    double y2 = event.toObject().value("y2").toDouble();
                    //获取经纬度，
                    qDebug() << "DownloadTask::getLonLatFromServer() start" << m_bagId << m_imageId << QPointF(x1,y1) << QPointF(x2,y2);
                    QPointF lonlat = getLonLatFromServer(bagId,imageId,QPointF(x1,y1),QPointF(x2,y2));
                    qDebug() << "DownloadTask::getLonLatFromServer() end" << m_bagId << m_imageId << lonlat;
                    //获取尺寸
                    QString eventName = event.toObject().value("event_type").toString();
                    QString scale;

                    if(eventName == "pit")
                        eventName = "坑槽";
                    if(eventName == "litter")
                        eventName = "洒落物";
                    if(eventName == "strip_patch")
                        eventName = "修补";
                    if(eventName == "lane_gap")
                        eventName = "标线缺损";

                    //只有裂缝、坑槽、洒落物、修补、标线缺损需要尺寸
                    if(eventName == "裂缝" || eventName == "坑槽" || eventName == "洒落物"
                            || eventName == "修补" || eventName == "标线缺损")
                    {
                        qDebug() << "DownloadTask::getScaleFromServer() start" << m_bagId << m_imageId << QPointF(x1,y1) << QPointF(x2,y2);
                        scale = getScaleFromServer(bagId,imageId,QPointF(x1,y1),QPointF(x2,y2));
                        qDebug() << "DownloadTask::getScaleFromServer() end" << m_bagId << m_imageId << scale;
                    }
                    //获取方向 todo
                    if(lonlat.x() > 0 && lonlat.y() > 0)
                    {
                        //插入到缓存中
                        returnObj.insert("eventName",eventName);
                        returnObj.insert("bag_id",bagId);
                        returnObj.insert("imageId",bagId);
                        returnObj.insert("lon",lonlat.x());
                        returnObj.insert("lat",lonlat.y());
                        returnObj.insert("sacle",scale);
                        returnObj.insert("x1",x1);
                        returnObj.insert("y1",y1);
                        returnObj.insert("x2",x2);
                        returnObj.insert("y2",y2);
                        returnArray.push_back(returnObj);
                    }
                }
            }
        }
    }
    else
    {
        qDebug() << "DownloadTask::handleEvents() error" << m_bagId << m_imageId;
    }
    qDebug() << "DownloadTask::handleEvents() end" << m_bagId << m_imageId;
    return returnArray;
}

QPointF DownloadTask::getLonLatFromServer(QString bagId, QString imageId, QPointF pos1,QPointF pos2)
{
    QPointF returnResult(0,0);
    //从业务数据库中获取所有的事件
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    QUrlQuery m_postData;
    m_postData.clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",bagId);
    post_data.insert("fileName",imageId + ".jpg");
    QJsonObject temp;
    temp.insert("x",int((pos1.x() + pos2.x()/2)));
    temp.insert("y",int((pos1.y() + pos2.y()/2)));
    QJsonArray pixel_list;
    pixel_list.push_back(temp);
    post_data.insert("pixel_list",pixel_list);
    document.setObject(post_data);
    QNetworkRequest request(requestUrl + API_EVENT_CALC_LATLON_GET);
    // 设置SSL认证方式
    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //传输层安全协议，一般设为TlsV1_0 通用性高，具体要看服务器配置；
    sslconfig.setProtocol(QSsl::TlsV1_2);
    //配置SSL
    request.setSslConfiguration(sslconfig);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_manager->post(request,m_postData.toString(QUrl::FullyEncoded).toUtf8());
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(3000);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    QJsonObject returnObj;
    QStringList requestUrlList;
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
       QJsonArray arrayResult = obj.value("data").toArray();
       if(arrayResult.size() > 0)
       {
            returnResult = QPointF(arrayResult[0].toObject().value("lon").toDouble(),arrayResult[0].toObject().value("lat").toDouble());
       }
    }
    return returnResult;
}

QString DownloadTask::getScaleFromServer(QString bagId, QString imageId, QPointF pos1,QPointF pos2)
{
    QString returnResult = "unknown";
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    QUrlQuery m_postData;
    m_postData.clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",bagId);
    post_data.insert("fileName",imageId + ".jpg");
    QJsonObject temp;
    temp.insert("x1",int(pos1.x()));
    temp.insert("y1",int(pos1.y()));
    temp.insert("x2",int(pos2.x()));
    temp.insert("y2",int(pos2.y()));
    QJsonArray pixel_list;
    pixel_list.push_back(temp);
    post_data.insert("pixel_list",pixel_list);
    document.setObject(post_data);
    QNetworkRequest request(requestUrl + API_EVENT_CALC_SCALE_GET);
    // 设置SSL认证方式
    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //传输层安全协议，一般设为TlsV1_0 通用性高，具体要看服务器配置；
    sslconfig.setProtocol(QSsl::TlsV1_2);
    //配置SSL
    request.setSslConfiguration(sslconfig);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_manager->post(request,m_postData.toString(QUrl::FullyEncoded).toUtf8());
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(3000);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    QJsonObject returnObj;
    QStringList requestUrlList;
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
        {
           QJsonArray arrayResult = obj.value("data").toArray();
           if(arrayResult.size() > 0 && arrayResult[0].toObject().value("scale").toDouble())
           {
                returnResult = QString::number(arrayResult[0].toObject().value("scale").toDouble(),'f',1);
           }
        }
    }
    return returnResult;
}


QJsonArray BatchDownloader::getAllExportInfo()
{
    QMutexLocker locker(&m_mutex);
    return m_allExportInfo;
}
