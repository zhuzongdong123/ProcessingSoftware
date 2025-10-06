#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void sig_logout();//注销
    void sig_close();//退出

private slots:
    void slt_close();

private:
    Ui::TitleBar *ui;
    bool m_isCreateAnimation = false;
    QGraphicsOpacityEffect *m_effect = nullptr;
    QPropertyAnimation *m_anim = nullptr;
};

#endif // TITLEBAR_H
