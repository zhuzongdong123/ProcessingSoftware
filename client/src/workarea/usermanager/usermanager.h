#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QWidget>
#include "restfulapi.h"
#include <QJsonObject>

namespace Ui {
class UserManager;
}

class UserManager : public QWidget
{
    Q_OBJECT

public:
    explicit UserManager(QWidget *parent = nullptr);
    ~UserManager();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void slt_requestFinishedSlot(QNetworkReply *networkReply);
    void slt_displayAddPage();
    void slt_searchData();
    void slt_editBtnClicked(QJsonObject record);
    void slt_delBtnClicked(QJsonObject record);

private:
    Ui::UserManager *ui;
    RestFulApi m_restFulApi;

};

#endif // USERMANAGER_H
