#include "exportbagpage.h"
#include "ui_exportbagpage.h"
#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include "appdatabasebase.h"
#include <QTimer>
#include "batchdownloader.h"
#include "tipsdlgview.h"
#include <QtConcurrent>

void saveJsonArrayToCsv(QJsonArray jsonArray, QString filePath)
{
    qDebug() << "saveJsonArrayToCsv" << jsonArray << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly  | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << filePath;
        return;
    }

    QTextStream out(&file);

    // 写入CSV表头(假设所有对象有相同结构)
    if (!jsonArray.isEmpty())  {
        QJsonObject firstObj = jsonArray.first().toObject();
        QStringList headers = firstObj.keys();
        out << headers.join("\t")  << "\n";
    }

    // 逐行写入数据
    for (const QJsonValue& value : jsonArray) {
        QJsonObject obj = value.toObject();
        QStringList row;
        for (const QString& key : obj.keys())  {
            row.append(obj.value(key).toString());
        }
        out << row.join("\t")  << "\n";
    }

    file.close();
}

ExportBagPage::ExportBagPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::exportBagPage)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    connect(ui->selectBtn, &QPushButton::clicked, [this](){
        // 2. 记住上次选择目录（使用QSettings）
        QString lastDir = settings.value("LastDirectory",  QDir::homePath()).toString();
        QString newDir = QFileDialog::getExistingDirectory(this, "选择目录", lastDir);
        if(newDir.isEmpty())
            return;

        settings.setValue("LastDirectory",  newDir);
        ui->lineEditPath->setText(newDir);
    });
    connect(ui->closeBtn,&QPushButton::clicked, this, &ExportBagPage::close);
    connect(ui->exportBtn,&QPushButton::clicked, this, &ExportBagPage::slt_startExport);
}

ExportBagPage::~ExportBagPage()
{
    delete ui;
}

void ExportBagPage::setBagIds(QSet<QString> bagIdList)
{
    if(m_isRunning)
        return;

    m_bagIdList = bagIdList;

    //初始化页面
    ui->lineEditPath->clear();
    ui->localProcess_local->hide();
    ui->localProcess_server->hide();
}

//获取单例类的实例
ExportBagPage* ExportBagPage::getInstance()
{
    ExportBagPage* pAppCommonBase = nullptr;
    static ExportBagPage appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}

void ExportBagPage::showEvent(QShowEvent *event)
{
    m_mask.insertMask(getMainWindow(),"background-color:rgb(0,0,0,150)",0.5);

    raise();
    show();
}

void ExportBagPage::closeEvent(QCloseEvent *event)
{
    m_mask.deleteMask(getMainWindow());
}

void ExportBagPage::slt_startExport()
{
    m_isRunning = true;
    ui->exportBtn->setEnabled(false);
    ui->selectBtn->setEnabled(false);
    //先下载所有的事件
    QList<ExportInfo> tasks;
    for(auto bagId : m_bagIdList)
    {
        QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
        this->m_restFulApi.getPostData().clear();
        QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_IMAGES_LIST_GET).arg(bagId),VisitType::GET,ReplyType::IMAGES_LIST_GET_All1);
        QEventLoop loop;
        QTimer timer;
        timer.setInterval(3000);  // 设置超时时间 3 秒
        timer.setSingleShot(true);  // 单次触发
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();

        if(reply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(reply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,reply))
            {
                if(m_restFulApi.replyResultCheck(obj,reply))
                {
                    QJsonObject objData = obj.value("data").toObject();
                    QJsonArray array = objData.value("images").toArray();

                    // 1. 准备下载任务 (URL + 保存路径)
                    for(auto imageInfo : array)
                    {
                        QString filePath = imageInfo.toObject().value("file_path").toString();
                        QStringList list = filePath.split("/");
                        if(list.size() > 0)
                        {
                            filePath = *(list.end()-1);
                        }
                        else
                        {
                            continue;
                        }

                        QString dirPath = ui->lineEditPath->text() + "/" + bagId;
                        QDir dir(dirPath);
                        if (!dir.exists())  {
                            dir.mkpath(".");   // 创建路径及所有必要父目录
                        }

                        //获取bag文件的详情
                        QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();

                        ExportInfo info;
                        info.url = requestUrl + QString(API_IMAGE_DETIAL_GET).arg(bagId).arg(filePath);
                        info.savePath = QString("%1/%2").arg(dirPath).arg(filePath);
                        info.bagId =bagId;

                        filePath = filePath.replace(".jpg","");
                        filePath = filePath.replace(".png","");
                        info.imageId = filePath;
                        tasks.append(info);

                        qDebug() << "准备下载的信息：" << info.bagId << info.imageId << info.url << info.savePath;
                    }
                }
            }
        }
        else
        {
            qDebug() << "服务器连接失败";
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
            m_isRunning = false;
            return;
        }
    }

    if(tasks.size() == 0)
    {
        TipsDlgView* dlg = new TipsDlgView("需要下载的图片为空", nullptr);
        dlg->startTimer();
        dlg->show();
        m_isRunning = false;
        return;
    }

    ui->localProcess_local->show();

    // 2. 创建下载器实例
    BatchDownloader *downloader = new BatchDownloader;
    connect(downloader,&BatchDownloader::sig_save2LoacalStep,this,[=](int current, int total){
        double radius = double(current/total);
        ui->progressBar->setValue(100*radius);
    },Qt::QueuedConnection);

    connect(downloader,&BatchDownloader::sig_sendAllHandleEvents,this,[=](QJsonArray array){
        QString filePath = ui->lineEditPath->text() + "/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss.csv");
        QtConcurrent::run([array, filePath]() {
                saveJsonArrayToCsv(array, filePath);
                qDebug() << "CSV file saved successfully";
            });
    },Qt::QueuedConnection);

    connect(downloader,&BatchDownloader::sig_allHandlefinished,this,[=](){
        ui->progressBar->setValue(100);
       downloader->deleteLater();
       m_isRunning = false;
       ui->exportBtn->setEnabled(true);
       ui->selectBtn->setEnabled(true);
    },Qt::QueuedConnection);

    // 3. 开始下载
    downloader->downloadImages(tasks);
}

QWidget* ExportBagPage::getMainWindow()
{
    foreach(QWidget *w, qApp->topLevelWidgets())
    {
        QWidget* mainWin = qobject_cast<QWidget*>(w);
        if (nullptr != mainWin && mainWin->objectName() == "MainWindowWidget")
            return mainWin;
    }
    return nullptr;
}

