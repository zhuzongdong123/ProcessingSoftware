#include "usermanager.h"
#include "ui_usermanager.h"
#include "appdatabasebase.h"
#include "tipsdlgview.h"
#include "usercard.h"
#include "adduserpage.h"

UserManager::UserManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserManager)
{
    ui->setupUi(this);

    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &UserManager::slt_requestFinishedSlot);
    connect(ui->serchBtn, &QPushButton::clicked, this, &UserManager::slt_searchData);
    connect(ui->resetBtn, &QPushButton::clicked, ui->nameSearchEdit, &QLineEdit::clear);
    connect(ui->resetBtn, &QPushButton::clicked, this, &UserManager::slt_searchData);
}

UserManager::~UserManager()
{
    delete ui;
}

void UserManager::showEvent(QShowEvent *event)
{
    ui->nameSearchEdit->clear();
    slt_searchData();
}

void UserManager::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::IDENTITY_AUTHENTICATION_PAGELIST)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            //先添加第一个新增按钮
            UserCard* addBtn = new UserCard();
            connect(addBtn,&UserCard::sig_addBtnClicked,this,&UserManager::slt_displayAddPage);
            ui->gridlayout->pushBack(addBtn);

            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QJsonArray array = obj.value("data").toArray();
                for(auto dataObj : array)
                {
                    UserCard* addBtn = new UserCard();
                    addBtn->showCardInfo(dataObj.toObject());
                    //connect(addBtn,&UserCard::sig_addBtnClicked,this,&UserManager::slt_displayAddPage);
                    connect(addBtn,&UserCard::sig_editBtnClicked,this,&UserManager::slt_editBtnClicked);
                    connect(addBtn,&UserCard::sig_delBtnClicked,this,&UserManager::slt_delBtnClicked);
                    ui->gridlayout->pushBack(addBtn);
                }
            }
        }
        else
        {
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }
    }
}

void UserManager::slt_displayAddPage()
{
    AddUserPage* addPage = new AddUserPage();
    connect(addPage,&AddUserPage::sig_optSuccess,this,&UserManager::slt_searchData);
    addPage->show();
}

void UserManager::slt_searchData()
{
    //检索所有的用户
    ui->gridlayout->clearAll();
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();

    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    QJsonObject objParams;
    if(!ui->nameSearchEdit->text().isEmpty())
        objParams.insert("name_like",ui->nameSearchEdit->text());
    post_data.insert("params",objParams);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    m_restFulApi.visitUrl(requestUrl + API_IDENTITY_AUTHENTICATION_PAGELIST,
                          VisitType::POST,ReplyType::IDENTITY_AUTHENTICATION_PAGELIST,"application/json",post_param,true,5000);
}

void UserManager::slt_editBtnClicked(QJsonObject record)
{
    AddUserPage* addPage = new AddUserPage();
    connect(addPage,&AddUserPage::sig_optSuccess,this,&UserManager::slt_searchData);
    addPage->editUser(record);
    addPage->show();
}

void UserManager::slt_delBtnClicked(QJsonObject record)
{
    AddUserPage* addPage = new AddUserPage();
    connect(addPage,&AddUserPage::sig_optSuccess,this,&UserManager::slt_searchData);
    addPage->delUser(record);
}