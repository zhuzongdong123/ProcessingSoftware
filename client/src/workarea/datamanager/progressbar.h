#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include "mymask.h"

namespace Ui {
class ProgressBar;
}

class ProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressBar(QWidget *parent = nullptr);
    ~ProgressBar();

    void startStep1();
    void startStep2();
    void startStep4();
    void setDownStep(double downStep);
    void setSaveStep(double saveStep);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void startTracking();
    void stopTracking();
    void updateTime();
    void slt_closeBtnClicked();

private:
    Ui::ProgressBar *ui;
    QElapsedTimer m_elapsedTimer;
    QTimer* m_timer = nullptr;
    double m_downStep = 0.0;
    double m_saveStep = 0.0;
    MyMask m_mask;

private:
    QWidget *getMainWindow();
};

#endif // PROGRESSBAR_H
