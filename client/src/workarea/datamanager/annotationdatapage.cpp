#include "annotationdatapage.h"
#include "ui_annotationdatapage.h"

AnnotationDataPage::AnnotationDataPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnnotationDataPage)
{
    ui->setupUi(this);
}

AnnotationDataPage::~AnnotationDataPage()
{
    delete ui;
}
