#include "identitydao.h"
#include <QUuid>

IdentityDao::IdentityDao()
{

}

//先更新，更新失败则增加
bool IdentityDao::addOrUpdateIdentity(QJsonObject objWhere)
{
    QString id = objWhere.value("id").toString();

    //新增
    if(id.isEmpty())
    {
        id = QUuid::createUuid().toString().remove("-").remove("{").remove("}");
        QMap<QString, QVariantList> data;
        QJsonObject::iterator ite;
        for(ite = objWhere.begin(); ite != objWhere.end(); ite++)
        {
            QString key = fromHump(ite.key());
            data.insert(key,QVariantList() << objWhere.value(ite.key()).toVariant());
        }
        data.insert("id",QVariantList() << QVariant(id));
        return  insertDataForBaseList("auth_identity",data);
    }
    else
    {
        QMap<QString,QVariant> update;
        QMap<QString,QVariant> where;
        where.insert("id", objWhere.value("id").toString());

        QJsonObject::iterator ite;
        for(ite = objWhere.begin(); ite != objWhere.end(); ite++)
        {
            QString key = fromHump(ite.key());
            if("id" == key)
            {
                continue;
            }
            update.insert(key,objWhere.value(ite.key()).toVariant());
        }

        return updateDataForBase("auth_identity", update, where);
    }
}

//分页查询
bool IdentityDao::queryIdentityResource(QJsonArray &resultArray, QJsonObject objWhere){
    QString pageSize = "";
    QString pageNo = "";
    QList<QString> keyList;
    QMap<QString, QVariant> whereMap;
    if(!objWhere.value("pageSize").toVariant().isNull())
    {
        pageSize = QString::number(objWhere.value("pageSize").toInt());
    }

    if(!objWhere.value("pageNo").toVariant().isNull())
    {
        pageNo = QString::number(objWhere.value("pageNo").toInt() - 1);
    }

    if(!objWhere.value("params").isNull())
    {
        auto objParams = objWhere.value("params").toObject();
        QJsonObject::iterator ite;
        for(ite = objParams.begin(); ite != objParams.end(); ite++)
        {
            QString key = fromHump(ite.key());
            whereMap.insert(key,ite.value().toVariant());
        }
    }

    resultArray = queryDataForBase(m_tableName, keyList, whereMap, pageNo, pageSize);
    return true;
}

bool IdentityDao::deleteIdentityResource(QJsonObject objWhere)
{
    if(objWhere.isEmpty())
    {
        m_lastError = "删除条件不能为空";
        return false;
    }
    QMap<QString, QVariant> whereMap;
    QJsonObject::iterator ite;
    for(ite = objWhere.begin(); ite != objWhere.end(); ite++)
    {
        QString key = fromHump(ite.key());
        whereMap.insert(key,ite.value().toVariant());
    }
    auto result = deleteDataForBase(m_tableName, whereMap);
    return result;
}

//验证密码
bool IdentityDao::queryPwdIdentityResource(QJsonObject &resultObj, QJsonObject objWhere){

    QJsonObject objReturn;
    QSqlQuery queryResult;
    QString acc = objWhere.value("acc").toString();
    QString pwd = objWhere.value("pwd").toString();
    if(acc.isEmpty() || pwd.isEmpty())
    {
        m_lastError = "用户名和密码不能为空";
        return false;
    }

    QString sql = QString("select * from %1 where acc = '%2' and pwd='%3'")
            .arg(m_tableName)
            .arg(acc)
            .arg(pwd);
    auto result = execSQL(queryResult, sql);
    if (result)
    {
        while (queryResult.next())
        {
            for (int i = 0; i < queryResult.record().count(); ++i)
            {
                auto name = queryResult.record().fieldName(i);
                QVariant temp = queryResult.value(name);
                resultObj.insert(toHump(name),queryResult.value(name).toString());
            }
        }
    }

    if(resultObj.isEmpty())
    {
        m_lastError = "用户名或密码不正确";
        return false;
    }
    else
    {
        return true;
    }
}

//获取错误原因
QString IdentityDao::getLastError()
{
    return m_lastError;
}
