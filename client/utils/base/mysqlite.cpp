#include "mysqlite.h"
#include <QDir>
#include <QApplication>
#include <QPluginLoader>
#include <QDebug>
#include <QDateTime>

MySqlite::MySqlite(QObject *parent) : QObject(parent)
{

}

//获取单例类的实例
MySqlite* MySqlite::getInstance()
{
    MySqlite* pAppCommonBase = nullptr;
    static MySqlite appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}

MySqlite::~MySqlite()
{
    if(m_db.isOpen())
        m_db.close();
}

bool MySqlite::createDB(QString DBName)
{
    QString path = QDir::currentPath();
    QApplication::addLibraryPath(path);
    QPluginLoader loader(path+QString("/sqldrivers/qsqlite.dll"));

    //打印Qt支持的数据库驱动
    qDebug()<<QSqlDatabase::drivers();

    //1.检查链接是否存在
    if(QSqlDatabase::contains("qsqlite_test"))
        //qsqlite_test是定义的连接名   不填将使用默认连接名
        m_db = QSqlDatabase::database("QSQLITE","qsqlite_test");
    else
        m_db = QSqlDatabase::addDatabase("QSQLITE","qsqlite_test");

    /*    db.setHostName("easybook-3313b0");          //设置数据库主机名
        db.setDatabaseName("qtDB.db");              //设置数据库名
        db.setUserName("aa");                //设置数据库用户名
        db.setPassword("123456"); */                  //设置数据库密码

    //3.打开数据库 如果不存在则创建新的
    if(m_db.databaseName() != DBName)
    {
        m_db.setDatabaseName(DBName + ".db");
        if(!m_db.open())
        {
            qDebug("db open failed ");
            return false;
        }
        else
            qDebug("db open ok ");
    }

    return true;
}

bool MySqlite::deleteDB(QString DBName)
{
    if(m_db.open() && m_db.databaseName() != DBName)
    {
        //关闭数据库
        m_db.close();

        //删除本地数据库
        QFile file(DBName + ".db");
        if(file.exists()) {
            return file.remove();
        }
    }

    return true;
}

bool MySqlite::initTable(QString sTableName, QStringList sNameList, QStringList sType)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    QSqlQuery query;
    query  = QSqlQuery(m_db);
    if(m_db.isOpen())
    {
        QStringList tablelist = m_db.tables();
        if(!tablelist.contains(sTableName))
        {
            //表不存在  创建表
            QString sqlCreate = "create table " + sTableName;
            QString colValue = "";
            for(int i = 0; i < sNameList.size(); i++)
            {
                if(colValue == "")
                    colValue = sNameList[i] + " " + sType[i];
                else
                    colValue = colValue + "," + sNameList[i] + " " + sType[i];
            }
            sqlCreate += "(" + colValue + ")";

            //操作数据库
            if(!m_db.transaction())
            {
                qDebug() << "事务打开失败";
                return false;
            }
            bool result = query.exec(sqlCreate);

            //操作结束的时间戳
            QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());

            if(!result)
            {
                m_db.rollback();
                qDebug() << "initTable: " << sTableName + ",操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong()) << sqlCreate;
            }
            else
            {
                m_db.commit();
                qDebug() << "initTable: " << sTableName + ",操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;
            }

            return result;
        }
    }
}

bool MySqlite::insertDataForBase(QString sTableName, QMap<QString, QString> data, bool isOpenTransaction)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    QSqlQuery query;
    query  = QSqlQuery(m_db);
    QString sql="insert into "+sTableName+  "(";
    QString values=" values(";
    for(QMap<QString,QString>::const_iterator i=data.constBegin();i!=data.constEnd();i++)
    {
        sql+=i.key()+", ";
        values+= "'" + i.value() + "'" + ", ";
    }
    sql.chop(2);
    values.chop(2);
    sql+=")";
    values+=")";
    sql+=values;

    qDebug() << sql;
    //操作数据库
    if(isOpenTransaction)
    {
        if(!m_db.transaction())
        {
            qDebug() << "事务打开失败";
            return false;;
        }
    }
    bool result = query.exec(sql);

    //操作结束的时间戳
    QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());

    if(!result)
    {
        if(isOpenTransaction)
            m_db.rollback();
        qDebug() << "insertDataForBase: " << sTableName + ",操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong()) << sql;
    }
    else
    {
        if(isOpenTransaction)
            m_db.commit();
        qDebug() << "insertDataForBase: " << sTableName + ",操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;
    }

    return result;
}

