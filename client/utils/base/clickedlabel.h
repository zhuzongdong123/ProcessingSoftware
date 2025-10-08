#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H

#include <QObject>
#include "QMouseEvent"
#include "qlabel.h"
#include "QJsonObject"
//带有点击事件的标签：
class ClickedLabel : public QLabel
{
    Q_OBJECT
    public:
        explicit ClickedLabel(QWidget *parent = 0,Qt::WindowFlags f=Qt::WindowFlags());
        ~ClickedLabel();
    //设置跳转用参数
    void setJumpJsonObj(QJsonObject jumpJsonObj);

    void setToolTip(const QString value){m_toolTip = value;}

signals:
    void clicked();

protected:
    QJsonObject _jumpJsonObj;
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    QString m_toolTip;
};

#endif // CLICKEDLABEL_H
