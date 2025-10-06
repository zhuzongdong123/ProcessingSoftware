//数据管理页面
#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QWidget>
#include "restfulapi.h"

namespace Ui {
class DataManager;
}

class DataManager : public QWidget
{
    Q_OBJECT

public:
    explicit DataManager(QWidget *parent = nullptr);
    ~DataManager();

    struct RecordInfo
    {
        bool checkBox = false;
        QString ID;
        QString fileGroup;
        QString fileName;
        QString filePath;
        QString createTime;
        QString handleStatus;
        QString drawStatus;
//        QString drawPerson;
//        QString drawStartTime;
//        QString drawEndTime;
        QString dataFrom;
        QString memory;
        QString operation;
    };

    enum COLNAME
    {
        ID = 0,
        fileGroup,
        fileName,
        filePath,
        createTime,
        handleStatus,
        drawStatus,
//        drawPerson,
//        drawStartTime,
//        drawEndTime,
        dataFrom,
        memory,
        operation,
        UUID
    };

signals:
    void sig_turn2BagDetialPage(QString id);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void slt_requestFinishedSlot(QNetworkReply *networkReply);
    void slt_operateBtnClicked();

private:
    Ui::DataManager *ui;
    RestFulApi m_restFulApi;
    QVector<RecordInfo> _recordInfoVec;     //所有记录。作为成员变量，后期查询用
    bool m_isGetSysInfo = false;

    //统计数量
    int m_allCount = 0;//全部数量
    int m_allHandleCount = 0;//处理数量
    int m_allunHandleCount = 0;//未处理数量

private:
    //获取各种统计数量
    void getBagCounts();

    //获取表格内容
    void getTableInfo();

    //获取系统占用信息
    void getSysInfo();

    //获取所有文件的标注状态
    void getAllAnnotationStatus();

    //清空表格
    void clearTable();

    //初始化表格
    void initTable();

    //显示表格内容
    void resetTableInfo(QJsonObject objResult = QJsonObject());

    //解析数据
    void processData(QString id);

    //标注数据
    void annotationData(QString id);

    //删除数据
    void delData(QString id);
};

#endif // DATAMANAGER_H
