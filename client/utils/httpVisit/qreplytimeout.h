#ifndef QREPLYTIMEOUT_H
#define QREPLYTIMEOUT_H

#include <QObject>
#include "QTimer"
#include "QNetworkReply"

class QReplyTimeout : public QObject {
    Q_OBJECT
public:
    QReplyTimeout(QNetworkReply *networkReply, const int timeout, bool isAbortReply = true) : QObject(networkReply){
        Q_ASSERT(networkReply);
        m_isAbortReply = isAbortReply;
        if (networkReply && networkReply->isRunning())
        {  // 启动单次定时器
            QTimer::singleShot(timeout, this, SLOT(onTimeout()));
        }
    }

signals:
    void timeout();  // 超时信号 - 供进一步处理

private:
    bool m_isAbortReply;

private slots:
    //网络超时处理
    inline void onTimeout() {
        QNetworkReply *networkReply = static_cast<QNetworkReply*>(parent());
        if (networkReply->isRunning() && m_isAbortReply){
            emit timeout();
        }
    }
};

#endif // QREPLYTIMEOUT_H
