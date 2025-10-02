#ifndef TIPSDLGVIEW3_H
#define TIPSDLGVIEW3_H

#include <QDialog>
#include <QTimer>
#include "./loginmodule/mainwindow/baseclass/mypopwidget.h"
#include "qeventloop.h"
namespace Ui {
class tipsdlgviewForChoise;
}

class tipsdlgviewForChoise : public MyPopWidget
{
    Q_OBJECT

public:
    explicit tipsdlgviewForChoise(const QString &msg,QWidget *parent = nullptr);
    ~tipsdlgviewForChoise();
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event);
    void setWindowGeometry(QWidget *widget,bool onRight=true);
    QTimer *getPTimer() const;
    //参数：是否含取消按钮
    int windowExec(bool hasCancle=true);
    //设置按钮文本
    void setButtonText(QString choiseA,QString choiseB);
private:
    Ui::tipsdlgviewForChoise *ui;
    QEventLoop _loop;
public slots:
    //确认2
    void on_ok_clicked();
    //确认1
    void on_ok_2_clicked();
};

#endif // TIPSDLGVIEW2_H
