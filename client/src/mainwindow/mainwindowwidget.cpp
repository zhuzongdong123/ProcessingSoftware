#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"

MainWindowWidget::MainWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);

    createConnect();
}

MainWindowWidget::~MainWindowWidget()
{
    delete ui;
}

void MainWindowWidget::showEvent(QShowEvent *event)
{
    emit sig_show();

    showDefaultPage();
}

void MainWindowWidget::init()
{

}

void MainWindowWidget::createConnect()
{
    connect(ui->titleBar,&TitleBar::sig_logout,this,&MainWindowWidget::sig_logout);
}

void MainWindowWidget::showDefaultPage()
{
    if(nullptr == m_dataManagerPage)
    {
        m_dataManagerPage = new DataManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_dataManagerPage);
    }

    if(nullptr == ui->stackedWidget->currentWidget())
    {
        ui->stackedWidget->setCurrentWidget(m_dataManagerPage);
        ui->menuBar->setMenuCheck(MenuBar::MenuType::DATA_MANAGER);
    }
}
