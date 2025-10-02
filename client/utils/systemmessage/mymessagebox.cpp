#include "mymessagebox.h"
#include "ui_mymessagebox.h"
#include "./globalmodule/globalparams.h"

MyMessageBox::MyMessageBox(QString header, QString tip, QString okText, QString cancelText,QWidget *parent):
    QDialog(parent),
    ui(new Ui::MyMessageBox)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip);
    //为当前控件的所有子控件设置鼠标滑过的手势
    GlobalParams::getInstance()->setChildrenCursor(this);

    setWindowTitle(header);
    ui->TipTitle->setText(header);
    ui->tiplabel->setText(tip);
    ui->okButton->setText(okText);
    ui->cancleButton->setText(cancelText);

    if("" == okText)
        ui->okButton->hide();

    if("" == cancelText)
        ui->cancleButton->hide();

    connect(ui->okButton,&QPushButton::clicked,[=]{QDialog::accept();});
    connect(ui->cancleButton,&QPushButton::clicked,[=]{QDialog::reject();});
}

MyMessageBox::~MyMessageBox()
{
    delete ui;
}

void MyMessageBox::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if(GlobalParams::getInstance()->getMainWindow() == nullptr ||
            GlobalParams::getInstance()->getMainWindow()->isHidden())
        return;

    QWidget*  f_sender = GlobalParams::getInstance()->getMainWindow();
    auto point = f_sender->mapToGlobal(f_sender->rect().topLeft());

    unsigned int moveX = (f_sender->rect().width() - this->width())/2;
    unsigned int moveY = (f_sender->rect().height() - this->height())/2;
    this->setGeometry(point.x() + moveX,point.y() + moveY,this->width(),this->height());
}
