#include "gridlayout.h"
#include "ui_gridlayout.h"
#include <QStyleOption>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

Gridlayout::Gridlayout(QWidget *parent) :
    WidgetBase(parent),
    ui(new Ui::Gridlayout)
{
    ui->setupUi(this);
}

Gridlayout::~Gridlayout()
{
    delete ui;
}

void Gridlayout::setRowStretch()
{
//    ui->gridLayout->setRowStretch(0,0);
//    ui->gridLayout->setColumnStretch(0,0);
}

int Gridlayout::pushFront(QWidget *widget)
{
    if(nullptr == widget)
        return -1;

    widget->setFocusPolicy(Qt::NoFocus);
    list_wgt.push_front(widget);
    upDate();
    return 0;
}

int Gridlayout::pushBack(QWidget *widget)
{
    if(nullptr == widget)
        return -1;

    widget->setFocusPolicy(Qt::NoFocus);
    list_wgt.push_back(widget);
    upDate();
    return 0;
}

int Gridlayout::pushBackMore(QList<QWidget *> list)
{
    if(list.isEmpty())
        return -1;

    foreach(auto temp, list)
    {
        temp->setFocusPolicy(Qt::NoFocus);
        list_wgt.push_back(temp);
    }

    upDate();
    return 0;
}

QList<QWidget *> *Gridlayout::getCurWidgetList()
{
    return &list_wgt;
}

int Gridlayout::deleteWidget(QWidget *widget)
{
    if(nullptr == widget)
        return -1;

    QString strIndex = widget->property("currentIndex").toString();
    if("" == strIndex)
        return -1;

    return deleteWidget(strIndex.toInt());
}

void Gridlayout::clearAll()
{
    //移除所有的孩子
    while(ui->gridLayout->children().size())
    {
         ui->gridLayout->removeItem(ui->gridLayout->takeAt(0));
    }

    int count = list_wgt.size();
    for(int index = 0; index < count; index++)
    {
        QWidget* wgt = list_wgt.at(index);
        wgt->setParent(nullptr);
        wgt->deleteLater();
        wgt = nullptr;
    }
    list_wgt.clear();
}

void Gridlayout::removeHSpacer()
{
    ui->horizontalLayout->removeItem(ui->horizontalSpacer);
}

int Gridlayout::getSplitWidth()
{
    int width = (this->width() - (m_colCount-1)*20 - 40) / m_colCount;
    return width;
}

void Gridlayout::setContentMargin(int left, int top, int right, int bottom)
{
    ui->scrollAreaWidgetContents->layout()->setContentsMargins(left, top, right, bottom);
}

void Gridlayout::createPageLeft(int speed)
{
    if(nullptr == m_leftBtn)
    {
        //添加监视
        ui->scrollAreaWidgetContents->installEventFilter(this);
        this->installEventFilter(this);

        m_leftBtn = new QPushButton(this);
        m_leftBtn->setAttribute(Qt::WA_TranslucentBackground,true);//设置背景透明
        timerLeft = new QTimer(this);
        m_leftBtn->setStyleSheet("image: url(:/resources/image/mainwindow/setting/pageLeft.png);background-color: rgba(0,0,0,200)");
        m_leftBtn->installEventFilter(this);
        m_leftBtn->hide();

        //默认滑动速度
        if(0 == speed)
        {
            speed = 5 + getAllCount()*4;
        }

        connect(m_leftBtn,&QPushButton::clicked,this,[=](){
            QScrollBar* bar = ui->scrollArea->horizontalScrollBar();
            if(nullptr != bar)
            {
                int currentValue = bar->value();
                int stepValue = currentValue - 150;
                if(stepValue <= 0)
                {
                    bar->setValue(0);
                }
                else
                {
                    bar->setValue(stepValue);
                }
            }
        });
    }
}

