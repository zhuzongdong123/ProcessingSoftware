#include "adduserpage.h"
#include "ui_adduserpage.h"
#include <QRegExpValidator>
#include <QAction>
#include "tipsdlgview.h"
#include "appdatabasebase.h"
#include <QFileDialog>
#include <QBuffer>
#include <QScreen>

AddUserPage::AddUserPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddUserPage)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    //账号相关
    {
        // 创建正则表达式验证器
        QRegExp rx("[a-zA-Z0-9]+");  // 只允许字母和数字
        QRegExpValidator *validator = new QRegExpValidator(rx, this);
        // 应用到 QLineEdit
        ui->accEdit->setValidator(validator);
    }


    //密码相关
    {
        QAction *toggleAction = ui->pwdEdit->addAction(
            QPixmap(":/resources/image/showPwd.png").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation),  QLineEdit::TrailingPosition
        );
        connect(toggleAction, &QAction::triggered, [=]() {
            ui->pwdEdit->setEchoMode(
                ui->pwdEdit->echoMode() == QLineEdit::Password
                ? QLineEdit::Normal
                : QLineEdit::Password
            );
        });

        // 禁用自动完成
        ui->pwdEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
        ui->pwdEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        ui->pwdEdit->setAutoFillBackground(false);

        // 防止密码泄露
        ui->pwdEdit->setContextMenuPolicy(Qt::NoContextMenu);  // 禁用右键菜单
        ui->pwdEdit->setStyleSheet(
            "QLineEdit[echoMode='2'] { lineedit-password-character: 9679; }"  // 使用圆点替代星号
        );

        // 允许：字母、数字和常用特殊符号 !@#$%^&*()_+-=
        QRegularExpressionValidator *validator = new QRegularExpressionValidator(
            QRegularExpression("^[a-zA-Z0-9!@#$%^&*()_+\\-=]+$"),
            ui->pwdEdit
        );
        ui->pwdEdit->setValidator(validator);
        ui->pwdEdit->setEchoMode(QLineEdit::Password);  // 密文显示
        ui->pwdEdit->setMaxLength(32);  // 最大长度限制
    }

    //关闭按钮
    connect(ui->closeBtn,&QPushButton::clicked,this,&AddUserPage::close);
    connect(ui->saveBtn,&QPushButton::clicked,this,&AddUserPage::slt_saveBtnClicked);
    connect(ui->picLabel,&ClickedLabel::clicked,this,&AddUserPage::slt_picSelectBtnClicked);
    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &AddUserPage::slt_requestFinishedSlot);
    setAttribute(Qt::WA_DeleteOnClose);
}

AddUserPage::~AddUserPage()
{
    delete ui;
}

void AddUserPage::editUser(QJsonObject objInfo)
{
    ui->nameEdit->setText(objInfo.value("name").toString());
    ui->accEdit->setText(objInfo.value("acc").toString());
    ui->pwdEdit->setText(objInfo.value("pwd").toString());
    ui->telEdit->setText(objInfo.value("tel").toString());
    m_recordId = objInfo.value("id").toString();
    QString strPic = objInfo.value("pic").toString();
    if(!strPic.isEmpty())
    {
        QByteArray decodedData = QByteArray::fromBase64(strPic.toLatin1());
        QPixmap pixMap;
        pixMap.loadFromData(decodedData);
        if(!pixMap.isNull())
        {
            ui->picLabel->setProperty("hasPic",true);
            ui->picLabel->setPixmap(pixMap);
        }
    }

    if(objInfo.value("acc").toString() == "admin")
    {
        ui->accEdit->setReadOnly(true);
    }
}

void AddUserPage::delUser(QJsonObject objInfo)
{
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("id",objInfo.value("id").toString());
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    m_restFulApi.visitUrl(requestUrl + API_IDENTITY_AUTHENTICATION_DEL,
                          VisitType::POST,ReplyType::IDENTITY_AUTHENTICATION_DEL,"application/json",post_param,true,5000);
}

