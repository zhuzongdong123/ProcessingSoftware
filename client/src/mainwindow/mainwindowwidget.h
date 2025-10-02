#ifndef MAINWINDOWWIDGET_H
#define MAINWINDOWWIDGET_H

#include <QWidget>

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

private:
    Ui::MainWindowWidget *ui;
};

#endif // MAINWINDOWWIDGET_H
