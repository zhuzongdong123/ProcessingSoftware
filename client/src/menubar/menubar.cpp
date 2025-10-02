#include "menubar.h"
#include "ui_menubar.h"
#include "appdatabasebase.h"

MenuBar::MenuBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuBar)
{
    ui->setupUi(this);
}

MenuBar::~MenuBar()
{
    delete ui;
}

void MenuBar::setMenuCheck(MenuBar::MenuType type)
{
    if(type == MenuType::DATA_MANAGER)
    {
        ui->dataManager->setChecked(true);
    }
    else if(type == MenuType::USER_MANAGER)
    {
        ui->userManager->setChecked(true);
    }
    else if(type == MenuType::SYS_MANAGER)
    {
        ui->sysManager->setChecked(true);
    }
}

void MenuBar::showEvent(QShowEvent *event)
{
    if(AppDatabaseBase::getInstance()->m_userType != "0")
    {
        ui->userManager->hide();
    }
    else
    {
        ui->userManager->show();
    }
}
