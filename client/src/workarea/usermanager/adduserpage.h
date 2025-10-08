#ifndef ADDUSERPAGE_H
#define ADDUSERPAGE_H

#include <QWidget>
#include "mymask.h"
#include "restfulapi.h"
#include <QLabel>

namespace Ui {
class AddUserPage;
}

class AddUserPage : public QWidget
{
    Q_OBJECT

public:
    explicit AddUserPage(QWidget *parent = nullptr);
    ~AddUserPage();
    void editUser(QJsonObject objInfo);
    void delUser(QJsonObject objInfo);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void sig_optSuccess();

private slots:
    void slt_saveBtnClicked();

    void slt_requestFinishedSlot(QNetworkReply *networkReply);

    void slt_picSelectBtnClicked();

private:
    Ui::AddUserPage *ui;
    MyMask m_mask;
    QString m_recordId;
    RestFulApi m_restFulApi;

private:
    QWidget *getMainWindow();
    QString getPixmapBase64Data(QLabel *label, const char *format = "PNG");
};

#endif // ADDUSERPAGE_H