void AddUserPage::showEvent(QShowEvent *event)
{
    m_mask.insertMask(getMainWindow(),"background-color:rgb(0,0,0,100)",0.5);

    raise();
    show();

    QTimer::singleShot(10, this, [this]() {
        QScreen *activeScreen = QGuiApplication::screenAt(QCursor::pos());
        QRect screenGeometry = activeScreen->geometry();
        this->move(screenGeometry.center()  - this->rect().center());
    });
}

void AddUserPage::closeEvent(QCloseEvent *event)
{
    m_mask.deleteMask(getMainWindow());
}

void AddUserPage::slt_saveBtnClicked()
{
    //必输项校验
    if(ui->nameEdit->text().isEmpty())
    {
        TipsDlgView* dlg = new TipsDlgView("姓名字段不能为空", nullptr);
        dlg->startTimer();
        dlg->show();
        return;
    }

    if(ui->accEdit->text().isEmpty())
    {
        TipsDlgView* dlg = new TipsDlgView("账号字段不能为空", nullptr);
        dlg->startTimer();
        dlg->show();
        return;
    }

    if(ui->pwdEdit->text().isEmpty())
    {
        TipsDlgView* dlg = new TipsDlgView("密码字段不能为空", nullptr);
        dlg->startTimer();
        dlg->show();
        return;
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("acc",ui->accEdit->text());
    post_data.insert("name",ui->nameEdit->text());
    post_data.insert("pic","");
    post_data.insert("pwd",ui->pwdEdit->text());
    post_data.insert("tel",ui->telEdit->text());
    post_data.insert("id",m_recordId);
    if(ui->picLabel->property("hasPic").toBool())
    {
        post_data.insert("pic",getPixmapBase64Data(ui->picLabel));
    }

    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    m_restFulApi.visitUrl(requestUrl + API_IDENTITY_AUTHENTICATION_ADD,
                          VisitType::POST,ReplyType::IDENTITY_AUTHENTICATION_ADD,"application/json",post_param,true,5000);
}

QWidget* AddUserPage::getMainWindow()
{
    foreach(QWidget *w, qApp->topLevelWidgets())
    {
        QWidget* mainWin = qobject_cast<QWidget*>(w);
        if (nullptr != mainWin && mainWin->objectName() == "MainWindowWidget")
            return mainWin;
    }
    return nullptr;
}

void AddUserPage::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::IDENTITY_AUTHENTICATION_ADD ||
            replyTypeMap.value(networkReply)==ReplyType::IDENTITY_AUTHENTICATION_DEL)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                TipsDlgView* dlg = new TipsDlgView("操作成功", nullptr);
                dlg->startTimer();
                dlg->show();
                emit sig_optSuccess();
                close();
            }
            else
            {
                TipsDlgView* dlg = new TipsDlgView(obj.value("message").toString(), nullptr);
                dlg->startTimer();
                dlg->show();
            }
        }
        else
        {
            TipsDlgView* dlg = new TipsDlgView("网络发生错误", nullptr);
            dlg->startTimer();
            dlg->show();
        }
        networkReply->deleteLater();
    }
}

void AddUserPage::slt_picSelectBtnClicked()
{
    // 设置文件过滤器
    QString filter = "Image Files (*.jpg *.jpeg *.png)";
    // 弹出文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择图片"),
        QDir::homePath(),
        filter
    );

    ui->picLabel->setProperty("hasPic",false);
    if (!fileName.isEmpty())  {
        // 用户选择了有效的图片文件
        QPixmap pixmap(fileName);
        if (!pixmap.isNull())
        {
            ui->picLabel->setProperty("hasPic",true);
            ui->picLabel->setPixmap(pixmap.scaled(ui->picLabel->size()*2,
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
        }
    }
}

QString AddUserPage::getPixmapBase64Data(QLabel *label, const char *format)
{
    if (!label || !label->pixmap()) {
        return QString();
    }

    const QPixmap &pixmap = *label->pixmap();
    if (pixmap.isNull())  {
        return QString();
    }

    QByteArray imageData;
    QBuffer buffer(&imageData);
    if (!buffer.open(QIODevice::WriteOnly))  {
        return QString();
    }

    if (!pixmap.save(&buffer,  format)) {
        return QString();
    }
    buffer.close();

    return QString::fromLatin1(imageData.toBase64());
}
