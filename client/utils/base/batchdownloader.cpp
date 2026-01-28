#include "batchdownloader.h"
#include <QPointF>
#include <QDebug>


// 头文件声明
#include <QMap>
#include <QString>

// 全局静态映射
static QMap<QString, QString> en2cMapping;

// 初始化函数
void initEn2CMapping() {
    en2cMapping["pit"] = "坑槽";
    en2cMapping["litter"] = "洒落物";
    en2cMapping["strip_patch"] = "修补";
    en2cMapping["lane_gap"] = "标线缺损";
    en2cMapping["expansion_gap"] = "伸缩缝";
    en2cMapping["signboard"] = "标志牌";
    en2cMapping["hundred_pile"] = "百米桩";
    en2cMapping["km_pile"] = "公里桩";
    en2cMapping["yellow_circle_outline"] = "轮廓标";
    en2cMapping["unglare_plate"] = "防眩板";
}

// 使用示例
QString getChineseTranslation(const QString& english) {
    if(en2cMapping.size() == 0)
        initEn2CMapping();

    if(en2cMapping.find(english) != en2cMapping.end())
    {
        return en2cMapping.find(english).value();
    }
    else
    {
        return english;
    }
}


bool hasChineseCharacters(const QString& str) {
    for (const QChar& ch : str) {
        const ushort unicode = ch.unicode();
        // 核心判断逻辑：汉字 Unicode 范围
        if ((unicode >= 0x4E00 && unicode <= 0x9FFF) ||   // 基本汉字
            (unicode >= 0x3400 && unicode <= 0x4DBF) ||   // 扩展A区
            (unicode >= 0x20000 && unicode <= 0x2A6DF)) { // 扩展B区
            return true;
        }
    }
    return false;
}

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
    QNetworkRequest request(requestUrl + API_ANNOTATION_QUERY_EVENTS);
    // 设置SSL认证方式
    QSslConfiguration sslconfig;
    sslconfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    //传输层安全协议，一般设为TlsV1_0 通用性高，具体要看服务器配置；
    sslconfig.setProtocol(QSsl::TlsV1_2);
    //配置SSL
    request.setSslConfiguration(sslconfig);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_manager.post(request,post_param);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(1000*30);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &timer, &QTimer::stop);
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

                //先获取投影数据
                //获取bag文件的详情
                QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
                QString urlTemp = requestUrl + QString(API_IMAGE_DETIAL_GET).arg(bagId).arg(imageId + ".jpg");
                QNetworkRequest request(urlTemp);
                QNetworkReply* reply = m_manager.get(request);
                QEventLoop loop;
                QTimer timer;
                timer.setInterval(1000*30);  // 设置超时时间 3 秒
                timer.setSingleShot(true);  // 单次触发
                connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
                connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                connect(reply, &QNetworkReply::finished, &timer, &QTimer::stop);
                timer.start();
                loop.exec();

                if(reply->error() == QNetworkReply::NoError) {
                    QByteArray readArray = reply->readAll();
                    QString picArray = readArray;
                    QByteArray decodedData = QByteArray::fromBase64(picArray.toLatin1());
                    qDebug () << "投影数据获取完成";
                }
                else
                {
                    qDebug () << "投影数据获取失败";
                }

                auto eventArray = doc.array();

                if(eventArray.size() == 0)
                {
                    qDebug() << "该图片没有事件" << m_bagId << m_imageId  << "zzdTemp";
                }

                for(auto event : eventArray)
                {
                    double x1 = event.toObject().value("x1").toDouble();
                    double y1 = event.toObject().value("x2").toDouble();
                    double x2 = event.toObject().value("y1").toDouble();
                    double y2 = event.toObject().value("y2").toDouble();
                    //获取经纬度，
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz.ddd") << "DownloadTask::getLonLatFromServer() start" << m_bagId << m_imageId << QPointF(x1,y1) << QPointF(x2,y2);
                    QPointF lonlat = getLonLatFromServer(bagId,imageId,QPointF(x1,y1),QPointF(x2,y2));
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz.ddd") << "DownloadTask::getLonLatFromServer() end" << m_bagId << m_imageId << lonlat;

                    //经纬度为0，不处理
                    if(fabs(lonlat.x()) < 1e-5 && fabs(lonlat.y()) < 1e-5)
                    {
                        qDebug() << "当前事件不处理"  << "zzdTemp";
                        continue;
                    }

                    //获取尺寸
                    QString eventName = event.toObject().value("event_type").toString();
                    QString scale;

                    //英文名称转换成中文名称
                    eventName = getChineseTranslation(eventName);

                    if(!m_eventsSet.contains(eventName) && hasChineseCharacters(eventName))
                    {
                        qDebug() << "当前事件不处理，未找到事件名称"  << "zzdTemp" << eventName;
                        continue;
                    }

                    //只有裂缝、坑槽、洒落物、修补、标线缺损需要尺寸
                    if(eventName == "裂缝" || eventName == "坑槽" || eventName == "洒落物"
                            || eventName == "修补" || eventName == "标线缺损")
                    {
                        qDebug() << "DownloadTask::getScaleFromServer() start" << m_bagId << m_imageId << QPointF(x1,y1) << QPointF(x2,y2);
                        scale = getScaleFromServer(bagId,imageId,QPointF(x1,y1),QPointF(x2,y2));
                        qDebug() << "DownloadTask::getScaleFromServer() end" << m_bagId << m_imageId << scale;
                    }
                    //获取方向 todo
                    //if(lonlat.x() > 0 && lonlat.y() > 0)
                    {
                        //插入到缓存中
                        returnObj.insert("eventName",eventName);
                        returnObj.insert("bag_id",bagId);
                        returnObj.insert("imageId",imageId);
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
        qDebug() << "DownloadTask::handleEvents() error" << m_bagId << m_imageId  << "zzdTemp";
    }
    reply->deleteLater();
    qDebug() << "DownloadTask::handleEvents() end" << m_bagId << m_imageId;
    return returnArray;
}

