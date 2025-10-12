#ifndef MAINWINDOWWIDGET_H
#define MAINWINDOWWIDGET_H

#include <QWidget>
#include "datamanager.h"
#include "annotationdatapage.h"
#include "usermanager.h"

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
    UserManager * m_userManager = nullptr;
    AnnotationDataPage* m_annotationDataPage = nullptr;

private:
    void init();
    void createConnect();
    bool createDBTable(QString tableName);

private slots:
    void sig_turn2BagDetialPage(QString id);
    void slt_menuClicked(QString menuName);
    void slt_showDefaultPage();
};

#endif // MAINWINDOWWIDGET_H
