#ifndef DBPOOL_H
#define DBPOOL_H

#include <QObject>
#include <QMutex>
#include <QSqlDatabase>
#include <QDateTime>
#include <QMap>
#include <QTimer>
class DbPool : public QObject
{
    Q_OBJECT
public:
    /*
    * @Function  global_CurrentTime
    * @Description 获取当前时间，对QDateTime::currentTime 的封装，线程安全
    * @return  返回值(return)  QDateTime 当前时间
    */
    static QDateTime global_CurrentTime();
    explicit DbPool(QObject *parent = nullptr);
    static DbPool& Instance();
    QSqlDatabase getDbbyId(QString dbId);
    void release();
    void initDB(QString type,QString host,int port,QString Username,QString pwd,QString dbName = "");
private:
    QMutex m_muConnectDbId;
    QMap<QString,QDateTime > m_mapConnectName;

    QString m_hostName = "127.0.0.1";
    int m_port = 3306;
    QString m_databaseName = "robot_controller";
    QString m_username = "root";
    QString m_password = "root";
    QString m_databaseType = "QMYSQL";

    QTimer* m_timerClean;
signals:
public slots:
    void slt_cleanDbConnect();
};

#endif // DBPOOL_H
