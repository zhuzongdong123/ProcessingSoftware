#include "aimappingmanager.h"
#include "mysqlite.h"
#include "dynamicplottinglisten.h"
#include <QApplication>
#include <QFileInfo>
AiMappingManager::AiMappingManager(QObject *parent) : QObject(parent)
{
    //获取所有的历史任务
    QString sql = QString("SELECT bag_id \
                          FROM AiMappingTask_record \
                          WHERE status IN ('AI标绘中', '等待AI标绘') \
                          ORDER BY \
                              CASE \
                                  WHEN status = 'AI标绘中' THEN 1 \
                                  WHEN status = '等待AI标绘' THEN 2 \
                              END;");
    QSqlQuery queryResult;
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    while (result && queryResult.next())
    {
        QString bagId = queryResult.value(0).toString(); // 索引0表示第一个字段（即status）

        //添加到容器中
        if(!m_downLoaderList.contains(bagId))
        {
            m_downLoaderList.append(bagId);
        }
    }

    connect(&m_timer,&QTimer::timeout,this,[=](){
        //
        startDownLoad();
    });
    m_timer.start(1000*10);

    connect(DynamicPlottingListen::getInstance(),&DynamicPlottingListen::sig_sendHandledFlag,this,[=](QString bagId){
        int index = m_downLoaderList.indexOf(bagId);
        if (index != -1)
        {
            m_downLoaderList.removeAt(index);
        }
        updateDownStatus(bagId,"标绘完成");
    });
}

//获取单例类的实例
AiMappingManager* AiMappingManager::getInstance()
{
    AiMappingManager* pAppCommonBase = nullptr;
    static AiMappingManager appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}

void AiMappingManager::insertDownLoadTask(QString bagId)
{
    if(!m_downLoaderList.contains(bagId))
    {
        updateDownStatus(bagId,"等待AI标绘");
        m_downLoaderList.append(bagId);
    }
}

QString AiMappingManager::getDownStatus(QString bagId)
{
    QString sql = QString("SELECT status FROM AiMappingTask_record WHERE bag_id = '%1'").arg(bagId);

    QSqlQuery queryResult;
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    if(result && queryResult.next())
    {
        return queryResult.value(0).toString(); // 索引0表示第一个字段（即status）
    }
    else
    {
        return "未AI标绘";
    }
}

void AiMappingManager::startDownLoad()
{
    if(m_downLoaderList.size() == 0)
        return;

    //标绘中，ruturn
    if(DynamicPlottingListen::getInstance()->getRunningFlag())
    {
        return;
    }

    //获取下载状态是否为已下载
    m_bagId = m_downLoaderList[0];
    QString status;
    {
        //获取所有的历史任务
        QString sql = QString("SELECT status \
                              FROM download_record \
                              WHERE bag_id = '%1'").arg(m_bagId);
        QSqlQuery queryResult;
        bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
        while (result && queryResult.next())
        {
            status = queryResult.value(0).toString(); // 索引0表示第一个字段（即status）
        }
    }

    if(status != "已下载")
    {
        emit DynamicPlottingListen::getInstance()->sig_sendMsgTip(m_bagId, "等待下载完成");
        return;
    }

    DynamicPlottingListen::getInstance()->setBagId(m_bagId);
    QString folderPath = QApplication::applicationDirPath() + "/" + m_bagId;
    QFileInfo folderInfo(folderPath);
    if(folderInfo.exists() && folderInfo.isDir())
    {
        //从业务数据库中获取所有的事件
        QString requestUrl = "http://127.0.0.1:8000/process_folder";
        this->m_restFulApi.getPostData().clear();
        QJsonObject post_data;
        QJsonDocument document;
        QByteArray post_param;
        post_data.insert("folder_path",folderPath);
        post_data.insert("workers",1);
        post_data.insert("recursive",false);
        //post_data.insert("request_id",m_bagId);
        qDebug() << "智能标绘请求数据" << post_data;
        document.setObject(post_data);
        post_param = document.toJson(QJsonDocument::Compact);
        QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl,
                              VisitType::POST,ReplyType::dynamicPlotting2,"application/json",post_param,true,1000*60);//40秒超时

        //添加遮罩
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QJsonObject returnObj;
        QStringList requestUrlList;
        if(reply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(reply->readAll()).object();
            if(obj.value("status").toString() == "received")
            {
                DynamicPlottingListen::getInstance()->setBagId(m_bagId);
                updateDownStatus(m_bagId,"AI标绘中");
            }
            else
            {
                DynamicPlottingListen::getInstance()->setBagId("");
            }
        }
        else
        {
            DynamicPlottingListen::getInstance()->setBagId("");
        }
    }
}

void AiMappingManager::updateDownStatus(QString bagId, QString status)
{
    if(bagId.isEmpty())
        return;

    QSqlQuery queryResult;
    QString sqlDel = QString("delete from AiMappingTask_record where bag_id = '%1'").arg(bagId);
    MySqlite::getInstance()->execSQL(queryResult,sqlDel);

    QString sql = QString("REPLACE INTO AiMappingTask_record (bag_id, status) VALUES ('%1', '%2')").arg(bagId).arg(status);
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    qDebug() << "AI标绘状态更新" << result << bagId << status;
}