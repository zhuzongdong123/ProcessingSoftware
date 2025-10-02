#ifndef LOGINWIDGET_H 
#define LOGINWIDGET_H 
 
#include <QWidget> 
#include <QSettings> 
#include "hrhttpclient.h"
#include "mainwindowwidget.h"
#include "restfulapi.h"
 
namespace Ui { 
class LoginWidget; 
} 
 
class LoginWidget : public QWidget 
{ 
    Q_OBJECT 
 
public: 
    explicit LoginWidget(QWidget *parent = nullptr); 
    ~LoginWidget(); 

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
 
private slots: 
    void on_loginButton_clicked(); 
    void on_settingsButton_clicked(); 
    void on_checkBox_rememberPassword_stateChanged(int arg1); 
    void on_cancelButton_clicked(); 
    void slt_requestFinishedSlot(QNetworkReply *networkReply);
    void slt_logout();

private:
    Ui::LoginWidget *ui; 
    QSettings *m_settings = nullptr;
    bool m_isServerSettingsVisible = true;
    //hrHttpClient m_httpClient;
    MainWindowWidget* m_mainWindow = nullptr;
    RestFulApi m_restFulApi;//登录请求

private:
    void saveSettings(); 
    void loadSettings(); 
    void toggleServerSettings();

};
 
#endif // LOGINWIDGET_H 