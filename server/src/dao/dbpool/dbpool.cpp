#include "dbpool.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlDriver>
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

    qDebug() << "Available drivers:";
    qDebug() << QSqlDatabase::drivers(); // 打印所有可用驱动

    if(dbId.isEmpty())
    {
        qDebug() << "AppGlobal::getDbbyId threadId is empty";
        return QSqlDatabase();
    }
    if ( QSqlDatabase::contains( dbId ) )
    {
        qDebug() << "AppGlobal::getDbbyId:dbid is exist: " << dbId;
        m_mapConnectName.insert(dbId,global_CurrentTime());
        return  QSqlDatabase::database( dbId );
    }
    else
    {
        qDebug() << "AppGlobal::getDbbyId new db connect: " << dbId;
        QSqlDatabase db = QSqlDatabase::addDatabase( m_databaseType, dbId );
        db.setHostName(m_hostName);
        db.setDatabaseName(m_databaseName);
        db.setPort(m_port);
        db.setUserName(m_username);
        db.setPassword(m_password);
        db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT = 3");
        // 关键：添加SSL禁用选项
       // db.setConnectOptions("MYSQL_OPT_SSL_MODE=SSL_MODE_DISABLED");

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
        m_mapConnectName.insert(dbId,global_CurrentTime());
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
