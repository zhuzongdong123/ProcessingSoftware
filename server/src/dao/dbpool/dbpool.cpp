#include "dbpool.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlDriver>
#include <QUuid>
#include <QSqlQuery>
#include <QApplication>

QDateTime DbPool::global_CurrentTime()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    return QDateTime::currentDateTime();
}

DbPool::DbPool(QObject *parent) : QObject(parent)
{
    m_timerClean = new QTimer();
    connect(m_timerClean,&QTimer::timeout,this,&DbPool::slt_cleanDbConnect);
    m_timerClean->start(1000*60);
}

DbPool &DbPool::Instance()
{
    static DbPool intstance;
    return intstance;
}

QSqlDatabase DbPool::getDbbyId(QString dbId)
{
    QMutexLocker muLock(&m_muConnectDbId);

    dbId = QUuid::createUuid().toString().remove("-").remove("{").remove("}");;

    qDebug() << "Available drivers:";
    qDebug() << QSqlDatabase::drivers(); // 打印所有可用驱动
    {
        qDebug() << "AppGlobal::getDbbyId new db connect: " << dbId;
        QSqlDatabase db = QSqlDatabase::addDatabase( m_databaseType, dbId );
        db.setHostName(m_hostName);
        db.setDatabaseName(QApplication::applicationDirPath() + "/" + m_databaseName);
        db.setPort(m_port);
        db.setUserName(m_username);
        db.setPassword(m_password);
        db.setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE=1;QSQLITE_BUSY_TIMEOUT=5000");
        // 关键：添加SSL禁用选项
        db.setConnectOptions("MYSQL_OPT_SSL_MODE=SSL_MODE_DISABLED");

        if(db.open())
        {
            qDebug() << "AppGlobal::getDbbyId new db open success";
        }
        else
        {
            qDebug() << "AppGlobal::getDbbyId new db open failure";
            QSqlError error = db.lastError();
            qDebug() << "Database connection failed: " << error.text();
            // 输出更详细的错误信息（如果可用）
            qDebug() << "Driver error: " << error.driverText();
            qDebug() << "Database error: " << error.databaseText();
        }
        return  db;
    }
     return QSqlDatabase();
}

void DbPool::release()
{
    QMutexLocker muLock(&m_muConnectDbId);
    QMap<QString,QDateTime>::iterator it = m_mapConnectName.begin();
    while (it != m_mapConnectName.end())
    {
        QString dbId = it.key();
        if(QSqlDatabase::contains( dbId ))
        {
            {
                QSqlDatabase db = QSqlDatabase::database(dbId);
                db.close();
            }
            QSqlDatabase::removeDatabase(dbId);
        }
        it++;
    }
    m_mapConnectName.clear();
}

void DbPool::initDB(QString type, QString host, int port, QString Username, QString pwd, QString dbName)
{
    m_databaseType = type;
    m_hostName = host;
    this->m_port = port;
    m_databaseName = dbName;
    m_password = pwd;
    this->m_username = Username;
}

void DbPool::slt_cleanDbConnect()
{
    QMutexLocker muLock(&m_muConnectDbId);
    QMap<QString,QDateTime>::iterator it = m_mapConnectName.begin();
    QDateTime curTime = global_CurrentTime();
    while (it != m_mapConnectName.end())
    {
        QString dbId = it.key();
        QDateTime dbTime = it.value();
        if(QSqlDatabase::contains( dbId ))
        {
            if(qAbs(dbTime.secsTo(curTime)) > (60 * 30))
            {
                {
                    QSqlDatabase db = QSqlDatabase::database(dbId);
                    db.close();
                }
                QSqlDatabase::removeDatabase(dbId);
                it = m_mapConnectName.erase(it);
                qDebug() << "slt_cleanDbConnect remove connect: " << dbId;
            }
            else
            {
              it++;
            }

        }
        else
        {
            it++;
        }

    }
}
