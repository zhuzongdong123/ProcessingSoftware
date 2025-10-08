#include "clickedlabel.h"
#include <QMovie>
#include <QPainter>
#include <QToolTip>

ClickedLabel::ClickedLabel(QWidget *parent, Qt::WindowFlags f):QLabel(parent,f)
{
    this->installEventFilter(this);
}

ClickedLabel::~ClickedLabel()
{

}

void ClickedLabel::setJumpJsonObj(const QJsonObject jumpJsonObj)
{
    _jumpJsonObj = jumpJsonObj;
}

void ClickedLabel::mousePressEvent(QMouseEvent *ev)
{
    //add by xintong-zhou
    if(ev->type() == QMouseEvent::MouseButtonPress)
    {
        emit clicked();
    }
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *ev)
{
}

void ClickedLabel::mouseMoveEvent(QMouseEvent *ev)
{
    //改变鼠标为点击样式：
    setCursor(Qt::PointingHandCursor);
}

void ClickedLabel::paintEvent(QPaintEvent *ev)
{
    if (this->movie() && this->movie()->isValid())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        // 下面这行不需要设置混合模式，测试代码时估计忘了删
        // painter.setCompositionMode(QPainter::CompositionMode_Source);
        QPixmap curr_pix = this->movie()->currentPixmap();
        if (this->hasScaledContents())
        {
            // 如果是要考虑高分屏，缩放size需要乘以this.devicePixelRatio
            // 并对pix设置同样的devicePixelRatio
            QPixmap pix = curr_pix.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(QPoint(0, 0), pix);
        }
        else
        {
            painter.drawPixmap(QPoint(0, 0), curr_pix);
        }
    }
    else
        QLabel::paintEvent(ev);
}

bool ClickedLabel::eventFilter(QObject *watched, QEvent *event)
{
    if(this == watched)
    {
        if (event->type() == QEvent::Enter || event->type() == QEvent::MouseButtonPress)
        {
            if("" != m_toolTip)
                QToolTip::showText(QCursor::pos(),m_toolTip,this);
        }
    }

    return QLabel::eventFilter(watched,event);
}
