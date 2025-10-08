#include "usercard.h"
#include "ui_usercard.h"

UserCard::UserCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserCard)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->stackedWidget->setCurrentIndex(0);
    connect(ui->addBtn,&QPushButton::clicked,this,&UserCard::sig_addBtnClicked);
    connect(ui->editBtn,&QPushButton::clicked,this,[=](){
        emit sig_editBtnClicked(m_obj);
    });
    connect(ui->delBtn,&QPushButton::clicked,this,[=](){
        emit sig_delBtnClicked(m_obj);
    });
}

UserCard::~UserCard()
{
    delete ui;
}

void UserCard::showCardInfo(QJsonObject infoObj)
{
    m_obj = infoObj;
    ui->stackedWidget->setCurrentIndex(1);
    ui->userName->setText(infoObj.value("name").toString());
    ui->userAcc->setText(infoObj.value("acc").toString());
    ui->userTel->setText(infoObj.value("tel").toString());

    if(infoObj.value("acc").toString() == "admin")
    {
        ui->delBtn->hide();
    }

    QString strPic = infoObj.value("pic").toString();
    if(!strPic.isEmpty())
    {
        QByteArray decodedData = QByteArray::fromBase64(strPic.toLatin1());
        QPixmap pixMap;
        pixMap.loadFromData(decodedData);
        if(!pixMap.isNull())
        {
            ui->picLabel->setPixmap(pixMap);
        }
    }
}
