#include <QApplication>
#include "loginwidget.h"
#include <QProcess>
#include"xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxworksheet.h"
#include "QDebug"
#include "xlsxworkbook.h"

bool checkTrialPeriod() {
    QSettings settings("sdgs", "ProcessingSoftWare");

    // 检查是否是首次运行
    if (!settings.contains("firstRunDate"))  {
        settings.setValue("firstRunDate",  QDateTime::currentDateTime().toString(Qt::ISODate));
        return true;
    }

    // 获取首次运行日期
    QDateTime firstRunDate = QDateTime::fromString(settings.value("firstRunDate").toString(),  Qt::ISODate);
    QDateTime currentDate = QDateTime::currentDateTime();

    // 计算试用期剩余天数
    qint64 daysElapsed = firstRunDate.daysTo(currentDate);
    qint64 trialDays = 30;
    if (daysElapsed >= trialDays) {
        return false;
    } else {
        return true;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    if (!checkTrialPeriod()) {
//        return -1; // 退出程序
//    }

    // 设置环境变量（必须在创建QNetworkAccessManager之前）
    qputenv("QT_NETWORK_CONNECTION_LIMIT", "200");       // 全局最大连接数
    qputenv("QT_NETWORK_CONNECTION_LIMIT_PER_HOST", "20"); // 每个主机的最大连接数

    LoginWidget w;
    w.show();
    return a.exec();
}