void Gridlayout::createPageRight(int speed)
{
    if(nullptr == m_rightBtn)
    {
        //添加监视
        ui->scrollAreaWidgetContents->installEventFilter(this);
        this->installEventFilter(this);

        m_rightBtn = new QPushButton(this);
        m_rightBtn->setAttribute(Qt::WA_TranslucentBackground,true);//设置背景透明
        m_rightBtn->setStyleSheet("image: url(:/resources/image/mainwindow/setting/pageRight.png);background-color: rgba(0,0,0,200)");
        m_rightBtn->installEventFilter(this);
        timerRight = new QTimer(this);
        m_rightBtn->hide();

        //默认滑动速度
        if(0 == speed)
        {
            speed = 5 + getAllCount()*4;
        }

        connect(m_rightBtn,&QPushButton::clicked,this,[=](){
            QScrollBar* bar = ui->scrollArea->horizontalScrollBar();
            if(nullptr != bar)
            {
                int maxValue = bar->maximum();
                int currentValue = bar->value();
                int stepValue = currentValue + 150;
                if(stepValue >= maxValue)
                {
                    bar->setValue(maxValue);
                }
                else
                {
                    bar->setValue(stepValue);
                }
            }
        });
    }
}

QScrollBar* Gridlayout::getVScrollBar()
{
    return ui->scrollArea->verticalScrollBar();
}

QScrollArea *Gridlayout::getScrollArea()
{
    return  ui->scrollArea;
}

void Gridlayout::resizeEvent(QResizeEvent *event)
{
    upDate();
}

bool Gridlayout::eventFilter(QObject *watched, QEvent *event)
{
    if(this == watched || ui->scrollAreaWidgetContents == watched)
    {
        if (event->type() == QEvent::Show || event->type() == QEvent::Resize)
        {
            QScrollBar* bar = ui->scrollArea->horizontalScrollBar();
            if(nullptr != m_leftBtn && nullptr != bar
                    && (ui->scrollAreaWidgetContents->width() > ui->scrollArea->width() || ui->scrollAreaWidgetContents->height() > ui->scrollArea->height()))
            {
                m_leftBtn->setFixedSize(40,80);
                int height = (this->height()-m_leftBtn->height())/2;
                m_leftBtn->setGeometry(30,height,m_leftBtn->width(),m_leftBtn->height());
                m_leftBtn->raise();
                m_leftBtn->show();
            }
            else if(nullptr != m_leftBtn && !m_leftBtn->isHidden())
            {
                m_leftBtn->hide();
            }

            if(nullptr != m_rightBtn && nullptr != bar
                    && (ui->scrollAreaWidgetContents->width() > ui->scrollArea->width() || ui->scrollAreaWidgetContents->height() > ui->scrollArea->height()))
            {
                m_rightBtn->setFixedSize(40,80);
                int height = (this->height()-m_rightBtn->height())/2;
                m_rightBtn->setGeometry(this->width()-20-m_rightBtn->width(),height,m_rightBtn->width(),m_rightBtn->height());
                m_rightBtn->raise();
                m_rightBtn->show();
            }
            else if(nullptr != m_rightBtn && !m_rightBtn->isHidden())
            {
                m_rightBtn->hide();
            }
        }
    }
    else if(m_leftBtn == watched)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            timerLeft->start(100);
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            timerLeft->stop();
        }
    }
    else if(m_rightBtn == watched)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            timerRight->start(100);
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            timerRight->stop();
        }
    }

    return WidgetBase::eventFilter(watched,event);
}

void Gridlayout::init()
{

}

void Gridlayout::upDate()
{
    //移除所有的孩子
    while(ui->gridLayout->children().size())
    {
         ui->gridLayout->removeItem(ui->gridLayout->takeAt(0));
    }

    int count = list_wgt.size();
    for(int index = 0; index < count; index++)
    {
        QWidget* wgt = list_wgt.at(index);
        int wgt_width = wgt->width();
        int thiswidth = this->width();
        m_colCount = thiswidth/(wgt_width+20);
        if(!m_isChangeRow)
            m_colCount = 1000;
        wgt->setProperty("currentIndex",QString::number(index));
        if(0 == m_colCount)
        {
            ui->gridLayout->addWidget(wgt,index,0,1,1,Qt::AlignTop);
        }
        else
        {
            ui->gridLayout->addWidget(wgt,index/m_colCount,index%m_colCount,1,1,Qt::AlignTop);
        }
    }
}

int Gridlayout::deleteWidget(int index)
{
    int allCount = list_wgt.size();
    if(index < 0 || index >= allCount)
        return -1;

    //移除并释放内存
    QWidget* wgt = list_wgt.at(index);
    list_wgt.removeAt(index);
    ui->gridLayout->removeWidget(wgt);
    wgt->setParent(nullptr);
    wgt->deleteLater();
    wgt = nullptr;

    //刷新，重新布局
    upDate();
    return 0;
}

void Gridlayout::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

