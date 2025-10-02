/**
 * @file loghelper.h
 * @brief
 * 日志助手头文件
 * @author 丁达
 * @date 2022-07-16
 */

#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QObject>
#include <QSet>

class LogHelper : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief getInstance 获取单例类的实例
     * @return AppCommon 返回类的实例
     */
    static LogHelper* getInstance();

    /**
    * @brief 日志助手初始化
    * @param 入参(in)： strLogPath 日志路径
    * @param 入参(in)： iLogType 日志类型（默认QtDebugMsg）
    * @param 入参(in)： iLogDays 日志天数（默认7）
    */
    void init(QString strLogPath, QtMsgType iLogType = QtDebugMsg, int iLogDays = 7);

    /**
    * @brief 获取日志路径
    * @return 返回值(return)： QString 日志路径
    */
    QString getLogPath();

    /**
    * @brief 获取日志路径
    * @return 返回值(return)： QtMsgType 日志类型
    */
    QtMsgType getLogType();

    /**
    * @brief 获取日志路径
    * @return 返回值(return)： int 日志天数
    */
    int getLogDays();

    /**
    * @brief 增加控制台输出文件
    * @param 入参(in)： strFile 代码所在文件名称
    */
    void addConsoleFile(QString strFile);

    /**
    * @brief 是否允许控制台输出
    * @param 入参(in)： strFile 代码所在文件名称
    * @return 返回值(return)： bool （true: 允许，false: 禁止）
    */
    bool isConsoleOut(QString strFile);

private:
    QString m_strLogPath = "";          //日志路径
    QtMsgType m_iLogType = QtDebugMsg;  //日志类型
    int m_iLogDays = 0;                 //日志天数
    QSet<QString> m_setConsoleFile;      //控制台输出文件集合
};

#endif // LOGHELPER_H
