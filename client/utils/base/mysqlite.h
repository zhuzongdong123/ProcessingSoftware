#ifndef MYSQLITE_H
#define MYSQLITE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

class MySqlite : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief createDB 创建数据库，存在则返回
     * \param DBName 数据库名称
     * \return true:创建成功   false:创建失败
     */
    bool createDB(QString DBName);

    /*!
     * \brief deleteDB 删除数据库
     * \param DBName 数据库名称
     * \return true:成功   false:失败
     */
    bool deleteDB(QString DBName);

    /*!
     * \brief initTable 初始化表格+列+列类型
     * \param sTableName 表名称
     * \param sNameList 列名称
     * \param sType 列类型
     * \return true:创建成功或者已存在   false:创建失败
     */
    bool initTable(QString sTableName, QStringList sNameList, QStringList sType);

    /*!
     * \brief insertDataForBase 增加1条数据到数据库
     * \param sTableName 表名称
     * \param data 数据集合
     * \return true:成功   false:失败
     */
    bool insertDataForBase(QString sTableName, QMap<QString, QString> data, bool isOpenTransaction = true);

    /*!
     * \brief insertDatasForBase 批量导入数据到数据库
     * \param sTableName 表名称
     * \param data 数据集合
     * \return true:成功   false:失败
     */
    bool insertDatasForBase(QString sTableName, QStringList sNameList, QVector<QVariantList> datas, bool isOpenTransaction = true);

    /*!
     * \brief deleteDataFromTable 删除符合条件的某一条数据
     * \param sTableName 表名称
     * \param queryCondition 查询条件
     * \return true:成功   false:失败
     */
    bool deleteDataFromTable(QString sTableName, QString queryCondition);

    /*!
     * \brief deleteTable 删除表
     * \param sTableName 表名称
     * \return true:成功   false:失败
     */
    bool clearTable(QString sTableName, bool isOpenTransaction = true);

    /*!
     * \brief updateDataForBase 更新某一条数据
     * \param sTableName 表名称
     * \param sDataList 数据集合
     * \param index 下标
     * \return true:成功   false:失败
     */
    bool updateDataForBase(QString sTableName, QStringList sDataList,int index);

    /*!
     * \brief queryDataForBase  查询数据
     * \param sTableName      表名称
     * \param queryResult       查询结果
     * \param keyList               列
     * \param whereMap         查询条件
     * \param isSelectOnce     是否只检索一条数据
     * \return  true:成功   false:失败
     */
    bool queryDataForBase(QString sTableName, QSqlQuery& queryResult,  QList<QString> keyList, QMap<QString, QString> whereMap, bool isSelectOnce = false);

    /*!
     * \brief execSQL   执行sql语句
     * \param queryResult   执行结果
     * \param sql   sql语句
     * \return
     */
    bool execSQL(QSqlQuery& queryResult, QString sql);

    /*!
     * \brief getDB 获取数据库
     * \return
     */
    QSqlDatabase& getDB();

    static MySqlite *getInstance();
signals:

private:
    explicit MySqlite(QObject *parent = nullptr);
    ~MySqlite();

private:
    QSqlDatabase m_db;
};

#endif // MYSQLITE_H
