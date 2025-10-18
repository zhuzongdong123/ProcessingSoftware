#include "loginwidget.h" 
#include "ui_loginwidget.h" 
#include <QMessageBox> 
#include <QDebug> 
#include <QPropertyAnimation> 
#include <QFile> 
#include <QCryptographicHash> 
#include <QDesktopWidget>
#include "appdatabasebase.h"
#include "appconfigbase.h"
#include "tipsdlgviewForSure.h"
 
LoginWidget::LoginWidget(QWidget *parent) : 
    QWidget(parent), 
    ui(new Ui::LoginWidget), 
    m_isServerSettingsVisible(false)
{ 
    ui->setupUi(this); 

    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    
    // 初始化设置 
    m_settings = new QSettings("MyCompany", "MyApp");

    //绑定信号槽
    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &LoginWidget::slt_requestFinishedSlot);
    connect(ui->closeBtn,&QPushButton::clicked,this,&LoginWidget::close);
    connect(ui->passwordEdit,&QLineEdit::returnPressed,this,&LoginWidget::on_loginButton_clicked);
    
    // 设置窗口属性 
    setWindowTitle("登录"); 
    setFixedSize(800, 500); 
    
    // 加载保存的设置 
    loadSettings(); 
    
    // 设置初始状态 
    ui->serverIp->setVisible(false);

    // 在QWidget派生类的构造函数中添加
    QDesktopWidget* desktop = QApplication::desktop(); // 获取桌面部件
    // 计算居中坐标：(屏幕宽度-窗口宽度)/2，(屏幕高度-窗口高度)/2
    move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);

    //读取配置文件
    AppConfigBase::getInstance()->readConfig();
} 
 
LoginWidget::~LoginWidget() 
{ 
    delete ui; 

    if(nullptr != m_settings)
    {
        m_settings->deleteLater();
        m_settings = nullptr;
    }

    if(nullptr != m_mainWindow)
    {
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
    }
}

void LoginWidget::showEvent(QShowEvent *event)
{
    ui->loginButton->setText("登录");
    ui->loginButton->setEnabled(true);
    ui->usernameEdit->setFocus();
}

void LoginWidget::closeEvent(QCloseEvent *event)
{
    //先隐藏
    hide();

    //等待N秒再执行
}
 
void LoginWidget::on_loginButton_clicked() 
{ 
    ui->errorTip->clear();
    QString username = ui->usernameEdit->text(); 
    QString password = ui->passwordEdit->text(); 
    QString serverIP = ui->serverIp->text();
    QString port = AppConfigBase::getInstance()->readConfigSettings("port","port_business","8083");
    
    if (username.isEmpty()  || password.isEmpty())  { 
        ui->errorTip->setText("用户名和密码不能为空");
        return; 
    } 
    
    if (serverIP.isEmpty()  || port.isEmpty())  { 
        ui->errorTip->setText("服务器IP不能为空");
        return; 
    } 
    
    // 保存设置 
    saveSettings(); 
    
    // 这里添加实际登录逻辑 
    qDebug() << "用户名:" << username; 
    qDebug() << "密码:" << password; 
    qDebug() << "服务器IP:" << serverIP; 
    qDebug() << "端口:" << port; 
    

    QString requestUrl = QString("http://%1:%2").arg(serverIP).arg(port);
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("acc",username);
    post_data.insert("pwd",password);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);

    m_restFulApi.visitUrl(requestUrl + API_IDENTITY_AUTHENTICATION_QUERYPWD,VisitType::POST,ReplyType::LOGIN,"application/x-www-form-urlencoded",post_param,true,5000);
    ui->loginButton->setText("登录中");
    ui->loginButton->setEnabled(false);
} 
 
void LoginWidget::on_settingsButton_clicked() 
{ 
    toggleServerSettings(); 
} 
 
void LoginWidget::on_checkBox_rememberPassword_stateChanged(int arg1) 
{ 
    // 记住密码状态改变时自动保存设置 
    saveSettings(); 
} 
 
void LoginWidget::on_cancelButton_clicked() 
{ 
    close(); 
} 
 
