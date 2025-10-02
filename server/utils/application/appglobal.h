#ifndef APPGLOBAL_H
#define APPGLOBAL_H

#include <QObject>
#include <QMutex>
#include <QTimer>
class AppGlobal : public QObject
{
    Q_OBJECT
public:
    AppGlobal(QObject *parent = nullptr);
    ~AppGlobal();
    static AppGlobal& Instance();
    bool dbOpenStatus();
    void setOpenStatus(bool status);
    void initDbStatus();
private:
    QMutex m_muStatus;
    bool m_bdbOpen = false;
    QTimer* m_timerDbStatus;
    QString m_dbId;
signals:
public slots:
    void slt_timerDbStatus();

};

#endif // APPGLOBAL_H
