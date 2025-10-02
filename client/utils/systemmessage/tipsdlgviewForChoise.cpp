#include "tipsdlgviewForChoise.h"
#include "ui_tipsdlgviewForChoise.h"
#include <thread>
#include <QStyle>
#include <QDesktopWidget>
#include "./globalmodule/globalparams.h"
#include "QEventLoop"
tipsdlgviewForChoise::tipsdlgviewForChoise(const QString &msg, QWidget *parent) :
    MyPopWidget(parent),
    ui(new Ui::tipsdlgviewForChoise)
{
    ui->setupUi(this);
    //为当前控件的所有子控件设置鼠标滑过的手势
    GlobalParams::getInstance()->setChildrenCursor(this);
    //setWindowOpacity(0.9);
    setAttribute(Qt::WA_DeleteOnClose);

}

tipsdlgviewForChoise::~tipsdlgviewForChoise()
{
    delete ui;
}

void tipsdlgviewForChoise::closeEvent(QCloseEvent *event)
{
    if(_loop.isRunning())
    {
        _loop.exit(1);
    }
    MyPopWidget::closeEvent(event);
}


int tipsdlgviewForChoise::windowExec(bool hasCancle)
{
    this->setAttribute(Qt::WA_ShowModal, true);
    MyPopWidget::show();
    return _loop.exec();
}

void tipsdlgviewForChoise::setButtonText(QString choiseA, QString choiseB)
{
    ui->ok_2->setText(choiseA);
    ui->ok->setText(choiseB);
}



void tipsdlgviewForChoise::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    //    if(GlobalParams::getInstance()->getMainWindow() == nullptr ||
    //            GlobalParams::getInstance()->getMainWindow()->isHidden())
    //        return;

    //    QWidget*  f_sender = GlobalParams::getInstance()->getMainWindow();
    //    auto point = f_sender->mapToGlobal(f_sender->rect().topLeft());

    //    unsigned int moveX = (f_sender->rect().width() - this->width())/2;
    //    unsigned int moveY = (f_sender->rect().height() - this->height())/2;
    //    this->setGeometry(point.x() + moveX,point.y() + moveY,this->width(),this->height());
}

void tipsdlgviewForChoise::setWindowGeometry(QWidget *widget,bool onRight)
{
    QWidget*  f_sender = widget;
    //弹框显示在右边
    if(onRight)
    {
        auto point = f_sender->mapToGlobal(f_sender->rect().bottomRight());
        this->setGeometry(point.x(),point.y(),this->width(),this->height());
    }
    //弹框显示在左边
    else
    {
        auto point = f_sender->mapToGlobal(f_sender->rect().bottomLeft());
        this->setGeometry(point.x()-this->width(),point.y(),this->width(),this->height());
    }
}


void tipsdlgviewForChoise::on_ok_clicked()
{
    _loop.exit(3);
    this->close();
}

void tipsdlgviewForChoise::on_ok_2_clicked()
{
    _loop.exit(4);
    this->close();
}
