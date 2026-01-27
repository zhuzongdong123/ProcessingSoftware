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
#include <QScreen>
#include "appconfigbase.h"
#include <QVariant>
#include"xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxworksheet.h"
#include "QDebug"
#include "xlsxworkbook.h"

void exportRecordToExcel(QJsonArray jsonArray, QString filePath)
{
    QXlsx::Format titleFormat;
    titleFormat.setFont(QFont(("宋体"),11));
    titleFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);//横向居中
    titleFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);//纵向居中
    titleFormat.setBorderStyle(QXlsx::Format::BorderThin);//边框样式

    static QMutex m_mutex;
    bool exportIsSuccess = false;
    QXlsx::Document xlsx;

    //写入表头
    if (!jsonArray.isEmpty())  {
        QJsonObject firstObj = jsonArray.first().toObject();
        QStringList headers = firstObj.keys();

        m_mutex.lock();
        for(int i = 0; i < headers.size(); i++)
        {
            xlsx.write(1,i+1,headers[i],titleFormat);


            if(headers[i] == "imageId" || headers[i] == "lat" || headers[i] == "lon")
            {
                xlsx.setColumnWidth(i+1,25);
            }
            else
            {
                xlsx.setColumnWidth(i+1,15);
            }
        }
        m_mutex.unlock();
    }

    xlsx.selectSheet("Sheet1");

    int index = 0;
    for(auto dataObj : jsonArray)  {
        QJsonObject currentObj = dataObj.toObject();
        QStringList headers = currentObj.keys();
        m_mutex.lock();
        for(int i = 0; i < headers.size(); i++)
        {
            QString value = currentObj[headers[i]].toString();
            if(headers[i] == "lat" || headers[i] == "lon")
            {
                value = QString::number(currentObj[headers[i]].toDouble(),'f',8);
            }
            else if(currentObj[headers[i]].isDouble())
            {
                value = QString::number(currentObj[headers[i]].toDouble());
            }
            xlsx.write(index+2,i+1,value,titleFormat);
        }
        m_mutex.unlock();
        index++;
    }

    xlsx.renameSheet("Sheet1","record");

    QString fileName =  QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+ ".xlsx";
    if(!filePath.isEmpty())
    {
        fileName = filePath;
    }
    qDebug().noquote() << fileName;

    //保存文件存在的话，删除
    QFile file(fileName);
    if(file.exists()) {
        file.remove();
    }

    m_mutex.lock();
    exportIsSuccess = xlsx.saveAs(fileName);
    m_mutex.unlock();
}

void saveJsonArrayToCsv(QJsonArray jsonArray, QString filePath, QMap<QString,QString> dirctionMap)
{
    QJsonArray arrayTemp;
    for(auto jsonTemp : jsonArray)
    {
        QJsonObject tempObj = jsonTemp.toObject();
        QString bagId = tempObj.value("bag_id").toString();
        QString dirction;
        if(dirctionMap.find(bagId) != dirctionMap.end())
        {
            dirction = dirctionMap.find(bagId).value();
        }
        tempObj.insert("direction",dirction);
        arrayTemp.push_back(tempObj);
    }
    jsonArray = arrayTemp;

    qDebug() << "saveJsonArrayToCsv" << jsonArray << filePath;

    //导出csv文件
    if(1)
    {
        filePath.replace(".csv",".xlsx");//保存的文件后缀修改
        exportRecordToExcel(jsonArray,filePath);
        return;
    }

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

            QJsonValue jsonValue = obj.value(key);
            if(jsonValue.isString())
                row.append(obj.value(key).toString());
            else if(jsonValue.isDouble())
            {
                if(key == "lon" || key == "lat")
                {
                    row.append(QString::number(obj.value(key).toDouble(),'f',8));
                }
                else
                {
                    row.append(QString::number(obj.value(key).toDouble()));
                }
            }
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
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

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
    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &ExportBagPage::slt_requestFinishedSlot);
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

    QTimer::singleShot(10, this, [this]() {
        QScreen *activeScreen = QGuiApplication::screenAt(QCursor::pos());
        QRect screenGeometry = activeScreen->geometry();
        this->move(screenGeometry.center()  - this->rect().center());
    });

    if(!m_isRunning)
    {
        ui->progressBar->setValue(0);
        ui->localProcess_local->hide();
    }
}

void ExportBagPage::closeEvent(QCloseEvent *event)
{
    m_mask.deleteMask(getMainWindow());
}

