/**
 * @file gridlayout.h
 * @brief
 * 动态布局用，主要用于卡片列表显示
 * @author 朱宗冬
 * @date 2023-04-03
 */
#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include <QWidget>
#include <QList>
#include <QTimer>
#include <QPushButton>
#include "widgetbase.h"
#include <QScrollBar>
#include <QScrollArea>

namespace Ui {
class Gridlayout;
}

class Gridlayout : public WidgetBase
{
    Q_OBJECT

public:
    explicit Gridlayout(QWidget *parent = nullptr);
    ~Gridlayout();

    void setRowStretch();
    //前插
    int pushFront(QWidget* widget);

    //前插
    int pushBack(QWidget* widget);

    //前插多个widget add by zxt
    int pushBackMore(QList<QWidget*> list);

    //返回当前widget列表 add by zxt
    QList<QWidget*> *getCurWidgetList();

    //删除指定的控件
    int deleteWidget(QWidget* widget);

    //清空全部
    void clearAll();

    //移除横向的弹簧
    void removeHSpacer();

    //(总宽度-间隙)/列
    int getSplitWidth();

    //总个数
    int getAllCount(){return list_wgt.size();}

    //是否换行
    void setChangeRow(bool value){m_isChangeRow = value;}

    //设置间距
    void setContentMargin(int left, int top, int right, int bottom);

    //创建向左翻页
    void createPageLeft(int speed = 0);

    //创建向右翻页
    void createPageRight(int speed = 0);

    //获取纵向滚动条
    QScrollBar* getVScrollBar();

    //获取纵向滚动条
    QScrollArea* getScrollArea();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    Ui::Gridlayout *ui;
    QList<QWidget*> list_wgt;
    int m_colCount = 8;
    bool m_isChangeRow = true;
    QPushButton* m_leftBtn = nullptr;
    QTimer* timerLeft = nullptr;
    QPushButton* m_rightBtn = nullptr;
    QTimer* timerRight = nullptr;

private:
    //初始化
    void init();

    //刷新页面，重新布局
    void upDate();

    //删除指定的控件
    int deleteWidget(int index);
};

#endif // GRIDLAYOUT_H
