#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"
#include "annotationdatapage.h"

MainWindowWidget::MainWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

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
        connect(m_dataManagerPage,&DataManager::sig_turn2BagDetialPage,this,&MainWindowWidget::sig_turn2BagDetialPage);
        ui->stackedWidget->addWidget(m_dataManagerPage);
    }

    if(nullptr == ui->stackedWidget->currentWidget())
    {
        ui->stackedWidget->setCurrentWidget(m_dataManagerPage);
        ui->menuBar->setMenuCheck(MenuBar::MenuType::DATA_MANAGER);
    }
}

void MainWindowWidget::sig_turn2BagDetialPage(QString id)
{
    if(nullptr == m_annotationDataPage)
    {
        m_annotationDataPage = new AnnotationDataPage(ui->stackedWidget);
        m_annotationDataPage->setBagId(id);
        ui->stackedWidget->addWidget(m_annotationDataPage);
    }
    ui->stackedWidget->setCurrentWidget(m_annotationDataPage);
    ui->menuBar->hide();
}
