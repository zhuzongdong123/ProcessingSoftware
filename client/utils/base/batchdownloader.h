#ifndef BATCHDOWNLOADER_H
#define BATCHDOWNLOADER_H

#include <QObject>
#include <QThreadPool>
#include <QNetworkAccessManager>
#include <QFile>
#include <QMutex>
#include <QEventLoop>
#include <QNetworkReply>
#include <QQueue>
#include <QWaitCondition>
#include <QThread>
#include <QRunnable>
#include <QScopedPointer>
#include <QPair>
#include <QDateTime>
#include <QDebug>
#include "appdatabasebase.h"
#include "restfulapi.h"
#include <QTimer>
#include <QPixmap>
#include "appconfigbase.h"

//导出的信息
struct ExportInfo
{
    //下载图片信息
    QUrl image_url;
    QString image_savePath;
    QString bagId;
    QString imageId;
    QSet<QString> selectedEventsSet;
    QUrl pointsImage_url;
    QString pointsImage_savePath;
};

static QNetworkAccessManager m_manager;
static bool m_isStartThread = false;
// 1. 图片下载任务类 (继承QRunnable)
class DownloadTask : public QObject, public QRunnable {
    Q_OBJECT
public:
    DownloadTask(const ExportInfo& exportInfo)
    {
        m_imageUrl = exportInfo.image_url;
        m_imageSavePath = exportInfo.image_savePath;
        m_bagId = exportInfo.bagId;
        m_imageId = exportInfo.imageId;
        m_eventsSet = exportInfo.selectedEventsSet;
        m_pointsImageUrl = exportInfo.pointsImage_url;
        m_pointsImageSavePath = exportInfo.pointsImage_savePath;
    }
    ~DownloadTask()
    {
        qDebug() << "delete DownloadTask";
    }
    void run() override {
        //如果图片id不为空，下载图片对应的事件
        if(!m_imageId.isEmpty() && !m_bagId.isEmpty())
        {
            QJsonArray returnArray =  handleEvents();
            //图片无事件不处理
            if(returnArray.size() == 0)
            {
                return;
            }
            else
            {
                emit sig_eventHandled(returnArray);
            }
        }

        //下载点云图片
        if(!m_pointsImageSavePath.isEmpty())
        {
            qDebug () << "下载点云图 start" << m_pointsImageUrl.url() << m_pointsImageSavePath;
            QNetworkRequest request(m_pointsImageUrl);
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
                QImage image;
                if (image.loadFromData(decodedData))  {
                    bool result = image.save(m_pointsImageSavePath,  "jpg");
                    if(!result)
                    {
                        qDebug () << "下载点云图 error 图片保存失败" << m_pointsImageUrl.url() << m_pointsImageSavePath << "\n\n\n";
                    }
                }
                else
                {
                    auto temp =QJsonDocument::fromJson(readArray).object();
                    qDebug () << "下载点云图 error 获取结果为非图片数据" << temp << m_pointsImageUrl.url() << m_pointsImageSavePath << "\n\n\n";
                }
            }
            else
            {
                qDebug () << "下载点云图 error 获取图片超时" << m_pointsImageUrl.url() << m_pointsImageSavePath << "\n\n\n";
            }
            reply->deleteLater();
            qDebug () << "下载点云图 end" << m_pointsImageSavePath;
        }

        //下载图片
        {
            qDebug () << "下载图片 start" << m_imageUrl;
            QNetworkRequest request(m_imageUrl);
            QNetworkReply* reply = m_manager.get(request);

            QEventLoop loop;
            QTimer timer;
            timer.setInterval(15000);  // 设置超时时间 3 秒
            timer.setSingleShot(true);  // 单次触发
            connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
            connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            connect(reply, &QNetworkReply::finished, &timer, &QTimer::stop);
            timer.start();
            loop.exec();

            if(reply->error() == QNetworkReply::NoError) {
                QFile file(m_imageSavePath);
                if (file.open(QIODevice::WriteOnly))  {
                    file.write(reply->readAll());
                    file.close();
                    emit sig_sendHandStatus(true);
                }
                else
                {
                    qDebug() << "file save error 保存失败" << m_imageSavePath << "\n\n\n";
                    emit sig_sendHandStatus(false);
                }
            }
            else
            {
                qDebug () << "下载图片 error 获取图片超时" << m_imageUrl.url() << m_imageSavePath << "\n\n\n";
                emit sig_sendHandStatus(false);
            }
            reply->deleteLater();
            qDebug () << "下载图片 end" << m_imageUrl;
        }
    }

signals:
    void sig_sendHandStatus(bool isSucessed);
    void sig_eventHandled(QJsonArray array);

private:
    //QNetworkAccessManager m_manager;
    QUrl m_imageUrl;
    QString m_imageSavePath;
    QString m_bagId;
    QString m_imageId;
    QSet<QString> m_eventsSet;
    QUrl m_pointsImageUrl;
    QString m_pointsImageSavePath;

