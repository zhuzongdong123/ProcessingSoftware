#include "annotitiondao.h"
#include <QUuid>

AnnotitionDao::AnnotitionDao()
{

}

//先更新，更新失败则增加
bool AnnotitionDao::addOrUpdateIdentity(QJsonObject objWhere)
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
        return  insertDataForBaseList("annotition",data);
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

        return updateDataForBase("annotition", update, where);
    }
}

//分页查询
bool AnnotitionDao::queryIdentityResource(QJsonArray &resultArray, QJsonObject objWhere){
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

bool AnnotitionDao::deleteIdentityResource(QJsonObject objWhere)
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

bool AnnotitionDao::queryResource(QJsonObject objWhere, QJsonArray& array)
{
    QMap<QString, QVariant> whereMap;
    QList<QString> keyList;
    keyList.append("*");
    QJsonObject::iterator ite;
    for(ite = objWhere.begin(); ite != objWhere.end(); ite++)
    {
        QString key = fromHump(ite.key());
        whereMap.insert(key,ite.value().toVariant());
    }

    array = queryDataForBase(m_tableName, keyList, whereMap, 1, "time desc");
    return true;
}

bool AnnotitionDao::addEvents(QJsonObject objData)
{
    QJsonArray array = objData.value("data").toArray();
    //先删除所有相同bagId的数据
    if(array.size() == 0)
    {
        return true;
    }
    else
    {
        QString bagId = array[0].toObject().value("bag_id").toString();
        {
            QMap<QString, QVariant> whereMap;
            whereMap.insert("bag_id",bagId);
            deleteDataForBase("events", whereMap);
        }

        QVariantList bagIdList;
        QVariantList dataList;
        QVariantList imageIdList;
        QVariantList UUIDList;
        for(auto eventObj : array)
        {
            QJsonObject objTemp = eventObj.toObject();
            bagIdList << objTemp.value("bag_id").toString();
            imageIdList << objTemp.value("image_id").toString();
            dataList << objTemp.value("data").toString();
        }

        //批量插入数据
        QMap<QString, QVariantList> data;
        data.insert("bag_id",bagIdList);
        data.insert("image_id",imageIdList);
        data.insert("data",dataList);
        bool result = insertDatasForBaseList("events",data);
        return result;
    }
    return true;
}

bool AnnotitionDao::getAllEvents(QJsonObject objWhere, QJsonArray& array)
{
    QMap<QString, QVariant> whereMap;
    QList<QString> keyList;
    keyList.append("*");
    QJsonObject::iterator ite;
    for(ite = objWhere.begin(); ite != objWhere.end(); ite++)
    {
        QString key = ite.key();
        whereMap.insert(key,ite.value().toVariant());
    }

    array = queryDataForBase("events", keyList, whereMap);
    return true;
}

//获取错误原因
QString AnnotitionDao::getLastError()
{
    return m_lastError;
}
