/*
 * @file dao.h
 * @brief dao层基类，实现对表格控制的基本通用方法
 * @author jason
 * @date 2023-10-17
 */
#ifndef DAO_H
#define DAO_H

#include <QStringList>
#include <QString>
#include <QVariantList>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlRecord>

class DAO
{
public:
    DAO();
public:
    QString toHump(QString value);
    /*
    * @Function  openConnection
    * @Description 打开数据库连接，每次数据库操作前调用这个函数
    * @param 入参(in) QString threadId  数据库连接名，即线程唯一表示
    * @return  返回值(return)  bool 数据库打开成功返回true否则false
    */
    bool openConnection(QString threadId);
    bool execSQL(QStringList sql);//批量执行sql
    bool execSQL(QString sql);//执行一条sql
    //执行一条sql，并返回query
    bool execSQL(QSqlQuery& queryResult, QString sql);
    /*
    * @Function  insertDataForBaseList
    * @Description 通用插入数据
    * @param 入参(in) QString sTableName  表名
    * @param 参(in) QMap<QString, QVariantList> data  插入的数据，key为数据库字段名，value为字段对应的数据
    * @return  返回值(return)  bool 成功true失败false
    */
    bool insertDataForBaseList(QString sTableName, QMap<QString, QVariantList> data);
    /*
    * @Function  updateDataForBase
    * @Description 更新数据通用方法
    * @param 入参(in) QString sTableName  表名
    * @param 入参(in) QMap<QString,QVariant> update  需要更新的数据key:字段名称 value更新的字段对应的值
    * @param 入参(in) QMap<QString,QVariant>  where 更新的条件key:字段名称 value更新的字段对应的值
    * @return  返回值(return)  bool 成功true失败false
    */
    bool updateDataForBase(QString sTableName,QMap<QString,QVariant> update,QMap<QString,QVariant> where);
    /*
    * @Function  updateDataForBase
    * @Description 重载更新方法，不需要条件
    * @param 入参(in) QMap<QString,QVariant> update  需要更新的数据key:字段名称 value更新的字段对应的值
    * @return  返回值(return)  bool 成功true失败false
    */
    bool updateDataForBase(QString sTableName,QMap<QString,QVariant> update);
    /*
    * @Function  queryDataForBase
    * @Description 查询通用方法,无查询条件
    * @param 入参(in) QString sTableName  表名
    * @param 入参(in) QList<QString> filed  查询的数据表里字段
    * @return  返回值(return)  QJsonArray 查询的数据集合
    */
    QJsonArray queryDataForBase(QString sTableName,QList<QString> filed);
    int queryDataForBase(QString sTableName);
    /*
    * @Function  queryDataForBase
    * @Description 查询通用方法
    * @param 入参(in) QString sTableName  表名
    * @param 入参(in) QList<QString> filed  查询的数据表里字段
    * @param 入参(in) QQMap<QString,QVariant> where  查询的数据条件，key字段名词  value 字段对应的值
    * @return  返回值(return)  QJsonArray 查询的数据集合
    */
    QJsonArray queryDataForBase(QString sTableName,QList<QString> filed,QMap<QString,QVariant> where);
    /*
    * @Function  queryDataForBase
    * @Description 分页查询通用方法（_like结尾的都是模糊查询）重载函数不要有默认值
    * @param 入参(in) QString sTableName  表名
    * @param 入参(in) QList<QString> filed  查询的数据表里字段
    * @param 入参(in) QQMap<QString,QVariant> where  查询的数据条件，key字段名词  value 字段对应的值
    * @return  返回值(return)  QJsonArray 查询的数据集合
    */
    QJsonArray queryDataForBase(QString sTableName,QList<QString> keyList ,QMap<QString,QVariant> whereMap , QString pageNum, QString pageSize);
    /*
    * @Function  deleteDataForBase
    * @Description 删除通用方法
    * @param 入参(in) QString sTableName  表名
    * @param 入参(in) QMap<QString,QVariant> where  删除的条件key 字段名 value：字段对应值
    * @return  返回值(return)  bool 成功true失败false
    */
    bool deleteDataForBase(QString sTableName,QMap<QString,QVariant> where);
    /*
    * @Function  clearTableForBase
    * @Description 清空表里数据
    * @param 入参(in) QString sTableName  表名
    * @return  返回值(return)  bool 成功true失败false
    */
    bool clearTableForBase(QString sTableName);

    QString m_lastError = "数据库连接错误";
    QString fromHump(QString value);
private:

    QSqlDatabase m_db; //数据库连接实例
    QString m_threadId = "";
};

#endif // DAO_H