private:
    QJsonArray handleEvents();
    QPointF getLonLatFromServer(QString bagId, QString imageId, QPointF pos1, QPointF pos2);
    QString getScaleFromServer(QString bagId, QString imageId, QPointF pos1, QPointF pos2);
};

// 2. 磁盘写入管理器 (专用IO线程)
class DiskWriter : public QThread {
    Q_OBJECT
public:
    DiskWriter(QObject* parent = nullptr) : QThread(parent) {}

    ~DiskWriter() {
        requestInterruption();
        m_condition.wakeAll();
        wait();
    }

    void addTask(const QByteArray& data, const QString& path) {
        QMutexLocker locker(&m_mutex);
        m_taskQueue.enqueue({data,  path});
        m_condition.wakeOne();
    }

protected:
    void run() override {
        while (!isInterruptionRequested()) {
            QPair<QByteArray, QString> task;
            {
                QMutexLocker locker(&m_mutex);
                if (m_taskQueue.isEmpty())  {
                    m_condition.wait(&m_mutex);
                }
                if (m_taskQueue.isEmpty())  continue;
                task = m_taskQueue.dequeue();
            }

            QFile file(task.second);
            if (file.open(QIODevice::WriteOnly))  {
                file.write(task.first);
                file.close();
            }
            else
            {
                qDebug() << "file save error " << task.second;
            }
            emit fileWritten(task.second);

            QThread::msleep(10);
        }

        // 清空剩余任务
        QMutexLocker locker(&m_mutex);
        while (!m_taskQueue.isEmpty())  {
            auto task = m_taskQueue.dequeue();
            QFile file(task.second);
            if (file.open(QIODevice::WriteOnly))  {
                file.write(task.first);
            }
        }
    }

signals:
    void fileWritten(const QString& path);

private:
    QQueue<QPair<QByteArray, QString>> m_taskQueue;
    QMutex m_mutex;
    QWaitCondition m_condition;
};

// 3. 批处理下载器 (主控制器)
class BatchDownloader : public QObject {
    Q_OBJECT
public:
    explicit BatchDownloader(QObject* parent = nullptr) : QObject(parent) {
        //m_networkManager = new QNetworkAccessManager(this);
        int threadCount = AppConfigBase::getInstance()->readConfigSettings("thread","count","1").toInt();
        m_diskWriter.start();
        m_threadPool.setMaxThreadCount(threadCount);
    }

    ~BatchDownloader() {
        qDebug() << "stop BatchDownloader begin";
        m_threadPool.clear();
        m_threadPool.waitForDone();
        qDebug() << "stop BatchDownloader end";
    }

    void stopAllThread()
    {
        m_threadPool.clear();
        m_threadPool.waitForDone();
    }

    void downloadImages(const QList<ExportInfo> tasks) {
        qDebug() << "准备下载，任务数量：" << tasks.size();
        m_isStartThread = true;
        m_totalTasks = tasks.size();
        m_completedTasks = 0;

        for(const auto& task : tasks) {
            auto downloader = new DownloadTask(task);
            connect(downloader, &QObject::destroyed, this, [this]{
                m_completedTasks++;
                emit sig_downLoadFromUrlStep(m_completedTasks, m_totalTasks);
                emit sig_save2LoacalStep(m_completedTasks, m_totalTasks);
                if(m_completedTasks == m_totalTasks)
                {
                    emit sig_sendAllHandleEvents(m_allExportInfo);
                    emit sig_allHandlefinished();
                    m_isStartThread = false;
                }
                qDebug() << "下载进度:" << m_completedTasks << "总数:" << m_totalTasks << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            },Qt::QueuedConnection);
            connect(downloader, &DownloadTask::sig_eventHandled, this, [this](QJsonArray array){
                if(array.size() == 0)
                    return;

                QMutexLocker locker(&m_mutex);
                for(auto temp : array)
                {
                    m_allExportInfo.push_back(temp);
                }
            },Qt::QueuedConnection);

            connect(downloader, &DownloadTask::sig_sendHandStatus, this, [this](bool isSucessed){
                if(!isSucessed)
                {
                    m_errorCounts++;
                }
                else
                {
                    m_errorCounts = 0;
                }

                if(m_errorCounts > 2)
                {
                    emit sig_sendAllHandleEvents(m_allExportInfo);
                    emit sig_handelFail();
                    m_isStartThread = false;
                    m_errorCounts = -100;
                }
            },Qt::QueuedConnection);


            m_threadPool.start(downloader);
        }
    }

signals:
    void sig_downLoadFromUrlStep(int current, int total);
    void sig_save2LoacalStep(int current, int total);
    void sig_allHandlefinished();
    void sig_sendAllHandleEvents(QJsonArray array);
    void sig_handelFail();

private:
    QThreadPool m_threadPool;
    DiskWriter m_diskWriter;
    int m_totalTasks = 0;
    int m_errorCounts = 0;
    int m_completedTasks = 0;
    int m_filesWritten = 0;
    QMutex m_mutex;
    QJsonArray m_allExportInfo;

private:
    QJsonArray getAllExportInfo();
};

#endif // BATCHDOWNLOADER_H