bool MySqlite::insertDatasForBase(QString sTableName, QStringList sNameList, QVector<QVariantList> datas, bool isOpenTransaction)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    QSqlQuery query;
    query  = QSqlQuery(m_db);
    QString sql="insert into "+sTableName+  "(";
    for(auto name : sNameList)
    {
        sql += name + ", ";
    }
    sql.chop(2);
    sql+=")";

    QString values = "values (";
    for(auto name : sNameList)
    {
        values += ":" + name + ", ";
    }
    values.chop(2);
    values+=")";

    query.prepare(sql + values);

    int i = 0;
    for(auto name : sNameList)
    {
        query.bindValue(":" + name, QVariant(datas[i]));
        i++;
    }

     //操作数据库
    if(isOpenTransaction)
    {
        if(!m_db.transaction())
        {
            qDebug() << "事务打开失败";
            return false;;
        }
    }
    bool result = query.execBatch();

     //操作结束的时间戳
     QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());

     if(!result)
     {
         if(isOpenTransaction)
            m_db.rollback();
         qDebug() << "insertDatasForBase: " << sTableName + ",操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());
     }
     else
     {
         if(isOpenTransaction)
            m_db.commit();
         qDebug() << "insertDatasForBase: " << sTableName + ",操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;
     }

     return result;
}

bool MySqlite::deleteDataFromTable(QString sTableName, QString queryCondition)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    QSqlQuery query;
    query  = QSqlQuery(m_db);
    QString sql="delete from "+sTableName+  " where " + queryCondition;
    qDebug() << sql;

    //操作数据库
    if(!m_db.transaction())
    {
        qDebug() << "事务打开失败";
        return false;;
    }
    bool result = query.exec(sql);

    //操作结束的时间戳
    QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());

    if(!result)
    {
        m_db.rollback();
        qDebug() << "deleteDataFromTable: " << sTableName + ",操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong()) << sql;
    }
    else
    {
        m_db.commit();
        qDebug() << "deleteDataFromTable: " << sTableName + ",操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;
    }

    return result;
}

bool MySqlite::clearTable(QString sTableName,bool isOpenTransaction)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    QSqlQuery query;
    query  = QSqlQuery(m_db);
    QString sql="delete from " + sTableName;

    //操作数据库
    if(isOpenTransaction)
    {
        if(!m_db.transaction())
        {
            qDebug() << "事务打开失败";
            return false;;
        }
    }
    bool result = query.exec(sql);

    //操作结束的时间戳
    QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());

    if(!result)
    {
        m_db.rollback();
        qDebug() << "deleteTable: " << sTableName + ",操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong()) << sql;
    }
    else
    {
        m_db.commit();
        qDebug() << "deleteTable: " << sTableName + ",操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;
    }

    return result;
}

bool MySqlite::updateDataForBase(QString sTableName, QStringList sDataList, int index)
{
    if(!m_db.isOpen())
        return false;

    return true;
}

