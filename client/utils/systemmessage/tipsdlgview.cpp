#include "tipsdlgview.h"
#include "ui_tipsdlgview.h"
#include <thread>
#include <QStyle>
#include <QDesktopWidget>
#include <QScreen>

TipsDlgView::TipsDlgView(const QString &msg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TipsDlgView)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip);//不显示标题栏，不显示状态栏图标
    ui->tiplabel->setWordWrap(true);
    ui->tiplabel->setText(msg);

    QFontMetrics fm(ui->tiplabel->font());
    float textWidth = fm.boundingRect(msg).width();//求文字的像素宽度
    float radius_f = textWidth/275;
    if(radius_f > 1)
    {
        this->setMinimumHeight(38*(int(radius_f) + 1) + 50);
    }
    setAttribute(Qt::WA_DeleteOnClose);

    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(true);
    connect(m_pTimer, &QTimer::timeout, this, [=](){this->close();});
}

TipsDlgView::~TipsDlgView()
{
    delete ui;
}

void TipsDlgView::exec()
{
    show();
}

void TipsDlgView::startTimer(int time)
{
    this->m_pTimer->start(time);
}

QTimer *TipsDlgView::getTimer()
{
    return m_pTimer;
}

void TipsDlgView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    QTimer::singleShot(10, this, [this]() {
        QScreen *activeScreen = QGuiApplication::screenAt(QCursor::pos());
        QRect screenGeometry = activeScreen->geometry();
        this->move(screenGeometry.center()  - this->rect().center());
    });
}

void TipsDlgView::setWindowGeometry(QWidget *widget)
{
    QWidget*  f_sender = widget;
    auto point = f_sender->mapToGlobal(f_sender->rect().topLeft());

    int moveX = (f_sender->rect().width() - this->width())/2;
    int moveY = (f_sender->rect().height() - this->height())/2;
    this->setGeometry(point.x() + moveX,point.y() + moveY,this->width(),this->height());
}

QTimer *TipsDlgView::getPTimer() const
{
    return m_pTimer;
}

void TipsDlgView::show(bool right)
{
    if(right)
    {

    }
    else
    {
        ui->Tipicon->setStyleSheet("border-image: url(:/image/mainwindow/错误提示.png);");
    }
    QWidget::show();
}

void TipsDlgView::on_pushButton_clicked()
{
    this->close();
}

void TipsDlgView::setShowCenter(bool value)
{
    m_showCenter = value;
}
