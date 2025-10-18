#include <QApplication>
#include "loginwidget.h"
#include <QProcess>
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
    if (!checkTrialPeriod()) {
        return -1; // 退出程序
    }

    LoginWidget w;
    w.show();
    return a.exec();
}
