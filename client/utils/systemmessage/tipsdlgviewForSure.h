#ifndef TIPSDLGVIEW2_H
#define TIPSDLGVIEW2_H

#include <QDialog>
#include <QTimer>
#include "qeventloop.h"
#include "mymask.h"
namespace Ui {
class tipsdlgviewForSure;
}

class tipsdlgviewForSure : public QWidget
{
    Q_OBJECT

public:
    explicit tipsdlgviewForSure(const QString &msg,QWidget *parent = nullptr);
    ~tipsdlgviewForSure();
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event);
    void setWindowGeometry(QWidget *widget);
    QTimer *getPTimer() const;
    //参数：是否含取消按钮
    int windowExec(bool hasCancle=true);

signals:
    void sig_onClickOk();

private:
    Ui::tipsdlgviewForSure *ui;
    QEventLoop _loop;
    MyMask m_mask;
public slots:
    //关闭窗口
    void on_cancle_clicked();
    //确认
    void on_ok_clicked();
};

#endif // TIPSDLGVIEW2_H
