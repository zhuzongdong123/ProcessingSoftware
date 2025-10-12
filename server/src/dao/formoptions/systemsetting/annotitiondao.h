/**
 * @file basicsettings.h
 * @brief   身份相关数据库操作
 * @author 马存亮
 * @date 2023-10-20
 */
#ifndef AnnotitionDao_H
#define AnnotitionDao_H
#include "dao.h"

class AnnotitionDao : public  DAO
{
public:
    AnnotitionDao();
public:
    /**
    * @brief queryIdentityResource 分页查询数据源
    */
    bool queryIdentityResource(QJsonArray &resultArray, QJsonObject objWhere);

    /**
    * @brief getLastError 获取错误原因
    */
    QString getLastError();

    /**
    * @brief addOrUpdateIdentity 更新失败就增加
    */
    bool addOrUpdateIdentity(QJsonObject objWhere);

    /**
    * @brief deleteIdentityResource 删除数据
    */
    bool deleteIdentityResource(QJsonObject objWhere);

    /**
    * @brief queryResource 查询数据
    */
    bool queryResource(QJsonObject objWhere, QJsonArray& array);

    //新增事件
    bool addEvents(QJsonObject objData);

    //获取所有的事件
    bool getAllEvents(QJsonObject objWhere, QJsonArray& array);

    QString m_tableName = "annotition"; //表名
private:

};

#endif // AnnotitionDao_H
