#include "datamanager.h"
#include "ui_datamanager.h"

DataManager::DataManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataManager)
{
    ui->setupUi(this);

    // 初始化表格
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels({"名称", "状态"});
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   // ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 设置表头样式
    ui->tableWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color:  #3F474F;"
        "    color: #FFFFFF;"
        "    border-bottom: 1px solid #3C4C5C;"
        "    padding: 5px;"
        "}"
    );

    // 启用样式渲染（必要时）
    ui->tableWidget->horizontalHeader()->setAttribute(Qt::WA_StyledBackground, true);
}

DataManager::~DataManager()
{
    delete ui;
}
