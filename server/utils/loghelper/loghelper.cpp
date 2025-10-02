#include "loghelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QTextStream>
#include <QDateTime>
#include <QThread>

//#define LOG_LEVEL QtDebugMsg
//#define LOG_DAYS 7
//#define LOG_PATH "/log/"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    LogHelper *pLogHelper = LogHelper::getInstance();
    if(nullptr == pLogHelper)
    {
        return;
    }
    QString strLogPath = pLogHelper->getLogPath();
    QtMsgType iLogType = pLogHelper->getLogType();
    int iLogDays = pLogHelper->getLogDays();


    QString strType("");
    switch (type)
    {
    case QtDebugMsg:
        strType = QString("Debug");
        break;
    case QtWarningMsg:
        strType = QString("Warning");
        break;
    case QtCriticalMsg:
        strType = QString("Critical");
        break;
    case QtFatalMsg:
        strType = QString("Fatal");
        break;
    case QtInfoMsg:
        strType = QString("Info");
        break;
    }

    // 设置输出信息格式
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString strMessage = QString("Level:(%1) File:(%2) Line:(%3) Func:(%4) Thread:(0x%5)\n~~ %6 ~~\n%7\n")
        .arg(strType).arg(context.file).arg(context.line).arg(context.function).arg(QString::number((long)QThread::currentThreadId(), 16)).arg(strDateTime).arg(msg);

    //控制台输出
    if(pLogHelper->isConsoleOut(context.file))
    {
        fprintf(stderr, "%s", QString("Func(%1)==>\n %2\n").arg(context.function).arg(strType + msg).toLocal8Bit().data());
    }

    //判断是否符合日志等级及日志天数
    if (iLogType > type || 0 >= iLogDays)
    {
        return;
    }

    // 加锁
    static QMutex mutex;
    mutex.lock();

    QString strLogFile = QString("%1%2_%3.log").arg(strLogPath).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")).arg(qAppName());
    QFile fileLog(strLogFile);
    if (!fileLog.exists())
    {
        QDir dirt(strLogPath);
        if(!dirt.exists())
        {
            dirt.mkdir(strLogPath);
        }

        QStringList strFilterst;
        strFilterst << QString("*.log") << QString("*.LOG");
        QFileInfoList file_list = dirt.entryInfoList(strFilterst, QDir::Files | QDir::NoSymLinks);

        if (file_list.size() > 0)
        {
            for (int i = 0; i < file_list.size(); ++i)
            {
                QString strName = file_list[i].fileName();
                QDate dataFile = QDate::fromString((strName.left(10)), "yyyy-MM-dd");
                if(dataFile.isValid())
                {
                    if (dataFile <= QDate::currentDate().addDays(-iLogDays))
                    {
                        QFile::remove(file_list[i].absoluteFilePath());
                    }
                }
            }
        }
    }

    fileLog.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&fileLog);
    // 输出信息至文件中（读写、追加形式）
    stream << strMessage << "\r\n";
    fileLog.flush();
    fileLog.close();

    // 解锁
    mutex.unlock();
}

//获取单例类的实例
LogHelper *LogHelper::getInstance()
{
    static LogHelper logHelper;
    return &logHelper;
}

//日志助手初始化
void LogHelper::init(QString strLogPath, QtMsgType iLogType, int iLogDays)
{
    m_strLogPath = strLogPath;
    m_iLogType = iLogType;
    m_iLogDays = iLogDays;

    // 安装消息处理程序
    qInstallMessageHandler(myMessageOutput);
    qDebug() << "日志助手初始化成功，路径" << strLogPath << "级别" << iLogType << "存储天数" << iLogDays;
}

//获取日志路径
QString LogHelper::getLogPath()
{
    return m_strLogPath;
}

//获取日志路径
QtMsgType LogHelper::getLogType()
{
    return m_iLogType;
}

//获取日志路径
int LogHelper::getLogDays()
{
    return m_iLogDays;
}

//增加控制台输出文件
void LogHelper::addConsoleFile(QString strFile)
{
    m_setConsoleFile.insert(strFile);
}

//是否允许控制台输出
bool LogHelper::isConsoleOut(QString strFile)
{
    return m_setConsoleFile.contains(strFile);
}
