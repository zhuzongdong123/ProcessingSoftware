#include "dao.h"
#include <QDebug>
#include "dbpool.h"
#include <QRegularExpression>
DAO::DAO()
{

}

QString DAO::toHump(QString value)
{
    QStringList list = value.split("_");
    if(list.size() < 2)
        return value;

    QString result = list[0];
    for(int i = 1; i < list.size(); i++)
    {
        QString tempV = list[i];
        tempV = tempV.mid(0,1).toUpper() + tempV.mid(1);
        result +=tempV;
    }

    return result;
}

QString DAO::fromHump(QString value)
{
    return value.replace(QRegularExpression("([a-z])([A-Z])"),  "\\1_\\2").toLower();
}

bool DAO::openConnection(QString threadId)
{
    m_db = DbPool::Instance().getDbbyId(threadId);
    if(!m_db.isOpen())
    {
        m_lastError = m_db.lastError().databaseText();
        qDebug() << "DAO::openConnection db open failure: " << m_threadId << m_lastError;
        return  false;
    }
    else
    {
        return true;
    }
}

bool DAO::execSQL(QStringList sql)
{
    bool result = false;
    QSqlQuery queryResult  = QSqlQuery(m_db);
    m_db.transaction();
    foreach(QString sql,sql)
    {
      result = queryResult.exec(sql);
      if(!result)
      {
          break;
      }
    }

    if(!result)
    {
        m_db.rollback();
    }
    else
    {
        m_db.commit();
    }
    m_lastError = queryResult.lastError().text();

    return result;
}

bool DAO::execSQL(QString sql)
{
    bool result = false;
    QSqlQuery queryResult  = QSqlQuery(m_db);
    result = queryResult.exec(sql);
    m_lastError = queryResult.lastError().text();

    return result;
}

bool DAO::execSQL(QSqlQuery &queryResult, QString sql)
{
    qDebug() << sql;
    bool result = false;
    queryResult  = QSqlQuery(m_db);
    result = queryResult.exec(sql);
    m_lastError = queryResult.lastError().text();

    return result;
}

bool DAO::insertDataForBaseList(QString sTableName, QMap<QString, QVariantList> data)
{
    bool ret = false;
    QSqlQuery query;
    query  = QSqlQuery(m_db);
    //query.prepare("INSERT INTO test.student (id,name,age) VALUES(:id,:name,:age)");
    QList<QString> sNameList = data.keys();
    QString sql="insert into "+sTableName+  " (";
    QString values = " values (";
    for(auto name : sNameList)
    {
        sql += name + ", ";
        values += ":" + name + ", ";
    }
    sql.chop(2);
    sql+=")";

    values.chop(2);
    values+=")";

    query.prepare(sql + values);


    for(auto name : sNameList)
    {
        query.bindValue(":" + name, data[name]);
    }

    ret = query.execBatch();
    m_lastError = query.lastError().text();


    return ret;
}

bool DAO::insertDatasForBaseList(QString sTableName, QMap<QString, QVariantList> data)
{
    bool ret = false;
    QSqlQuery query(m_db);

    // 获取字段名列表
    QList<QString> sNameList = data.keys();
    if (sNameList.isEmpty())  {
        m_lastError = "No fields provided";
        return false;
    }

    // 检查所有字段的值的数量是否一致
    int recordCount = data[sNameList.first()].size();
    for (auto name : sNameList) {
        if (data[name].size() != recordCount) {
            m_lastError = QString("Field '%1' has inconsistent number of values").arg(name);
            return false;
        }
    }

    // 构建SQL语句
    QString sql = "INSERT INTO " + sTableName + " (";
    QString values = " VALUES (";

    for(auto name : sNameList) {
        sql += name + ", ";
        values += ":" + name + ", ";
    }
    sql.chop(2);
    sql += ")";
    values.chop(2);
    values += ")";

    query.prepare(sql  + values);

    // 开始事务
    m_db.transaction();

    try {
        // 绑定并执行每条记录
        for (int i = 0; i < recordCount; ++i) {
            for(auto name : sNameList) {
                query.bindValue(":"  + name, data[name].at(i));
            }
            if (!query.exec())  {
                m_lastError = query.lastError().text();
                m_db.rollback();
                return false;
            }
        }

        // 提交事务
        if (!m_db.commit())  {
            m_lastError = m_db.lastError().text();
            return false;
        }

        ret = true;
    } catch (...) {
        m_db.rollback();
        m_lastError = "Exception occurred during batch insert";
        return false;
    }

    return ret;
}