void LoginWidget::saveSettings() 
{ 
    // 保存服务器设置 
    m_settings->setValue("Server/IP", ui->serverIp->text());
    m_settings->setValue("Server/Port", AppConfigBase::getInstance()->readConfigSettings("port","port_business","8083"));
    
    // 保存记住密码状态 
    //m_settings->setValue("Login/RememberPassword", ui->checkBox_rememberPassword->isChecked());
    
//    // 如果勾选了记住密码，则保存用户名和加密后的密码
//    if (ui->checkBox_rememberPassword->isChecked()) {
//        m_settings->setValue("Login/Username", ui->usernameEdit->text());
        
//        // 简单加密密码 (实际应用中应使用更安全的加密方式)
//        QByteArray passwordBytes = ui->passwordEdit->text().toUtf8();
//        QByteArray hashBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
//        m_settings->setValue("Login/PasswordHash", hashBytes.toHex());
//    } else {
//        // 不记住密码，清除保存的密码
//        m_settings->remove("Login/PasswordHash");
//    }
    
    m_settings->sync();
} 
 
void LoginWidget::loadSettings() 
{ 
    // 加载服务器设置 
    ui->serverIp->setText(m_settings->value("Server/IP", "127.0.0.1").toString());
    //ui->portEdit->setText(settings->value("Server/Port", "8080").toString());
    
    // 加载登录设置 
    bool rememberPassword = m_settings->value("Login/RememberPassword", false).toBool();
   // ui->checkBox_rememberPassword->setChecked(rememberPassword);
    
    // 如果记住密码，则加载用户名 
    if (rememberPassword) { 
        ui->usernameEdit->setText(m_settings->value("Login/Username").toString());
        // 这里只是演示，实际应用中应该从安全存储中获取密码 
        // 为了简单起见，这里不加载密码，实际应用中应该使用安全的方式存储和获取 
    } 
} 
 
void LoginWidget::toggleServerSettings() 
{ 
    m_isServerSettingsVisible = !m_isServerSettingsVisible;
    
    // 创建动画效果 
    QPropertyAnimation *animation = new QPropertyAnimation(ui->serverIp, "maximumHeight");
    animation->setDuration(300); 
    
    if (m_isServerSettingsVisible) {
        ui->serverIp->setVisible(true);
        animation->setStartValue(0); 
        animation->setEndValue(100); 
    } else { 
        animation->setStartValue(100); 
        animation->setEndValue(0); 
        connect(animation, &QPropertyAnimation::finished, [=]() { 
            ui->serverIp->setVisible(false);
        }); 
    } 
    
    animation->start(QAbstractAnimation::DeleteWhenStopped); 
} 

void LoginWidget::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::LOGIN)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                //保存用户的基础信息
                AppDatabaseBase::getInstance()->m_userName = obj.value("data").toObject().value("name").toString();
                AppDatabaseBase::getInstance()->m_userId = obj.value("data").toObject().value("id").toString();
                AppDatabaseBase::getInstance()->m_userType = obj.value("data").toObject().value("type").toString();
                AppDatabaseBase::getInstance()->m_serverIp = ui->serverIp->text();
                AppDatabaseBase::getInstance()->m_businessIp = ui->serverIp->text();
                AppDatabaseBase::getInstance()->m_businessPort = AppConfigBase::getInstance()->readConfigSettings("server","port_business","8083");
                AppDatabaseBase::getInstance()->m_bagPort = AppConfigBase::getInstance()->readConfigSettings("server","port_bag","8898");

                qDebug() << "登录成功";
                ui->errorTip->setText("登录成功");

                //跳转首页
                if(nullptr == m_mainWindow)
                {
                    m_mainWindow->deleteLater();
                    m_mainWindow = nullptr;
                }

                m_mainWindow = new MainWindowWidget();
                connect(m_mainWindow,&MainWindowWidget::sig_show,this,&LoginWidget::close);
                connect(m_mainWindow,&MainWindowWidget::sig_logout,this,&LoginWidget::slt_logout);
                m_mainWindow->showMaximized();
            }
            else
            {
                qDebug() << "登录失败";
                ui->loginButton->setText("登录");
                ui->loginButton->setEnabled(true);
                ui->errorTip->setText(obj.value("message").toString());
            }
        }
        else
        {
            qDebug() << "登录失败";
            ui->loginButton->setText("登录");
            ui->loginButton->setEnabled(true);
            ui->errorTip->setText("服务器连接失败");
        }
        networkReply->deleteLater();
    }
}

void LoginWidget::slt_logout()
{
    tipsdlgviewForSure box("是否退出登录？",nullptr);
    if(box.windowExec() == 1)
    {
        return;
    }

    if(nullptr != m_mainWindow)
    {
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
    }

    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    ui->errorTip->clear();
    this->show();
}