QPointF DownloadTask::getLonLatFromServer(QString bagId, QString imageId, QPointF pos1,QPointF pos2)
{
//    return QPointF(0,0);

    QPointF returnResult(0,0);
    //从业务数据库中获取所有的事件
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    QUrlQuery m_postData;
    m_postData.clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",bagId);
    post_data.insert("fileName",imageId + ".jpg");
    QJsonObject temp;
    temp.insert("x",int((pos1.x() + pos2.x())/2));
    temp.insert("y",int((pos1.y() + pos2.y())/2));
    QJsonArray pixel_list;
    pixel_list.push_back(temp);
    post_data.insert("pixel_list",pixel_list);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
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
    QNetworkReply* reply = m_manager.post(request,post_param);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(1000*60*10);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &timer, &QTimer::stop);
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
       else
       {
            qDebug() << "经纬度结果获取失败" << obj << "zzdTemp";
       }
    }
    else
    {
        qDebug() << "getLonLatFromServer error" << "zzdTemp";
    }
    reply->deleteLater();
    return returnResult;
}

QString DownloadTask::getScaleFromServer(QString bagId, QString imageId, QPointF pos1,QPointF pos2)
{
    QString returnResult = "";
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
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
    post_param = document.toJson(QJsonDocument::Compact);
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
    QNetworkReply* reply = m_manager.post(request,post_param);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(1000*60);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &timer, &QTimer::stop);
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
           else
           {
               qDebug() << "getScaleFromServer error" << "zzdTemp";
           }
        }
    }
    else
    {
        qDebug() << "getScaleFromServer error" << "zzdTemp";
    }
    reply->deleteLater();
    return returnResult;
}


QJsonArray BatchDownloader::getAllExportInfo()
{
    QMutexLocker locker(&m_mutex);
    return m_allExportInfo;
}