bool DAO::updateDataForBase(QString sTableName, QMap<QString, QVariant> update, QMap<QString, QVariant> where)
{
    if(sTableName.isEmpty() || update.isEmpty())
    {
        return false;
    }
    bool ret = false;
    QSqlQuery query;
    query  = QSqlQuery(m_db);

    // UPDATE Persons SET age=:age,name=:haha WHERE id=:hehe
    QList<QString> sNameList = update.keys();
    QList<QString> sWhereList = where.keys();
    QString sql = "update " + sTableName + " set ";
    QString wheres = " where 1=1";
    for(auto name : sNameList)
    {
         sql += name + "=:v"+name + ",";
    }
    for(auto con : sWhereList)
    {
        wheres += " and " + con + "=:w" + con ;
    }
    sql.chop(1);
    sql += wheres;
    query.prepare(sql);
    for(auto name : sNameList)
    {
         query.bindValue(":v" + name, update[name]);
    }
    for(auto con : sWhereList){
        query.bindValue(":w" + con, where[con]);
    }
    ret = query.exec();
    m_lastError = query.lastError().text();


    return ret;
}

bool DAO::updateDataForBase(QString sTableName, QMap<QString, QVariant> update)
{
    if(sTableName.isEmpty() || update.isEmpty())
    {
        return false;
    }
    bool ret = false;
    QSqlQuery query;
    query  = QSqlQuery(m_db);

    // UPDATE Persons SET age=:age,name=:haha WHERE id=:hehe
    QList<QString> sNameList = update.keys();

    QString sql = "update " + sTableName + " set ";

    for(auto name : sNameList)
    {
         sql += name + "=:v"+name + ",";
    }

    sql.chop(1);

    query.prepare(sql);
    for(auto name : sNameList)
    {
         query.bindValue(":v" + name, update[name]);
    }

    ret = query.exec();
    m_lastError = query.lastError().text();


    return ret;
}

QJsonArray DAO::queryDataForBase(QString sTableName, QList<QString> filed)
{
    QJsonArray ret;
    QSqlQuery query;
    query  = QSqlQuery(m_db);

    if(filed.isEmpty())
    {
        return ret;
    }
    QString sql = "select ";
    QString sFileds;
    QString sCons;
    foreach(QString key,filed)
    {
        sFileds += key + ",";
    }
    sFileds.chop(1);

    sql += sFileds +" from "+sTableName;
    query.prepare(sql);

    qDebug()<<"sql--------"<<sql;
    bool suc = query.exec();
    if(suc)
    {
        while (query.next())
        {
            QJsonObject obj;
            for (int i = 0; i < query.record().count(); ++i)
            {
                QString fileName = query.record().fieldName(i);
                obj.insert(fileName,query.value(fileName).toJsonValue());
            }
            ret.push_back(obj);
        }
    }
    m_lastError = query.lastError().text();

    return ret;
}

int DAO::queryDataForBase(QString sTableName)
{
    bool result = false;
    int iRetNum = 0;
    QSqlQuery queryResult  = QSqlQuery(m_db);
    QString sql = QString("SELECT count(*) as num FROM %1 ").arg(sTableName);
    result = queryResult.exec(sql);
    m_lastError = queryResult.lastError().text();
    if(queryResult.next() && result)
    {
        iRetNum = queryResult.value("num").toInt();
    }

    return iRetNum;
}

