#include "dynamicplottinglisten.h"
#include <QDebug>
#include <QJsonArray>
#include "appdatabasebase.h"
#include <QRect>
#include "mysqlite.h"
#include <QUuid>
#include <QEventLoop>

DynamicPlottingListen::DynamicPlottingListen(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, [=]() {
        m_isRunning = false;
        m_timer.stop();
    });

    synData2Server();
    connect(&m_synTimer, &QTimer::timeout, [=]() {
        synData2Server();
    });
    m_synTimer.start(8000);
}

//获取单例类的实例
DynamicPlottingListen* DynamicPlottingListen::getInstance()
{
    DynamicPlottingListen* pAppCommonBase = nullptr;
    static DynamicPlottingListen appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}

void DynamicPlottingListen::setRunningFlag(bool flag)
{
    m_isRunning = flag;

    if(m_isRunning)
    {
        m_timer.start(1000*15);//15秒后收不到数据，设置为标绘结束
    }
}

bool DynamicPlottingListen::getRunningFlag()
{
    return m_isRunning;
}

void DynamicPlottingListen::setBagId(QString bagId)
{
    m_bagId = bagId;
}

void DynamicPlottingListen::slt_rcvPlottingResult(QJsonObject obj)
{
    if(!obj.value("request_id").toString().isEmpty())
    {
        m_isRunning = true;
        m_timer.start(1000*15);//15秒后收不到数据，设置为标绘结束

        //获取进度
        double step = obj.value("step").toDouble();
        QString status_code = obj.value("status_code").toString();

        //组装提示语
        QString msg = QString("智能标绘进度: %1%").arg(QString::number(step,'f',2));
        //向外界发送处理进度
        emit sig_sendMsgTip(msg);

        //判断是否处理完成
        if(status_code == "handled" || fabs(step-100) < 1e-5)
        {
            m_isRunning = false;
            m_timer.stop();
            emit sig_sendMsgTip("");
        }

        //智能识别有结果的场合
        if(obj.value("detections").toArray().size() > 0)
        {
            //发送到缓存中，为了方便标绘一个显示一个。
            QString imageId = obj.value("image_name").toString().replace(".jpg","");
            emit sig_sendPlottingResult(m_bagId,imageId,obj);


            //保存处理结果到数据库中
            saveDataToServer(m_bagId,imageId,obj);

            qDebug() << "智能标绘结果：" << obj;
        }
    }
}

void DynamicPlottingListen::saveDataToServer(QString bagId, QString imageId, QJsonObject obj)
{
    if(obj.value("detections").toArray().size() == 0)
        return;

    QJsonArray dataArrayTemp;
    for(auto annotation : obj.value("detections").toArray()) {
        if(annotation.toObject().value("bbox").toArray().size() != 4)
            continue;

        QPoint topLeft = QPoint(annotation.toObject().value("bbox").toArray()[0].toInt(),annotation.toObject().value("bbox").toArray()[1].toInt());
        QPoint bottomRight = QPoint(annotation.toObject().value("bbox").toArray()[2].toInt(),annotation.toObject().value("bbox").toArray()[3].toInt());
        QRectF rect = QRect(topLeft,bottomRight);
        QString text = annotation.toObject().value("label").toString();

        QJsonObject objTemp;
        objTemp.insert("x1",rect.x());
        objTemp.insert("x2",rect.y());
        objTemp.insert("y1",rect.x() + rect.width());
        objTemp.insert("y2",rect.y() + rect.height());
        objTemp.insert("event_type",text);
        objTemp.insert("isHandle",true);
        dataArrayTemp.push_back(objTemp);
    }
    if(dataArrayTemp.size() == 0)
        return;

//    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
//    this->m_restFulApi.getPostData().clear();
//    QJsonObject post_data;
//    QJsonDocument document;
//    QByteArray post_param;
//    post_data.insert("data",dataArrayTemp);//标绘的数据
//    post_data.insert("bag_id",bagId);
//    post_data.insert("image_id",imageId);
//    document.setObject(post_data);
//    post_param = document.toJson(QJsonDocument::Compact);
//    m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_ADD_PLOTTING_EVENTS,
//                          VisitType::POST,ReplyType::ANNOTATION_ADD_PLOTTING_EVENTS,"application/json",post_param,true);


    //防止万一，存入本地数据库一份
    QSqlQuery queryResult;
    QString byte = QJsonDocument(dataArrayTemp).toJson(QJsonDocument::Compact);
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).replace("-","");
    QString sql = QString("REPLACE INTO plotting_record (id,bag_id,image_id,data,isSyn) VALUES ('%1', '%2', '%3', '%4','0')").arg(uuid).arg(bagId).arg(imageId).arg(byte);
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    qDebug() << "数据库存储结果：" << result << sql;
}

void DynamicPlottingListen::synData2Server()
{
    QSqlQuery queryResult;
    QString sql = QString("select * from plotting_record where isSyn != '1'");
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    if(!result)
    {
        return;
    }

    QJsonArray sysArray;
    QStringList idList;
    while (result && queryResult.next())
    {
        QString uuid = queryResult.value("id").toString();
        idList.push_back(uuid);
        QString bagId = queryResult.value("bag_id").toString();
        QString imageId = queryResult.value("image_id").toString();
        QString data = queryResult.value("data").toString();
        QJsonObject obj;
        obj.insert("bagId",bagId);
        obj.insert("imageId",imageId);
        obj.insert("data",data);
        sysArray.push_back(obj);
    }

    if(sysArray.size() == 0)
        return;

    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("data",sysArray);//标绘的数据
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_ADD_PLOTTING_EVENTS,
                          VisitType::POST,ReplyType::ANNOTATION_ADD_PLOTTING_EVENTS,"application/json",post_param,true,4000);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    //同步成功
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
        if(m_restFulApi.replyResultCheck(obj,reply))
        {
            result = MySqlite::getInstance()->batchUpdateIsSyn(idList);
            qDebug() << "数据库同步标绘数据状态：" << result << sql;
        }
    }
}