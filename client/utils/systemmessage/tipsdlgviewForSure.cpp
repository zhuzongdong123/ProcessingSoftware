#include "tipsdlgviewForSure.h"
#include "ui_tipsdlgviewForSure.h"
#include <thread>
#include <QStyle>
#include <QDesktopWidget>
#include "QEventLoop"

QWidget* getMainWindow()
{
    foreach(QWidget *w, qApp->topLevelWidgets())
    {
        QWidget* mainWin = qobject_cast<QWidget*>(w);
        if (nullptr != mainWin && mainWin->objectName() == "MainWindowWidget")
            return mainWin;
    }
    return nullptr;
}

tipsdlgviewForSure::tipsdlgviewForSure(const QString &msg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tipsdlgviewForSure)
{
    ui->setupUi(this);
    setParent(parent);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);//不显示标题栏，不显示状态栏图标
    //为当前控件的所有子控件设置鼠标滑过的手势
    ui->tiplabel->setWordWrap(true);
    //setWindowOpacity(0.9);
    ui->tiplabel->setText(msg);

    //无父对象则自动释放内存
    if(nullptr == parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
    }
}

tipsdlgviewForSure::~tipsdlgviewForSure()
{
    delete ui;
}

void tipsdlgviewForSure::closeEvent(QCloseEvent *event)
{
    if(_loop.isRunning())
    {
        _loop.exit(1);
    }
    QWidget::closeEvent(event);
}


int tipsdlgviewForSure::windowExec(bool hasCancle)
{
    //判断是否取消确认按钮
    if(!hasCancle)
    {
        ui->cancle->hide();
    }
    else
    {
        ui->cancle->show();
    }
    this->setAttribute(Qt::WA_ShowModal, true);
    QWidget::show();
    return _loop.exec();
}



void tipsdlgviewForSure::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    m_mask.insertMask(getMainWindow(),"background-color:rgb(0,0,0,100)",0.5);

    raise();
    show();
}

void tipsdlgviewForSure::setWindowGeometry(QWidget *widget)
{
    QWidget*  f_sender = widget;
    auto point = f_sender->mapToGlobal(f_sender->rect().topLeft());

    int moveX = (f_sender->rect().width() - this->width())/2;
    int moveY = (f_sender->rect().height() - this->height())/2;
    this->setGeometry(point.x() + moveX,point.y() + moveY,this->width(),this->height());
}


void tipsdlgviewForSure::on_cancle_clicked()
{
    m_mask.deleteMask(getMainWindow());
    _loop.exit(1);
    this->close();
}
void tipsdlgviewForSure::on_ok_clicked()
{
    m_mask.deleteMask(getMainWindow());
    emit sig_onClickOk();
     _loop.exit(0);
     this->close();
}
