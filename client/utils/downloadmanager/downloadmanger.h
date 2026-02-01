#ifndef DOWNLOADMANGER_H
#define DOWNLOADMANGER_H

#include <QObject>
#include "base/batchdownloader.h"
#include "restfulapi.h"

class DownLoadManger : public QObject
{
    Q_OBJECT
public:
    explicit DownLoadManger(QObject *parent = nullptr);

    static DownLoadManger *getInstance();

    //添加下载任务
    void insertDownLoadTask(QList<ExportInfo> tasks);

    //获取下载状态
    QString getDownStatus(QString bagId);

signals:
    void sig_sendDownLoadStep(QString bagId,QString text);

private:
    QList<QList<ExportInfo>> m_downLoaderList;//所有的下载任务
    bool m_downLoadStatus = false;
    QString m_downLoadBagId;
    BatchDownloader *m_downloader = nullptr;
    RestFulApi m_restFulApi;//登录请求

private:
    void startDownLoad();
    void updateDownStatus(QString bagId, QString status);
};

#endif // DOWNLOADMANGER_H
