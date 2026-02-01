#ifndef DYNAMICPLOTTINGLISTEN_H
#define DYNAMICPLOTTINGLISTEN_H

#include <QObject>
#include <QJsonObject>
#include <QTimer>
#include "restfulapi.h"

class DynamicPlottingListen : public QObject
{
    Q_OBJECT
public:
    explicit DynamicPlottingListen(QObject *parent = nullptr);

    static DynamicPlottingListen *getInstance();

    void setRunningFlag(bool flag);
    bool getRunningFlag();

    void setBagId(QString bagId);

public:
    void slt_rcvPlottingResult(QJsonObject obj);

signals:
    void sig_sendMsgTip(QString tip);
    void sig_sendPlottingResult(QString bagId,QString imageId,QJsonObject result);

private:
    bool m_isRunning = false;
    QTimer m_timer;
    QTimer m_synTimer;//同步定时器
    QString m_bagId;
    RestFulApi m_restFulApi;

private:
    void saveDataToServer(QString bagId, QString imageId, QJsonObject obj);
    void synData2Server();

};

#endif // DYNAMICPLOTTINGLISTEN_H
