#ifndef MAINWINDOWWIDGET_H
#define MAINWINDOWWIDGET_H

#include <QWidget>
#include "datamanager.h"

namespace Ui {
class MainWindowWidget;
}

class MainWindowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindowWidget(QWidget *parent = nullptr);
    ~MainWindowWidget();

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void sig_show();
    void sig_logout();

private:
    Ui::MainWindowWidget *ui;
    DataManager *m_dataManagerPage = nullptr;

private:
    void init();
    void createConnect();
    void showDefaultPage();
};

#endif // MAINWINDOWWIDGET_H
