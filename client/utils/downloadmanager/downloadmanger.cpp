#include "downloadmanger.h"
#include "mysqlite.h"
#include "math.h"

DownLoadManger::DownLoadManger(QObject *parent) : QObject(parent)
{
    //获取所有的历史任务
    QString sql = QString("SELECT bag_id \
                          FROM download_record \
                          WHERE status IN ('下载中', '等待下载') \
                          ORDER BY \
                              CASE \
                                  WHEN status = '下载中' THEN 1 \
                                  WHEN status = '等待下载' THEN 2 \
                              END;");
    QSqlQuery queryResult;
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    while (result && queryResult.next())
    {
        QString bagId = queryResult.value(0).toString(); // 索引0表示第一个字段（即status）

        //获取bag文件对应的图片列表
        QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
        this->m_restFulApi.getPostData().clear();
        QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_IMAGES_LIST_GET).arg(bagId),VisitType::GET,ReplyType::IMAGES_LIST_GET_All);
        reply->setProperty("bagId",bagId);
    }
}

//获取单例类的实例
DownLoadManger* DownLoadManger::getInstance()
{
    DownLoadManger* pAppCommonBase = nullptr;
    static DownLoadManger appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}

void DownLoadManger::insertDownLoadTask(QList<ExportInfo> tasks)
{
    if(tasks.size() == 0)
        return;

    m_downLoaderList.push_back(tasks);
    if(!m_downLoadStatus)
    {
        startDownLoad();
    }
    else
    {
        QString bagId = tasks[0].bagId;
        emit sig_sendDownLoadStep(bagId,QString("等待下载"));
        updateDownStatus(bagId,"等待下载");
    }
}

QString DownLoadManger::getDownStatus(QString bagId)
{
    QString sql = QString("SELECT status FROM download_record WHERE bag_id = '%1'").arg(bagId);

    QSqlQuery queryResult;
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    if(result && queryResult.next())
    {
        return queryResult.value(0).toString(); // 索引0表示第一个字段（即status）
    }
    else
    {
        return "未下载";
    }
}

void DownLoadManger::startDownLoad()
{
    if(m_downLoadStatus || m_downLoaderList.size() == 0)
        return;

    // 2. 创建下载器实例
    if(nullptr == m_downloader)
    {
        m_downloader = new BatchDownloader;

        connect(m_downloader,&BatchDownloader::sig_save2LoacalStep,this,[=](int current, int total){
           double step = double(current)/double(total);
           qDebug() << "下载进度" << step << m_downLoadBagId;

           emit sig_sendDownLoadStep(m_downLoadBagId,QString("下载中%1%").arg(QString::number(step*100,'f',1)));

           //下载完成
           if(fabs(1-step) < 1e-9)
           {
               //设置下载状态为下载完成
               emit sig_sendDownLoadStep(m_downLoadBagId,QString("已下载"));
               updateDownStatus(m_downLoadBagId,"已下载");
               qDebug() << "下载完成" << m_downLoadBagId;
               m_downLoadBagId.clear();
               startDownLoad();
           }
        },Qt::QueuedConnection);

        connect(m_downloader,&BatchDownloader::sig_allHandlefinished,this,[=](){
           m_downLoadStatus = false;
           m_downLoaderList.removeAt(0);

           //设置下载状态为下载完成
           emit sig_sendDownLoadStep(m_downLoadBagId,QString("已下载"));
           updateDownStatus(m_downLoadBagId,"已下载");
           qDebug() << "下载完成" << m_downLoadBagId;
           m_downLoadBagId.clear();
           startDownLoad();
        },Qt::QueuedConnection);

        connect(m_downloader,&BatchDownloader::sig_handelFail,this,[=](){
            m_downLoadStatus = false;
            m_downLoaderList.removeAt(0);

            //设置下载状态为下载失败
            emit sig_sendDownLoadStep(m_downLoadBagId,QString("下载失败"));
            updateDownStatus(m_downLoadBagId,"下载失败");
            qDebug() << "下载失败" << m_downLoadBagId;
            m_downLoadBagId.clear();
            startDownLoad();
        },Qt::QueuedConnection);
    }

    // 3. 开始下载
    QList<ExportInfo> tasks = m_downLoaderList.at(0);
    m_downLoadBagId = tasks[0].bagId;
    m_downloader->downloadImages(tasks);
    m_downLoadStatus = true;

    //设置数据库下载状态为下载中
    updateDownStatus(m_downLoadBagId,"下载中");
}

void DownLoadManger::updateDownStatus(QString bagId, QString status)
{
    QSqlQuery queryResult;
    QString sqlDel = QString("delete from download_record where bag_id = '%1'").arg(bagId);
    MySqlite::getInstance()->execSQL(queryResult,sqlDel);

    QString sql = QString("REPLACE INTO download_record (bag_id, status) VALUES ('%1', '%2')").arg(bagId).arg(status);
    bool result = MySqlite::getInstance()->execSQL(queryResult,sql);
    qDebug() << "下载状态更新" << result << bagId << status;
}
