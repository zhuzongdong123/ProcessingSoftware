#ifndef MYMASK_H
#define MYMASK_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QMap>

class MyMask : public QWidget
{
public:
    MyMask(QWidget *parent = nullptr);

    /*!
     * \brief insertMask    指定控件上增加遮罩，并且可以设置遮罩上的显示文字
     * \param parent    指定控件类
     * \param text
     */
    void insertMask(QWidget* parent, QString styleSheet, float opacity = 1, QString text = "");

    void deleteMask(QWidget* parent,int timeMsec=10);

    void resetGeometry();

    /*!
     * \brief insertMaskAndLoading  增加遮罩图层和loading
     * \param parent
     * \param photoPath
     * \param styleSheet
     * \param opacity
     */
    void insertMaskAndLoading(QWidget *parent, QString styleSheet, float opacity = 0.5, QString photoPath = ":/image/mainwindow/加载中.gif");

    //事件过滤器
    bool eventFilter(QObject *target, QEvent *event);

private:
    QMap<QWidget*,QFrame*> m_maskMap;
};

#endif // MYMASK_H
