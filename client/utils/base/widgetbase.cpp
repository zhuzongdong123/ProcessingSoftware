#include "widgetbase.h"
#include <QPainter>
#include <QStyleOption>
#include <QStyle>

WidgetBase::WidgetBase(QWidget *parent) : QWidget(parent)
{

}

void WidgetBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}


