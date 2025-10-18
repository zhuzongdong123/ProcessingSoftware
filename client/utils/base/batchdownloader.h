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

#include "appdatabasebase.h"
#include "restfulapi.h"
#include <QTimer>

//导出的信息
struct ExportInfo
{
    //下载图片信息
    QUrl url;
    QString savePath;
    QString bagId;
    QString imageId;
};

// 1. 图片下载任务类 (继承QRunnable)
class DownloadTask : public QObject, public QRunnable {
    Q_OBJECT
public:
    DownloadTask(QNetworkAccessManager* manager, const QUrl& url, const QString& savePath, const QString& bagId, const QString& imageId)
        : m_manager(manager), m_url(url), m_savePath(savePath),m_bagId(bagId), m_imageId(imageId) {}

    void run() override {

        //如果图片id不为空，下载图片对应的事件
        if(!m_imageId.isEmpty() && !m_bagId.isEmpty())
        {
            QJsonArray returnArray =  handleEvents();
            emit sig_eventHandled(returnArray);
        }

        //下载图片
        {
            QNetworkRequest request(m_url);
            QNetworkReply* reply = m_manager->get(request);
            QEventLoop loop;
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            if(reply->error() == QNetworkReply::NoError) {
                emit sig_downloaded(reply->readAll(), m_savePath);
            }
            reply->deleteLater();
        }
    }

signals:
    void sig_downloaded(const QByteArray& data, const QString& path);
    void sig_eventHandled(QJsonArray array);

private:
    QNetworkAccessManager* m_manager;
    QUrl m_url;
    QString m_savePath;
    QString m_bagId;
    QString m_imageId;

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
                emit fileWritten(task.second);
            }
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
        m_networkManager = new QNetworkAccessManager(this);
        m_diskWriter.start();
        m_threadPool.setMaxThreadCount(8);
    }

    ~BatchDownloader() {
        m_threadPool.waitForDone();
    }

    void downloadImages(const QList<ExportInfo> tasks) {
        m_totalTasks = tasks.size();
        m_completedTasks = 0;

        for(const auto& task : tasks) {
            auto downloader = new DownloadTask(m_networkManager, task.url,  task.savePath, task.bagId, task.imageId);
            connect(downloader, &DownloadTask::sig_downloaded,
                    &m_diskWriter, &DiskWriter::addTask);
            connect(downloader, &QObject::destroyed, this, [this]{
                m_completedTasks++;
                emit sig_downLoadFromUrlStep(m_completedTasks, m_totalTasks);
                qDebug() << "下载进度:" << m_completedTasks << "总数:" << m_totalTasks << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            });
            connect(downloader, &DownloadTask::sig_eventHandled, this, [this](QJsonArray array){
                QMutexLocker locker(&m_mutex);
                if(array.size() == 0)
                    return;

                for(auto temp : array)
                {
                    m_allExportInfo.push_back(temp);
                }
            });
            m_threadPool.start(downloader);
        }

        connect(&m_diskWriter, &DiskWriter::fileWritten, this, [this](const QString&) {
            qDebug() << "文件保存到磁盘进度" << m_filesWritten << "总数:" << m_totalTasks << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            emit sig_save2LoacalStep(m_filesWritten,m_totalTasks);

            if (++m_filesWritten == m_totalTasks) {
                emit sig_sendAllHandleEvents(m_allExportInfo);
                emit sig_allHandlefinished();
            }
        });
    }

signals:
    void sig_downLoadFromUrlStep(int current, int total);
    void sig_save2LoacalStep(int current, int total);
    void sig_allHandlefinished();
    void sig_sendAllHandleEvents(QJsonArray array);

private:
    QNetworkAccessManager* m_networkManager;
    QThreadPool m_threadPool;
    DiskWriter m_diskWriter;
    int m_totalTasks = 0;
    int m_completedTasks = 0;
    int m_filesWritten = 0;
    QMutex m_mutex;
    QJsonArray m_allExportInfo;

private:
    QJsonArray getAllExportInfo();
};

#endif // BATCHDOWNLOADER_H