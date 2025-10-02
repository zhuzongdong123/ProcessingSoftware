/**
 * @file basicsettings.h
 * @brief   身份相关数据库操作
 * @author 马存亮
 * @date 2023-10-20
 */
#ifndef IDENTITYDAO_H
#define IDENTITYDAO_H
#include "dao.h"

class IdentityDao : public  DAO
{
public:
    IdentityDao();
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
    * @brief queryPwdIdentityResource 验证密码
    */
    bool queryPwdIdentityResource(QJsonArray &resultArray, QJsonObject objWhere);

    QString m_tableName = "auth_identity"; //表名
private:

};

#endif // IDENTITYDAO_H
