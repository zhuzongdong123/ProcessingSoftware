/**
 * @file WidgetBase
 * @brief widget基类，提供基础能力
 * @author 朱宗冬
 * @date 2022-07-21
 */
#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include <QWidget>
#include <QJsonObject>

class WidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetBase(QWidget *parent = nullptr);

signals:

protected:
    /**
     * @brief paintEvent 解决QWidget提升后样式不可用问题
     */
    virtual void paintEvent(QPaintEvent *event);

};

#endif // WIDGETBASE_H