QJsonArray DAO::queryDataForBase(QString sTableName, QList<QString> filed, QMap<QString, QVariant> where, int limit, QString orderBy)
{
    //SELECT id,age FROM `student` where age = 33
    QJsonArray ret;
    QSqlQuery query;
    query  = QSqlQuery(m_db);

    if(filed.isEmpty())
    {
        return ret;
    }
    QString sql = "select ";
    QString sFileds;
    QString sCons;
    QList<QString> sWhereList = where.keys();
    foreach(QString key,filed)
    {
        sFileds += key + ",";
    }
    sFileds.chop(1);
    QString wheres = " where 1=1";
    for(auto con : sWhereList)
    {
        wheres += " and " + con + "=:w" + con ;
    }
    sql += sFileds +" from "+sTableName + wheres;

    //是否只检索1条
    if(!orderBy.isEmpty())
    {
        sql += QString(" order by %1").arg(orderBy);
    }
    sql += QString(" limit %1").arg(limit);

    query.prepare(sql);
    for(auto con : sWhereList)
    {
        query.bindValue(":w" + con, where[con]);
    }
    qDebug()<<"sql--------"<<sql;
    bool suc = query.exec();
    if(suc)
    {
        while (query.next())
        {
            QJsonObject obj;
            for (int i = 0; i < query.record().count(); ++i)
            {
                QString fileName = query.record().fieldName(i);
                obj.insert(fileName,query.value(fileName).toJsonValue());
            }
            ret.push_back(obj);
        }
    }
    m_lastError = query.lastError().text();

    return ret;
}

QJsonArray DAO::queryDataForBase(QString sTableName, QList<QString> keyList, QMap<QString, QVariant> whereMap, QString pageNum, QString pageSize)
{
    QJsonArray ret;
    QSqlQuery query;
    query  = QSqlQuery(m_db);

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
        int index=0;
        for(QMap<QString,QVariant>::const_iterator i = whereMap.constBegin(); i != whereMap.constEnd(); i++)
        {
            if (i.key().endsWith("_like"))
            {
                auto key=i.key();
                key.chop(5);
                sql+=key+" like ";
                sql+=QString(":param%1").arg(QString::number(index))+" ";
            }
            else
            {
                sql+=i.key()+"=";
                sql+=QString(":param%1").arg(QString::number(index))+" ";
            }
            if((whereMap.size()-1)!=index)
            {
                sql+=" and ";
            }
            index++;
        }
    }

    //进行分页查询
    if(!pageNum.isEmpty() && !pageSize.isEmpty())
    {
        sql+= QString(" Limit %1 Offset %2").arg(pageSize).arg(QString::number(pageNum.toUInt() * pageSize.toInt()));
    }

    //操作数据库
    query.prepare(sql);
    if(whereMap.size() > 0)
    {
        int index=0;
        for(QMap<QString,QVariant>::const_iterator i = whereMap.constBegin(); i != whereMap.constEnd(); i++)
        {
            if (i.key().endsWith("_like"))
            {
                query.bindValue(QString(":param%1").arg(QString::number(index)),"%"+i.value().toString()+"%");
            }
            else
            {
                query.bindValue(QString(":param%1").arg(QString::number(index)),i.value());
            }
            index++;
        }
    }
    bool result = query.exec();

    if(result)
    {
        QSqlQuery queryAllNum;
        QString allNum = "";

        //查询总数
        sql = QString("select count(*) as allnum from %1").arg(sTableName);
        bool result = execSQL(queryAllNum, sql);
        if(result && queryAllNum.next()){
            allNum = queryAllNum.value("allnum").toString();
        }

        while (query.next())
        {
            QJsonObject obj;
            for (int i = 0; i < query.record().count(); ++i)
            {
                QString fileName = query.record().fieldName(i);
                obj.insert(toHump(fileName),query.value(fileName).toJsonValue());
                obj.insert("allNum", allNum);
            }
            ret.push_back(obj);
        }
    }

    return ret;
}

bool DAO:: deleteDataForBase(QString sTableName, QMap<QString, QVariant> where)
{

    bool ret = false;
    QSqlQuery query;
    query  = QSqlQuery(m_db);
    //delete from student where age = 33
    QList<QString> sWhereList = where.keys();
    QString sql = "delete from ";
    QString wheres = " where 1=1";
    for(auto con : sWhereList)
    {
        wheres += " and " + con + "=:w" + con ;
    }
    sql += sTableName + wheres;
    query.prepare(sql);
    for(auto con : sWhereList)
    {
        query.bindValue(":w" + con, where[con]);
    }
    ret = query.exec();
    m_lastError = query.lastError().text();

    return ret;
}

bool DAO::clearTableForBase(QString sTableName)
{
    QString sql = QString("delete from %1").arg(sTableName);
    return  execSQL(sql);
}