void ExportBagPage::slt_startExport()
{
    m_bagDrictionMap.clear();
    m_isRunning = true;
    ui->exportBtn->setEnabled(false);
    ui->selectBtn->setEnabled(false);
    ui->errorTip->clear();
    ui->progressBar->setValue(0);

    //save selected events
    QSet<QString> selectedEventsSet;
    QList<QCheckBox*> list1 = ui->groupBox1->findChildren<QCheckBox*>();
    for(auto box : list1)
    {
        QString name = box->text();
        selectedEventsSet.insert(name);
    }

    QList<QCheckBox*> list2 = ui->groupBox2->findChildren<QCheckBox*>();
    for(auto box : list2)
    {
        QString name = box->text();
        selectedEventsSet.insert(name);
    }

    //先下载所有的事件
    QList<ExportInfo> tasks;
    for(auto bagId : m_bagIdList)
    {
        QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
        this->m_restFulApi.getPostData().clear();
        QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_IMAGES_LIST_GET).arg(bagId),VisitType::GET,ReplyType::IMAGES_LIST_GET_All1);
        QEventLoop loop;
        QTimer timer;
        timer.setInterval(15000);  // 设置超时时间 3 秒
        timer.setSingleShot(true);  // 单次触发
        connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        connect(reply, &QNetworkReply::finished, &timer, &QTimer::stop);
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
                        info.image_url = requestUrl + QString(API_IMAGE_DETIAL_GET).arg(bagId).arg(filePath);
                        info.image_savePath = QString("%1/%2").arg(dirPath).arg(filePath);
                        info.bagId =bagId;
                        info.selectedEventsSet = selectedEventsSet;

                        filePath = filePath.replace(".jpg","");
                        filePath = filePath.replace(".png","");
                        info.imageId = filePath;
                        tasks.append(info);

                        qDebug() << "准备下载的信息：" << info.bagId << info.imageId << info.image_url << info.image_savePath;
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
            ui->exportBtn->setEnabled(true);
            ui->selectBtn->setEnabled(true);
            m_isRunning = false;
            return;
        }
        reply->deleteLater();

        //get dir todo ========
        AppDatabaseBase::getInstance()->getBagServerUrl();
        this->m_restFulApi.getPostData().clear();
        QNetworkReply* replyTemp = m_restFulApi.visitUrl(requestUrl + QString(API_ORIENTATION).arg(bagId),VisitType::GET,ReplyType::ORIENTATION);
        replyTemp->setProperty("bag_id",bagId);
    }

    if(tasks.size() == 0)
    {
        TipsDlgView* dlg = new TipsDlgView("需要下载的图片为空", nullptr);
        dlg->startTimer();
        dlg->show();
        ui->exportBtn->setEnabled(true);
        ui->selectBtn->setEnabled(true);
        m_isRunning = false;
        return;
    }

    ui->localProcess_local->show();

    // 2. 创建下载器实例
    BatchDownloader *downloader = new BatchDownloader;
    connect(downloader,&BatchDownloader::sig_downLoadFromUrlStep,this,[=](int current, int total){
        double radius = double(current)/double(total);
        qDebug() << "step" << radius << int(100*radius);
        if(ui->errorTip->text().isEmpty())
            ui->progressBar->setValue(100*radius);
        //QApplication::processEvents();
    },Qt::QueuedConnection);

    connect(downloader,&BatchDownloader::sig_sendAllHandleEvents,this,[=](QJsonArray array){
        QString filePath = ui->lineEditPath->text() + "/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".csv";
        QMap<QString,QString> bagDrictionMap = m_bagDrictionMap;
        QtConcurrent::run([array, filePath,bagDrictionMap]() {
                saveJsonArrayToCsv(array, filePath, bagDrictionMap);
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

    connect(downloader,&BatchDownloader::sig_handelFail,this,[=](){
       qDebug() << "stop1 ===============";
       downloader->stopAllThread();
       qDebug() << "stop2 ===============";
       //downloader->deleteLater();
       qDebug() << "stop3 ===============";
       m_isRunning = false;
       ui->exportBtn->setEnabled(true);
       ui->selectBtn->setEnabled(true);

       ui->errorTip->setText("导出失败，请稍后重试");
       TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
       dlg->startTimer();
       dlg->show();
    },Qt::QueuedConnection);

    // 3. 开始下载
    downloader->downloadImages(tasks);
}

void ExportBagPage::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::ORIENTATION)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QString direction = obj.value("data").toObject().value("orientation").toString();
                QString bagId = networkReply->property("bag_id").toString();

                if(m_bagDrictionMap.find(bagId) == m_bagDrictionMap.end())
                {
                    m_bagDrictionMap.insert(bagId,direction);
                }
                else
                {
                    m_bagDrictionMap.find(bagId).value() = direction;
                }
            }
        }
        networkReply->deleteLater();
    }
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

