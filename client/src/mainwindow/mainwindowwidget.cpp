#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"

MainWindowWidget::MainWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);
}

MainWindowWidget::~MainWindowWidget()
{
    delete ui;
}

void MainWindowWidget::showEvent(QShowEvent *event)
{
    emit sig_show();
}
