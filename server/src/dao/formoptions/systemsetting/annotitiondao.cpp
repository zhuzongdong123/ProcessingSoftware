#include "annotitiondao.h"
#include <QUuid>
#include <QJsonDocument>
#include <QDebug>

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

bool AnnotitionDao::addPlottingEvent(QJsonObject objData)
{
    QJsonArray array = objData.value("data").toArray();
    if(array.size() == 0)
    {
        return true;
    }

    for(auto data : array)
    {
        QJsonObject currentObj = data.toObject();
        QString bagId = currentObj.value("bagId").toString();
        QString imageId = currentObj.value("imageId").toString();
        QString bagData = currentObj.value("data").toString();

        if(bagId.isEmpty() || imageId.isEmpty())
        {
            continue;
        }

        QJsonArray arrayTemp;
        QJsonObject objWhereTemp;
        objWhereTemp.insert("bag_id",bagId);
        objWhereTemp.insert("image_id",imageId);
        bool result = getAllEvents(objWhereTemp, arrayTemp);
        if(result)
        {
            if(arrayTemp.size() == 0)
            {
               //直接新增
                QMap<QString, QVariantList> data;
                data.insert("bag_id",QVariantList() << QVariant(bagId));
                data.insert("image_id",QVariantList() << QVariant(imageId));
                data.insert("data",QVariantList() << QVariant(bagData));
                insertDataForBaseList("events",data);
            }
            //更新数据
            else
            {
                QJsonObject obj = arrayTemp[0].toObject();
                QString bagDataTemp = obj.value("data").toString();
                QJsonArray bagDataOldArray = QJsonDocument::fromJson(bagDataTemp.toUtf8()).array();
                QJsonArray bagDataInsertArray = QJsonDocument::fromJson(bagData.toUtf8()).array();
                QJsonArray bagDataNewArray;
                for(auto temp : bagDataOldArray)
                {
                    bagDataNewArray.push_back(temp);
                }
                for(auto temp : bagDataInsertArray)
                {
                    //判断是否存在当前的标绘结果，不包含的话插入
                    if(!checkIsExit(bagDataNewArray,temp.toObject()))
                    {
                        bagDataNewArray.push_back(temp);
                    }
                }

                //删除
                QMap<QString, QVariant> whereMap;
                whereMap.insert("bag_id",bagId);
                whereMap.insert("image_id",imageId);
                deleteDataForBase("events", whereMap);

                QMap<QString, QVariantList> data;
                data.insert("bag_id",QVariantList() << QVariant(bagId));
                data.insert("image_id",QVariantList() << QVariant(imageId));

                QJsonDocument document;
                QByteArray post_param;
                document.setArray(bagDataNewArray);
                post_param = document.toJson(QJsonDocument::Compact);
                QString bagData = post_param;
                data.insert("data",QVariantList() << QVariant(bagData));

                insertDataForBaseList("events",data);
            }
        }
    }

    return true;
}

bool AnnotitionDao::checkIsExit(QJsonArray allArray, QJsonObject currentData)
{
    QMap<QString,QString> allMap;
    for(auto dataObj : allArray)
    {
        QString key = QString("%1,%2,%3,%4,%5")
                .arg(dataObj.toObject().value("event_type").toString())
                .arg(dataObj.toObject().value("x1").toInt())
                .arg(dataObj.toObject().value("x2").toInt())
                .arg(dataObj.toObject().value("y1").toInt())
                .arg(dataObj.toObject().value("y2").toInt());
        allMap.insert(key,key);
    }

    QString currentKey = QString("%1,%2,%3,%4,%5")
            .arg(currentData.value("event_type").toString())
            .arg(currentData.value("x1").toInt())
            .arg(currentData.value("x2").toInt())
            .arg(currentData.value("y1").toInt())
            .arg(currentData.value("y2").toInt());

    if(allMap.find(currentKey) != allMap.end())
    {
        return true;
    }
    else
    {
        return false;
    }
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
