#ifndef AiMappingManager_H
#define AiMappingManager_H

#include <QObject>
#include <QTimer>
#include "restfulapi.h"

class AiMappingManager : public QObject
{
    Q_OBJECT
public:
    explicit AiMappingManager(QObject *parent = nullptr);

    static AiMappingManager *getInstance();

    //添加识别任务
    void insertDownLoadTask(QString bagId);

    //获取识别状态
    QString getDownStatus(QString bagId);

signals:
    void sig_sendDownLoadStep(QString bagId,QString text);

private:
    QList<QString> m_downLoaderList;//所有的识别任务
    QString m_bagId;
    RestFulApi m_restFulApi;//登录请求

private:
    void startDownLoad();
    void updateDownStatus(QString bagId, QString status);
    QTimer m_timer;
};

#endif // AiMappingManager_H
