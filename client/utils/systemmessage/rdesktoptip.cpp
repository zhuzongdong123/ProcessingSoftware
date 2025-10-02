#include "rdesktoptip.h"
#include "ui_RDesktopTip.h"
#include "./messagemodule/alarmmessage/receivealarm.h"
#include <QApplication>
#include <QTime>
#include <QScreen>
#include <QDebug>
#include <memory>
#include <./globalmodule/globalparams.h>
#include <QJsonObject>
#include <QJsonDocument>
using namespace std;

RDesktopTip* RDesktopTip::instance=nullptr;
RDesktopTip::AnimationMode RDesktopTip::mode=RDesktopTip::AllAnimation;

RDesktopTip::RDesktopTip()
    : QWidget(nullptr),
      ui(new Ui::RDesktopTip),
      showGroup(new QParallelAnimationGroup(this))
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowModality(Qt::WindowModal);

    //resize会被label撑开
    setFixedSize(400,250);

    //关闭
    connect(ui->btnClose,&QPushButton::clicked,this,&RDesktopTip::hideTip);
    //程序退出时释放
    connect(qApp,&QApplication::aboutToQuit,this,&RDesktopTip::close);
    //动画设置
    initAnimation();
    //定时器设置
    initTimer();

    connect(&this->m_restFulApi.getAccessManager(), SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinishedSlot(QNetworkReply*)));
}

RDesktopTip::~RDesktopTip()
{
    qDebug()<<"delete DesktopTip";
    instance=nullptr;
    delete ui;
}

void RDesktopTip::hideTip()
{
    if(!instance){
        return;
    }
    instance->hideAnimation();
}

RDesktopTip::AnimationMode RDesktopTip::getMode()
{
    return mode;
}

void RDesktopTip::setMode(RDesktopTip::AnimationMode newMode)
{
    if(mode!=newMode){
        mode=newMode;
    }
}

void RDesktopTip::initAnimation()
{
    //透明度动画
    showOpacity=new QPropertyAnimation(this,"windowOpacity");
    //判断是否设置了此模式的动画
    if(mode&AnimationMode::OpacityAnimation){
        showOpacity->setDuration(1500);
        showOpacity->setStartValue(0);
    }else{
        showOpacity->setDuration(0);
        showOpacity->setStartValue(1);
    }
    showOpacity->setEndValue(1);
    showGroup->addAnimation(showOpacity);

    //位置动画
    showPos=new QPropertyAnimation(this,"pos");
    QScreen * screen = QGuiApplication::primaryScreen();
    if (screen) {
        const QRect desk_rect = screen->availableGeometry();
        const QPoint hide_pos{desk_rect.width()-this->width(),
                    desk_rect.height()};
        const QPoint show_pos{desk_rect.width()-this->width(),
                    desk_rect.height()-this->height()};
        //判断是否设置了此模式的动画
        if(mode&AnimationMode::PosAnimation){
            showPos->setDuration(1500);
            showPos->setStartValue(hide_pos);
        }else{
            showPos->setDuration(0);
            showPos->setStartValue(show_pos);
        }
        showPos->setEndValue(show_pos);
    }
    showGroup->addAnimation(showPos);
    //
    connect(showGroup,&QParallelAnimationGroup::finished,[this]{
        //back消失动画结束关闭窗口
        if(showGroup->direction()==QAbstractAnimation::Backward){
            //Qt::WA_DeleteOnClose后手动设置为null
//            instance=nullptr;
//            qApp->disconnect(this);
//            //关闭时设置为非模态，方式主窗口被遮挡，待测试
//            this->setWindowModality(Qt::NonModal);
//            this->close();
            showAnimEnd = false;
        }else{
            //配合keepAnimation
            showAnimEnd=true;
            //配合定时关闭
            if(hideCount>0)
                hideTimer->start();
        }
    });
}

void RDesktopTip::initTimer()
{
    hideTimer=new QTimer(this);
    hideTimer->setInterval(1000); //1s间隔
    connect(hideTimer,&QTimer::timeout,[this]{
        if(hideCount>1){
            hideCount--;
            ui->btnClose->setText(QString("%1 S").arg(hideCount));
        }else{
            //ui->btnClose->setText(QString("Close"));
            hideTimer->stop();
            hideTip();
        }
    });
}

void RDesktopTip::readyTimer(int timeout)
{
    //先设置，在显示动画结束再start开始计时器
    hideCount=timeout;
    hideTimer->stop();

    if(hideCount>0){
        ui->btnClose->setText(QString("%1 S").arg(hideCount));
    }else{
        //ui->btnClose->setText(QString("Close"));
    }
}

