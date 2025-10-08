#ifndef MYSCROLLAREA_H
#define MYSCROLLAREA_H

#include <QScrollArea>

class MyScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit MyScrollArea(QWidget *parent = nullptr);

signals:

public:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    QPoint m_pressPoint;
    bool m_isPressed = false;
};

#endif // MYSCROLLAREA_H
