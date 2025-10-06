#ifndef MAINWINDOWWIDGET_H
#define MAINWINDOWWIDGET_H

#include <QWidget>
#include "datamanager.h"
#include "annotationdatapage.h"

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
    AnnotationDataPage* m_annotationDataPage = nullptr;

private:
    void init();
    void createConnect();
    void showDefaultPage();

private slots:
    void sig_turn2BagDetialPage(QString id);
};

#endif // MAINWINDOWWIDGET_H