void RDesktopTip::showAnimation()
{
    showGroup->setDirection(QAbstractAnimation::Forward);
    //停止正在进行的动画重新
    if(showGroup->state()==QAbstractAnimation::Running){
        showGroup->stop();
    }
    showGroup->start();
    show();
}

void RDesktopTip::keepAnimation()
{
    //show没有完成，或者正在动画中才进入
    if(!showAnimEnd||showGroup->state()!=QAbstractAnimation::Stopped){
        showGroup->setDirection(QAbstractAnimation::Forward);
        showGroup->start();
        show();
    }else {
        showAnimation();
    }

}

void RDesktopTip::hideAnimation()
{
    //Backward反向执行动画
    showGroup->setDirection(QAbstractAnimation::Backward);
    showGroup->start();
}

void RDesktopTip::setTextList(const QString alarmPerson,const QString alarmPlace,const QString alarmTime,
                              const QString alarmType,const QString alarmDetails,const QString recvID)
{
    ALarmInfo alarmInfo;
    alarmInfo.alarmPerson = alarmPerson;
    alarmInfo.alarmPlace = alarmPlace;
    alarmInfo.alarmTime = alarmTime;
    alarmInfo.alarmType = alarmType;
    alarmInfo.alarmDetails = "";

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int id = qrand();

    //保存关联
    _aLarmInfoMap.insert(alarmPerson + QString::number(id),alarmInfo);

    ui->layout_labels->removeItem(ui->verticalSpacer);

    //画面显示内容
    QPushButton* button = new QPushButton(this);
    button->setStyleSheet("QPushButton{text-align : left;border:none;}");
    button->setObjectName(alarmPerson + QString::number(id));
    button->setText(tr("报警地点:") + alarmPlace+"," +tr("报警人:") + alarmPerson);
    int count = ui->layout_labels->count();
    ui->layout_labels->addWidget(button,count);
    ui->layout_labels->setStretch(count, 1);
    ui->layout_labels->addItem(ui->verticalSpacer);
    ui->layout_labels->setStretch(count+1, 100);

    connect(button,&QPushButton::clicked,this,[=](){
        QString id = button->objectName();
        ALarmInfo alarmInfo;
        if(_aLarmInfoMap.find(id) != _aLarmInfoMap.end())
        {
            alarmInfo = _aLarmInfoMap.find((id)).value();
        }
        else {
            return;
        }

        ui->layout_labels->removeWidget(button);
        button->deleteLater();

        int layoutCount = ui->layout_labels->count();
        if(layoutCount == 1)
        {
            hideTip();
        }

        //根据这个接收记录的ID，置为已读状态
        QJsonObject post_data;
        QJsonDocument document;
        QByteArray post_param;
        post_data.insert("readSentryIds","");
        post_data.insert("readSentryNames",GlobalParams::getInstance()->getUserName());
        QString timeHMS = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        post_data.insert("alarmReadTime",timeHMS);
        post_data.insert("alarmRead","1");
        document.setObject(post_data);
        post_param = document.toJson(QJsonDocument::Compact);
        QString DBPath = GlobalParams::getInstance()->getIniConfigReader().getDataBaseURL();
        QString url = DBPath + api_alarmrecv + recvID;
        this->m_restFulApi.visitUrl(url,VisitType::PUT,ReplyType::Alarmrecv_pull,true,post_param,"application/json");

        shared_ptr<ReceiveAlarm> receiveAlarmDialog = shared_ptr<ReceiveAlarm>(new ReceiveAlarm());
        receiveAlarmDialog->setAlarmInfo(alarmInfo.alarmPerson,alarmInfo.alarmPlace,alarmInfo.alarmTime,
                                         alarmInfo.alarmType,alarmInfo.alarmDetails);
        receiveAlarmDialog->exec();
    });
}

void RDesktopTip::setInspectTextList(QString id, QString sentryPostName, QString sentryName, QString nextSentryName, QString inspectTime, QString nextSentryDutyTime)
{
    ui->layout_labels->removeItem(ui->verticalSpacer);

    //画面显示内容
    QPushButton* button = new QPushButton(this);
    button->setStyleSheet("QPushButton{text-align : left;border:none;}");
    button->setText(tr("[") + sentryPostName+ tr("]") +tr("呼叫") + tr("[") + nextSentryName + tr("]"));
    int count = ui->layout_labels->count();
    ui->layout_labels->addWidget(button,count);
    ui->layout_labels->setStretch(count, 1);
    ui->layout_labels->addItem(ui->verticalSpacer);
    ui->layout_labels->setStretch(count+1, 100);

    connect(button,&QPushButton::clicked,this,[=](){
        ui->layout_labels->removeWidget(button);
        button->deleteLater();

        int layoutCount = ui->layout_labels->count();
        if(layoutCount == 1)
        {
            hideTip();
        }

        this->m_restFulApi.getPostData().clear();
        this->m_restFulApi.getPostData().addQueryItem("id",id);
        QString DBPath =  GlobalParams::getInstance()->getIniConfigReader().getDataBaseURL();
        QString url = DBPath + api_callrecord_read;
        this->m_restFulApi.visitUrl(url,VisitType::POST,ReplyType::callrecordRead);

//        shared_ptr<ReceiveAlarm> receiveAlarmDialog = shared_ptr<ReceiveAlarm>(new ReceiveAlarm());
//        receiveAlarmDialog->setInspectInfo(sentryPostName,sentryName,nextSentryName,inspectTime,nextSentryDutyTime);
//        receiveAlarmDialog->exec();
    });
}

