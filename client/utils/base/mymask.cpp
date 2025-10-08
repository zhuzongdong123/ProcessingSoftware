#include "mymask.h"
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>
#include <qtimer.h>
MyMask::MyMask(QWidget *parent) : QWidget(parent)
{

}

void MyMask::insertMask(QWidget *parent, QString styleSheet, float opacity, QString text)
{
    if(parent == nullptr || m_maskMap.find(parent) != m_maskMap.end())
        return;

    //对其父类安装事件过滤器,监听其resize事件：
    parent->installEventFilter(this);

    QFrame* mask = new QFrame(parent);
    mask->setStyleSheet(styleSheet);
    mask->setWindowFlags(Qt::FramelessWindowHint);
    mask->setWindowOpacity(opacity);
    mask->setGeometry(0,0,parent->width(),parent->height());
    mask->show();

    if(text != "")
    {
        QHBoxLayout* layout = new QHBoxLayout(mask);
        QLabel* labelTitle = new QLabel(mask);
        labelTitle->setStyleSheet("font-size:40px");
        labelTitle->setAlignment(Qt::AlignCenter);
        labelTitle->setText(text);
        layout->addWidget(labelTitle);
        mask->setLayout(layout);
    }

    m_maskMap.insert(parent,mask);
    qDebug() << "增加遮罩: " << parent->objectName();

}

void MyMask::insertMaskAndLoading(QWidget *parent, QString styleSheet, float opacity, QString photoPath)
{
    if(parent == nullptr || m_maskMap.find(parent) != m_maskMap.end())
        return;
    //对其父类安装事件过滤器,监听其resize事件：
    parent->installEventFilter(this);
    QFrame* mask;
    if(parent->parentWidget()!=nullptr)
    {
        mask = new QFrame(parent->parentWidget());
        mask->setStyleSheet("border:none;background-color:transparent;");
    //parent->parentWidget()->setStyleSheet("border: 3px solid yellow;");
   //    mask->setWindowFlags(Qt::FramelessWindowHint);
   //    mask->setWindowOpacity(opacity);
     mask->setAttribute(Qt::WA_TranslucentBackground, true);
     mask->setWindowFlags(Qt::FramelessWindowHint);//Qt::Tool|
     auto point=parent->mapToGlobal(QPoint(0,0));
     point=parent->parentWidget()->mapFromGlobal(point);
     mask->setGeometry(point.x(),point.y(),parent->width(),parent->height());
     mask->show();
    }
    else
    {
        mask = new QFrame(parent);
        mask->setStyleSheet("border:none;background-color:transparent;");
        //parent->parentWidget()->setStyleSheet("border: 3px solid yellow;");
       mask->setWindowFlags(Qt::FramelessWindowHint);
       mask->setWindowOpacity(opacity);
         mask->setGeometry(0,0,parent->width(),parent->height());
         mask->show();
    }
    //loading框
    QHBoxLayout* layout = new QHBoxLayout(mask);
    QLabel* labelTitle = new QLabel(mask);
    labelTitle->setAlignment(Qt::AlignCenter);
    labelTitle->setPixmap(QPixmap(""));
    QMovie* movie = new QMovie(photoPath,NULL,mask);
    labelTitle->setMovie(movie);
    movie->start();
    layout->addWidget(labelTitle);
    layout->setMargin(0);
    mask->setLayout(layout);

    m_maskMap.insert(parent,mask);
    qDebug() << "增加遮罩: " << parent->objectName();
}

bool MyMask::eventFilter(QObject *target, QEvent *event)
{
    //监听其父类的状态；
    if(event->type()==QEvent::Resize)
    {
        resetGeometry();
    }
    return false;
}

void MyMask::deleteMask(QWidget *parent,int timeMsec)
{
    if (m_maskMap.find(parent) != m_maskMap.end())
    {
        m_maskMap.find(parent).value()->hide();
        m_maskMap.find(parent).value()->deleteLater();
        m_maskMap.erase(m_maskMap.find(parent));
        //对其父类卸载事件过滤器
        parent->removeEventFilter(this);

        qDebug() << "删除遮罩: " << parent->objectName();
    }
}

void MyMask::resetGeometry()
{
    if(m_maskMap.size() == 0)
        return;

    QMap<QWidget*,QFrame*>::iterator maskMapIte;
    maskMapIte = m_maskMap.begin();
    for(; maskMapIte != m_maskMap.end(); maskMapIte++)
    {
        maskMapIte.value()->setGeometry(0,0,maskMapIte.key()->width(),maskMapIte.key()->height());
    }

    qDebug() << "重设位置!!!!!!!";
}