bool MySqlite::queryDataForBase(QString sTableName, QSqlQuery& queryResult,  QList<QString> keyList, QMap<QString, QString> whereMap, bool isSelectOnce)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    queryResult  = QSqlQuery(m_db);
    QString sql="select ";
    if(keyList.size() == 0)
    {
        sql += "*";
    }
    else
    {
        int len=keyList.size();
        for(int i=0;i<len;i++)
        {
            sql+=keyList.at(i);
            sql+=",";
        }
        sql.chop(1);
    }
    sql += " from " + sTableName;

    if(whereMap.size() > 0)
    {
        sql+=" where ";
        for(QMap<QString,QString>::const_iterator i = whereMap.constBegin(); i != whereMap.constEnd(); i++)
        {
            sql+=i.key()+"=";
            sql+=i.value()+" ";
        }
    }

    //是否只检索一条?
    if(isSelectOnce)
    {
        sql+= " limit 1";
    }

    //操作数据库
    qDebug() << sql;
    bool result = queryResult.exec(sql);

    //操作结束的时间戳
    QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());

    if(!result)
        qDebug() << "queryDataForBase: " << sTableName + ",操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong()) << sql;
    else
        qDebug() << "queryDataForBase: " << sTableName + ",操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;

    return result;

//    QSqlQuery::seek(int n) ：query指向结果集的第n条记录。

//    first() ：query指向结果集的第一条记录。

//    last() ：query指向结果集的最后一条记录。

//    next() ：query指向下一条记录，每执行一次该函数，便指向相邻的下一条记录。

//    previous() ：query指向上一条记录，每执行一次该函数，便指向相邻的上一条记录。

//    record() ：获得现在指向的记录。

//    value(int n) ：获得属性的值。其中n表示你查询的第n个属性，比方上面我们使用“select * from person”就相当于“select id, firstname，lastname from person”，那么value(0)返回id属性的值，value(1)返回firstname属性的值，value(2)返回lastname属性的值。该函数返回QVariant类型的数据，关于该类型与其他类型的对应关系，可以在帮助中查看QVariant。

//    at() ：获得现在query指向的记录在结果集中的编号

//    QSqlQuery query;
//    if(query.next())
//    //开始就先执行一次next()函数，那么query指向结果集的第一条记录
//    {
//        int rowNum= query.at();
//        //获取query所指向的记录在结果集中的编号
//        int columnNum=query.record().count();
//        //获取每条记录中属性（即列）的个数
//        int fieldNo1=query.record().indexOf("firstname");
//        //获取firstname属性所在列的编号，列从左向右编号，最左边的编号为0
//        int fieldNo2=query.record().indexOf("lastname");
//        //获取lastname属性所在列的编号，列从左向右编号，最左边的编号为0
//        int id = query.value(0).toInt();
//        //获取id属性的值，并转换为int型
//        QString firstname = query.value(1).toString();
//        //获取firstname属性的值
//        QString lastname = query.value(2).toString();
//        //获取lastname属性的值
//        qDebug()<<"rowNum is:"<<rowNum//将结果输出
//                <<"columnNum is :"<< columnNum
//                <<"fieldNo1 is:"<<fieldNo1
//                <<"fieldNo2 is:"<<fieldNo2
//                << "id is: " << id
//                << "firstname is :"<< firstname
//                << "lastname is :"<< lastname;
//    }

    //    return true;
}

bool MySqlite::execSQL(QSqlQuery &queryResult, QString sql)
{
    if(!m_db.isOpen())
        return false;

    //操作开始的时间戳
    QString timestampStart = QString::number(QDateTime::currentMSecsSinceEpoch());

    //操作数据库
    queryResult  = QSqlQuery(m_db);
    bool result = queryResult.exec(sql);

    //操作结束的时间戳
    QString timestampEnd = QString::number(QDateTime::currentMSecsSinceEpoch());
    qDebug() << sql;

    if(!result)
        qDebug() << "execSQL: " << "操作失败!" << m_db.lastError() << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong()) << sql;
    else
        qDebug() << "execSQL: " << "操作成功!" << "响应时间(毫秒): " << QString::number(timestampEnd.toLongLong() - timestampStart.toLongLong());;

    return result;
}

QSqlDatabase &MySqlite::getDB()
{
    return m_db;
}