void RDesktopTip::setStandSentryRemindList(QVariant variant)
{
    ui->layout_labels->removeItem(ui->verticalSpacer);

    ShowMsg showMsg = variant.value<ShowMsg>();
    //画面显示内容
    QPushButton* button = new QPushButton(this);
    button->setStyleSheet("QPushButton{text-align : left;border:none;}");

    if(showMsg.nextSentryNames == "") {

        button->setText(tr("[") + showMsg.sentryPostName+ tr("]") + "下班次未排班");
    } else {
        if(showMsg.msg == "空岗") {
        auto postListTableObj = CSingleton<StorageIDObj<PostListTable, QString>>::getInstance();
        auto postListTable=postListTableObj->getObjByID("PostListTable");
        QString newPeriod=showMsg.nextSentryDutyTime;
        if(postListTable!=nullptr)postListTable->setReminderPostID(showMsg.sentryPostID,newPeriod.replace("-","\n"));
            button->setText(tr("[") + showMsg.sentryPostName+ tr("]出现空岗!") + tr("下班次人员[") + showMsg.nextSentryNames+ tr("]还未上哨!"));
        } else {
            button->setText(tr("[") + showMsg.sentryPostName+ tr("]") + "的" + tr("[") + showMsg.nextSentryNames+ tr("]迟到上哨!"));
        }

    }

    int count = ui->layout_labels->count();
    ui->layout_labels->addWidget(button,count);
    ui->layout_labels->setStretch(count, 1);
    ui->layout_labels->addItem(ui->verticalSpacer);
    ui->layout_labels->setStretch(count+1, 100);

    connect(button,&QPushButton::clicked,this,[=](){
        ui->layout_labels->removeWidget(button);
        button->deleteLater();

        int layoutCount = ui->layout_labels->count();
        if(layoutCount == 1)
        {
            hideTip();
        }

        shared_ptr<ReceiveAlarm> receiveAlarmDialog = shared_ptr<ReceiveAlarm>(new ReceiveAlarm());
        receiveAlarmDialog->setStandSentryRemindInfo(showMsg);
        receiveAlarmDialog->exec();
    });
}

void RDesktopTip::addAlarmInfo(const QString alarmPerson,const QString alarmPlace,const QString alarmTime,
                               const QString alarmType,const QString alarmDetails, const QString recvID)
{
    if(!instance){
        //仅在ui线程
        instance=new RDesktopTip;
    }
    instance->readyTimer(0);
    //模态框
    instance->setWindowModality(Qt::WindowModal);

    instance->setTextList(alarmPerson,alarmPlace,alarmTime,alarmType,alarmDetails,recvID);

    instance->keepAnimation();
}

void RDesktopTip::addInspectInfo(QString id, QString sentryPostName, QString sentryName, QString nextSentryName, QString inspectTime,QString nextSentryDutyTime)
{
    if(!instance){
        //仅在ui线程
        instance=new RDesktopTip;
    }
    instance->readyTimer(0);
    //模态框
    instance->setWindowModality(Qt::WindowModal);

    instance->setInspectTextList(id,sentryPostName,sentryName,nextSentryName,inspectTime,nextSentryDutyTime);

    instance->keepAnimation();
}

void RDesktopTip::addStandSentryRemindInfo(QVariant variant)
{
    if(!instance){
        //仅在ui线程
        instance=new RDesktopTip;
    }
    instance->readyTimer(0);
    //模态框
    instance->setWindowModality(Qt::WindowModal);

    instance->setStandSentryRemindList(variant);

    instance->keepAnimation();
}

void RDesktopTip::requestFinishedSlot(QNetworkReply *networkReply)
{
    //获取报警记录
    if(replyTypeMap.value(networkReply)==ReplyType::Alarmrecv_pull)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();;
            qDebug().noquote() << obj.value("msg");
        }
        else
        {
           qDebug().noquote() << "请求出错";
        }
    }
}
