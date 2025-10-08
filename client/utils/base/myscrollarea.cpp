#include "myscrollarea.h"
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QtMath>

MyScrollArea::MyScrollArea(QWidget *parent) : QScrollArea(parent)
{
    installEventFilter(this);
}

bool MyScrollArea::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == this)
    {
        //触屏可以拖动
        if (e->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            m_pressPoint = mouseEvent->pos();
            m_isPressed = true;
        }
        else if (e->type() == QEvent::MouseMove)
        {
           QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
           if(m_isPressed)
           {
               QPoint currentPos = mouseEvent->pos();
               float moveRadio_y = 1.0;

               if(fabs(currentPos.y()-m_pressPoint.y()) > 1e-7)
               {
                   moveRadio_y = float(currentPos.y()-m_pressPoint.y())/(this->widget()->height() - this->height());
               }

               if(moveRadio_y != 1.0 && nullptr != this->verticalScrollBar())
               {
                   int minimum = this->verticalScrollBar()->minimum();
                   int maximum = this->verticalScrollBar()->maximum();
                   int value = this->verticalScrollBar()->value() - (maximum-minimum)*moveRadio_y;
                   if(value > maximum)
                       this->verticalScrollBar()->setValue(maximum);
                   else if(value < minimum)
                       this->verticalScrollBar()->setValue(minimum);
                   else
                       this->verticalScrollBar()->setValue(value);
               }

               float moveRadio_x = 1.0;

               if(fabs(currentPos.x()-m_pressPoint.x()) > 1e-7)
               {
                   int width = this->widget()->width();
                   int width2 = this->width();
                   moveRadio_x = float(currentPos.x()-m_pressPoint.x())/(this->widget()->width() - this->width());
               }

               if(moveRadio_x != 1.0 && nullptr != this->horizontalScrollBar())
               {
                   int minimum = this->horizontalScrollBar()->minimum();
                   int maximum = this->horizontalScrollBar()->maximum();
                   int value = this->horizontalScrollBar()->value() - (maximum-minimum)*moveRadio_x;
                   if(value > maximum)
                       this->horizontalScrollBar()->setValue(maximum);
                   else if(value < minimum)
                       this->horizontalScrollBar()->setValue(minimum);
                   else
                       this->horizontalScrollBar()->setValue(value);
               }

               m_pressPoint = mouseEvent->pos();
           }
        }
        else if (e->type() == QEvent::MouseButtonRelease)
        {
           m_isPressed = false;
        }
    }
    return QWidget::eventFilter(obj,e);
}
