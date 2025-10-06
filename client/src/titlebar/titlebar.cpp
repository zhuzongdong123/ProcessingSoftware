#include "titlebar.h"
#include "ui_titlebar.h"
#include "tipsdlgviewForSure.h"
#include <QDateTime>
#include "appdatabasebase.h"
#include "appeventbase.h"

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
    ui->setupUi(this);

    connect(ui->closeBtn,&QPushButton::clicked,this,&TitleBar::slt_close);
    connect(ui->logoutBtn,&QPushButton::clicked,this,&TitleBar::sig_logout);
    ui->serverError->setVisible(false);

    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        ui->system->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    });

    connect(AppEventBase::getInstance(),&AppEventBase::sig_sendServerStatus,this,[=](bool status){
        ui->serverError->setVisible(!status);

        if(!status)
        {
            if(!m_isCreateAnimation)
            {
                // 创建透明度效果
                m_effect = new QGraphicsOpacityEffect(ui->serverError);
                ui->serverError->setGraphicsEffect(m_effect);

                // 创建动画
                m_anim = new QPropertyAnimation(m_effect, "opacity");
                m_anim->setDuration(1500); // 动画周期1秒
                m_anim->setStartValue(1.0); // 完全不透明
                m_anim->setKeyValueAt(0.5, 0.2); // 中间点透明度
                m_anim->setEndValue(1.0); // 回到不透明
                m_anim->setLoopCount(-1); // 无限循环
                m_anim->start();
            }

            m_effect->setOpacity(1.0);
            m_anim->start();
        }
        else
        {
            if(nullptr != m_anim)
            {
                m_anim->stop();
            }
        }
    });

    timer->start(900);
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::showEvent(QShowEvent *event)
{
    ui->userBtn->setText(AppDatabaseBase::getInstance()->m_userName);
}

void TitleBar::slt_close()
{
    tipsdlgviewForSure box("是否退出程序？",nullptr);
    if(box.windowExec() == 1)
    {
        return;
    }

    QCoreApplication::quit();
}
