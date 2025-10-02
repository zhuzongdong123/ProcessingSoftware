#ifndef TIPSDLGVIEW_H
#define TIPSDLGVIEW_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class TipsDlgView;
}

class TipsDlgView : public QWidget
{
    Q_OBJECT

public:
    explicit TipsDlgView(const QString &msg,QWidget *parent = nullptr);
    ~TipsDlgView();

    void exec();

    /**
        Description:
            开启定时器
        @param	无
        @return	无
    */
    void startTimer(int time = 2000);

    QTimer* getTimer();

    void showEvent(QShowEvent *event);
    void setWindowGeometry(QWidget *widget);
    QTimer *getPTimer() const;
    void show(bool right=false);

    void setShowCenter(bool value);
private:
    Ui::TipsDlgView *ui;
    QTimer *m_pTimer;
    bool m_showCenter = false;

public slots:
    //关闭窗口
    void on_pushButton_clicked();

};

#endif // TIPSDLGVIEW_H
