#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"
#include "annotationdatapage.h"
#include "mysqlite.h"

MainWindowWidget::MainWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    createConnect();

    //创建数据库
    MySqlite::getInstance()->createDB(QApplication::applicationDirPath() + "/" + QApplication::applicationName());
    createDBTable("annotation_record");
}

MainWindowWidget::~MainWindowWidget()
{
    delete ui;
}

void MainWindowWidget::showEvent(QShowEvent *event)
{
    emit sig_show();
    slt_showDefaultPage();
}

void MainWindowWidget::init()
{

}

void MainWindowWidget::createConnect()
{
    connect(ui->titleBar,&TitleBar::sig_logout,this,&MainWindowWidget::sig_logout);
    connect(ui->menuBar,&MenuBar::sig_menuClicked,this,&MainWindowWidget::slt_menuClicked);
}

void MainWindowWidget::slt_showDefaultPage()
{
    if(nullptr == m_dataManagerPage)
    {
        m_dataManagerPage = new DataManager(ui->stackedWidget);
        connect(m_dataManagerPage,&DataManager::sig_turn2BagDetialPage,this,&MainWindowWidget::sig_turn2BagDetialPage);
        ui->stackedWidget->addWidget(m_dataManagerPage);
    }

    ui->stackedWidget->setCurrentWidget(m_dataManagerPage);
    ui->menuBar->setMenuCheck(MenuBar::MenuType::DATA_MANAGER);
}

void MainWindowWidget::sig_turn2BagDetialPage(QString id)
{
    if(nullptr == m_annotationDataPage)
    {
        m_annotationDataPage = new AnnotationDataPage(ui->stackedWidget);
        connect(m_annotationDataPage,&AnnotationDataPage::sig_return,this,&MainWindowWidget::slt_showDefaultPage);
        ui->stackedWidget->addWidget(m_annotationDataPage);
    }
    ui->stackedWidget->setCurrentWidget(m_annotationDataPage);
    ui->menuBar->hide();
    QApplication::processEvents();
    m_annotationDataPage->setBagId(id);
    QApplication::processEvents();
}

void MainWindowWidget::slt_menuClicked(QString menuName)
{
    if("数据管理" == menuName)
    {
        slt_showDefaultPage();
    }
    else if("用户管理" == menuName)
    {
        if(nullptr == m_userManager)
        {
            m_userManager = new UserManager(ui->stackedWidget);
            ui->stackedWidget->addWidget(m_userManager);
        }

        ui->stackedWidget->setCurrentWidget(m_userManager);
        ui->menuBar->setMenuCheck(MenuBar::MenuType::USER_MANAGER);
    }
    else if("系统管理" == menuName)
    {
//        if(nullptr == m_dataManagerPage)
//        {
//            m_userManager = new UserManager(ui->stackedWidget);
//            ui->stackedWidget->addWidget(m_userManager);
//        }

//        ui->stackedWidget->setCurrentWidget(m_userManager);
//        ui->menuBar->setMenuCheck(MenuBar::MenuType::USER_MANAGER);
    }
}

bool MainWindowWidget::createDBTable(QString tableName)
{
    //创建表
    QStringList sNameList;
    QStringList sType;

    //标绘表
    if(tableName == "draw_record")
    {
        sNameList << "id" << "bag_id" << "image_id" << "data" << "opt_sentry_id" << "opt_time";
        sType << "varchar(36)" << "varchar(36)" << "varchar(36)" << "text" << "varchar(36)" << "varchar(20)";
    }
    //标注记录表
    else if(tableName == "annotation_record")
    {
        sNameList << "id" << "bag_id" << "opt_sentry_id" << "status" << "opt_time";
        sType << "varchar(36)" << "varchar(36)" << "varchar(36)" << "varchar(1)" << "varchar(20)";
    }
    else
        return false;

    MySqlite::getInstance()->initTable(tableName,sNameList,sType);
}
