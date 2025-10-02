#include <QCoreApplication>

#include "httplistener.h"
#include <QSettings>
#include "hrhttpserver.h"
#include "loghelper.h"
#include <QTextCodec>
#include "appglobal.h"
#include "dbpool.h"
#include "qtsingleapplication.h"
int main(int argc, char *argv[])
{
    QtSingleApplication app("controllerdbserver",argc, argv);
    QObject::connect(&app,&QtSingleApplication::messageReceived,&app,[](QString message){qDebug() << message;});
    if(app.isRunning())
    {
        app.sendMessage("程序已运行，无法重复启动", 100);
        return EXIT_SUCCESS;
    }
    QString binDir=QCoreApplication::applicationDirPath();
    QString fileName = binDir + "/httpServer.ini";
    QSettings* listenerSettings=new QSettings(fileName,QSettings::IniFormat,&app);
    listenerSettings->setIniCodec(QTextCodec::codecForName("utf-8"));

    listenerSettings->beginGroup("database");
    QString dbType = listenerSettings->value("type").toString();
    if(dbType.isEmpty())
    {
        dbType = "QMYSQL";
        listenerSettings->setValue("type",dbType);
    }
    QString dbHost = listenerSettings->value("host").toString();
    if(dbHost.isEmpty())
    {
        dbHost = "127.0.0.1";
        listenerSettings->setValue("host",dbHost);
    }
    int dbPort = listenerSettings->value("port").toInt();
    if(0 == dbPort)
    {
        dbPort = 3306;
        listenerSettings->setValue("port",dbPort);
    }
    QString usrName = listenerSettings->value("user").toString();
    if(usrName.isEmpty())
    {
        usrName = "root";
        listenerSettings->setValue("user",usrName);
    }
    QString dbName = listenerSettings->value("name").toString();
    if(dbName.isEmpty())
    {
        dbName = "processing";
        listenerSettings->setValue("name",dbName);
    }
    QString dbPwd = listenerSettings->value("pwd").toString();
    if(dbPwd.isEmpty())
    {
        dbPwd = "root";
        listenerSettings->setValue("pwd",dbPwd);
    }
    DbPool::Instance().initDB(dbType,dbHost,dbPort,usrName,dbPwd,dbName);

    listenerSettings->endGroup();

    listenerSettings->beginGroup("listener");


    int iListenPort = listenerSettings->value("port").toUInt();
    if(iListenPort == 0)
    {
        listenerSettings->setValue("port",8083);
    }
    uint iMinNum = listenerSettings->value("minThreads").toUInt();
    if(0 == iMinNum)
    {
        listenerSettings->setValue("minThreads",4);
    }
    uint iMaxNum = listenerSettings->value("maxThreads").toUInt();
    if(0 == iMaxNum)
    {
        listenerSettings->setValue("maxThreads",100);
    }
    uint iClient = listenerSettings->value("cleanupInterval").toUInt();
    if(0 == iClient)
    {
        listenerSettings->setValue("cleanupInterval",60000);
    }
    uint iRead = listenerSettings->value("readTimeout").toUInt();
    if(0 == iRead)
    {
        listenerSettings->setValue("readTimeout",60000);
    }
    uint iMaxReq = listenerSettings->value("maxRequestSize").toUInt();
    if(0 == iMaxReq)
    {
        listenerSettings->setValue("maxRequestSize",10000000);

    }
    long long ulMaxMu = listenerSettings->value("maxMultiPartSize").toULongLong();
    if(0 == ulMaxMu)
    {
        listenerSettings->setValue("maxMultiPartSize",10000000);
    }
    HttpListener* httpListen = new HttpListener(listenerSettings,new hrHttpserver(&app),&app);
    //listenerSettings->endGroup();

    QString strLogPath = qApp->applicationDirPath() + "/log/";
    QtMsgType iLogType = QtDebugMsg;
    int iLogDays = 7;
    LogHelper::getInstance()->init(strLogPath, iLogType, iLogDays);

    AppGlobal::Instance().initDbStatus();

    int iret = app.exec();
    listenerSettings->deleteLater();
    httpListen->deleteLater();
    DbPool::Instance().release();
    return iret;
}
