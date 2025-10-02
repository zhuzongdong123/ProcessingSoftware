#include "titlebar.h"
#include "ui_titlebar.h"
#include "tipsdlgviewForSure.h"
#include <QDateTime>
#include "appdatabasebase.h"

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
    ui->setupUi(this);

    connect(ui->closeBtn,&QPushButton::clicked,this,&TitleBar::slt_close);
    connect(ui->logoutBtn,&QPushButton::clicked,this,&TitleBar::sig_logout);

    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
        ui->system->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
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
