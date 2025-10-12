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

// 1. 图片下载任务类 (继承QRunnable)
class DownloadTask : public QObject, public QRunnable {
    Q_OBJECT
public:
    DownloadTask(QNetworkAccessManager* manager, const QUrl& url, const QString& savePath)
        : m_manager(manager), m_url(url), m_savePath(savePath) {}

    void run() override {
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

signals:
    void sig_downloaded(const QByteArray& data, const QString& path);

private:
    QNetworkAccessManager* m_manager;
    QUrl m_url;
    QString m_savePath;
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

    void downloadImages(const QList<QPair<QUrl, QString>>& tasks) {
        m_totalTasks = tasks.size();
        m_completedTasks = 0;

        for(const auto& task : tasks) {
            auto downloader = new DownloadTask(m_networkManager, task.first,  task.second);
            connect(downloader, &DownloadTask::sig_downloaded,
                    &m_diskWriter, &DiskWriter::addTask);
            connect(downloader, &QObject::destroyed, this, [this]{
                m_completedTasks++;
                emit sig_downLoadFromUrlStep(m_completedTasks, m_totalTasks);
                qDebug() << "下载进度:" << m_completedTasks << "总数:" << m_totalTasks << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            });
            m_threadPool.start(downloader);
        }

        connect(&m_diskWriter, &DiskWriter::fileWritten, this, [this](const QString&) {
            qDebug() << "文件保存到磁盘进度" << m_filesWritten << "总数:" << m_totalTasks << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            emit sig_save2LoacalStep(m_filesWritten,m_totalTasks);
            if (++m_filesWritten == m_totalTasks) {
                emit sig_allHandlefinished();
            }
        });
    }

signals:
    void sig_downLoadFromUrlStep(int current, int total);
    void sig_save2LoacalStep(int current, int total);
    void sig_allHandlefinished();

private:
    QNetworkAccessManager* m_networkManager;
    QThreadPool m_threadPool;
    DiskWriter m_diskWriter;
    int m_totalTasks = 0;
    int m_completedTasks = 0;
    int m_filesWritten = 0;
};

#endif // BATCHDOWNLOADER_H