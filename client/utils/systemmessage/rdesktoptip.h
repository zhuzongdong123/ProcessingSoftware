#ifndef RDESKTOPTIP_H
#define RDESKTOPTIP_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include "./globalmodule/restfulapi.h"

namespace Ui {
class RDesktopTip;
}

class RDesktopTip : public QWidget
{
    Q_OBJECT
    explicit RDesktopTip();
public:
    //动画模式枚举
    enum AnimationMode
    {
        //无动画
        NoAnimation = 0x00,
        //仅透明度动画
        OpacityAnimation = 0x01,
        //仅位置动画
        PosAnimation = 0x02,
        //全部动画
        //OpacityAnimation|PosAnimation
        AllAnimation = 0xFF
    };

    struct ALarmInfo
    {
        QString alarmPerson;    //报警人
        QString alarmPlace;     //报警地点
        QString alarmTime;      //报警时间
        QString alarmType;      //报警类型
        QString alarmDetails;   //报警详情
    };

public:
    ~RDesktopTip();

    //隐藏弹框
    static void hideTip();
    //设置动画模式
    static RDesktopTip::AnimationMode getMode();
    static void setMode(RDesktopTip::AnimationMode newMode);

    static void addAlarmInfo(const QString alarmPerson,const QString alarmPlace, const QString alarmTime,
                      const QString alarmType,const QString alarmDetails,const QString recvID);

    static void addInspectInfo(QString id, QString sentryPostName, QString sentryName,QString nextSentryName,QString inspectTime,QString nextSentryDutyTime);

    static void addStandSentryRemindInfo(QVariant variant);

public slots:
    void requestFinishedSlot(QNetworkReply *networkReply);
private:
    //初始化动画设置
    void initAnimation();
    //初始化定时器设置
    void initTimer();
    //准备定时器
    void readyTimer(int timeout);
    //启动显示动画-已显示动画重新开始
    void showAnimation();
    //启动显示动画-已显示不重复动画
    void keepAnimation();
    //启动隐藏动画
    void hideAnimation();
    //显示的文本
    void setTextList(const QString alarmPerson,const QString alarmPlace,const QString alarmTime,
                     const QString alarmType,const QString alarmDetails,const QString recvID);
    //显示的文本
    void setInspectTextList(QString id, QString sentryPostName, QString sentryName,QString nextSentryName,QString inspectTime,QString nextSentryDutyTime);

    //显示的文本
    void setStandSentryRemindList(QVariant variant);

private:
    Ui::RDesktopTip *ui;
    //唯一实例
    static RDesktopTip *instance;

    //动画设置
    static AnimationMode mode;
    //动画组
    QParallelAnimationGroup *showGroup;
    //保存动画结束状态
    bool showAnimEnd=false;
    //透明度属性动画
    QPropertyAnimation *showOpacity=nullptr;
    //位置属性动画
    QPropertyAnimation *showPos=nullptr;

    //定时关闭
    QTimer *hideTimer=nullptr;
    //定时计数
    int hideCount=0;

    RestFulApi m_restFulApi;

    QMap<QString, ALarmInfo> _aLarmInfoMap;
};

#endif // RDESKTOPTIP_H
