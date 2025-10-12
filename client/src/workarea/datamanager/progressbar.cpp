#include "progressbar.h"
#include "ui_progressbar.h"
#include <QDateTime>
#include "tipsdlgview.h"

ProgressBar::ProgressBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProgressBar)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ProgressBar::updateTime);
    connect(ui->closeBtn, &QPushButton::clicked, this, &ProgressBar::slt_closeBtnClicked);
}

ProgressBar::~ProgressBar()
{
    delete ui;
}

void ProgressBar::startStep1()
{
    ui->step1->setText(QString("%1 bag文件开始处理...").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));


    //记录开始时间并开始定时器
    startTracking();
}

void ProgressBar::startStep2()
{
    ui->step2->setText(QString("%1 bag文件解析完成...").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
}

void ProgressBar::startStep4()
{
    ui->step4->setText(QString("%1 bag文件结束处理...").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

    //结束定时器
    stopTracking();
}

void ProgressBar::setDownStep(double downStep)
{
    m_downStep = downStep;
}

void ProgressBar::setSaveStep(double saveStep)
{
    ui->step3->setText(QString("文件下载进度为%1%").arg(QString::number(saveStep*100,'f',1)));
}

void ProgressBar::closeEvent(QCloseEvent *event)
{
    stopTracking();

    m_mask.deleteMask(getMainWindow());
}

void ProgressBar::showEvent(QShowEvent *event)
{
    m_mask.insertMask(getMainWindow(),"background-color:rgb(0,0,0,100)",0.5);

    raise();
    show();
}

void ProgressBar::startTracking()
{
    m_elapsedTimer.start();   // 开始计时
    m_timer->start(1000);    // 每秒触发一次
}

void ProgressBar::stopTracking()
{
    m_timer->stop();
}

void ProgressBar:: updateTime() {
    qint64 elapsedMs = m_elapsedTimer.elapsed();   // 获取经过的毫秒数

    // 转换为时分秒格式
    int seconds = (elapsedMs / 1000) % 60;
    int minutes = (elapsedMs / (1000 * 60)) % 60;
    int hours = elapsedMs / (1000 * 60 * 60);

    // 格式化显示
    QString timeStr = QString("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));

    ui->useTime->setText("总耗时: " + timeStr);
}

void ProgressBar::slt_closeBtnClicked()
{
    if(ui->step4->text().isEmpty())
    {
        TipsDlgView* dlg = new TipsDlgView("请等待下载结束", nullptr);
        dlg->startTimer();
        dlg->show();
        return;
    }

    close();
}

QWidget* ProgressBar::getMainWindow()
{
    foreach(QWidget *w, qApp->topLevelWidgets())
    {
        QWidget* mainWin = qobject_cast<QWidget*>(w);
        if (nullptr != mainWin && mainWin->objectName() == "MainWindowWidget")
            return mainWin;
    }
    return nullptr;
}