#include "datamanager.h"
#include "ui_datamanager.h"
#include "appdatabasebase.h"
#include "tipsdlgviewForSure.h"
#include "tipsdlgview.h"
#include "appeventbase.h"
#include <QDir>
#include "batchdownloader.h"

QString value2AnnotationName(QString value)
{
    if("unannotated" == value)
    {
        return "未标注";
    }
    else if("annotated" == value)
    {
        return "已标注";
    }
    else if("annotating" == value)
    {
        return "标注中";
    }
    else
    {
        return value;
    }
}

DataManager::DataManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataManager)
{
    ui->setupUi(this);

    initTable();
    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &DataManager::slt_requestFinishedSlot);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &DataManager::slt_refreshTableData);

    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[=](){
       getSysInfo();
    });
    timer->start(1500);
}

DataManager::~DataManager()
{
    delete ui;
}

void DataManager::showEvent(QShowEvent *event)
{
    slt_refreshTableData();

    //3、获取系统占用信息
    getSysInfo();
}

void DataManager::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::BAG_LIST_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                resetTableInfo(obj);
            }
        }
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::BAG_FILE_HANDLE)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            m_progressBar.close();
            TipsDlgView* dlg = new TipsDlgView(obj.value("message").toString(), nullptr);
            dlg->startTimer();
            dlg->show();
        }
        else
        {
            m_progressBar.close();
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }

        //重新刷新
        QApplication::processEvents();
        slt_refreshTableData();
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::BAG_FILE_DEL)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            TipsDlgView* dlg = new TipsDlgView(obj.value("message").toString(), nullptr);
            dlg->startTimer();
            dlg->show();
            QApplication::processEvents();

            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                //重新刷新
                slt_refreshTableData();
            }
        }
        else
        {
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::PERCENT_INFO_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply,false))
            {
                double cpuInfo = obj.value("data").toObject().value("cpu").toDouble();
                double memory = obj.value("data").toObject().value("memory").toDouble();
                double disk = obj.value("data").toObject().value("disk").toDouble();

                ui->cpuinfo->setText(QString("%1%").arg(QString::number(cpuInfo,'f',0)));
                ui->memoryInfo->setText(QString("%1%").arg(QString::number(memory,'f',0)));
                ui->diskInfo->setText(QString("%1%").arg(QString::number(disk,'f',0)));

                ui->cpuLabel->setFixedWidth(double(cpuInfo/100)*ui->cpuWidget->width());
                ui->memoryLabel->setFixedWidth(double(memory/100)*ui->memoryWidget->width());
                ui->diskLabel->setFixedWidth(double(disk/100)*ui->diskWidget->width());
            }
            else
            {
                ui->cpuinfo->setText("--");
                ui->memoryInfo->setText("--");
                ui->diskInfo->setText("--");

                ui->cpuLabel->setFixedWidth(0);
                ui->memoryLabel->setFixedWidth(0);
                ui->diskLabel->setFixedWidth(0);
            }
            emit AppEventBase::getInstance()->sig_sendServerStatus(true);
        }
        else
        {
            emit AppEventBase::getInstance()->sig_sendServerStatus(false);
        }
        networkReply->deleteLater();
        m_isGetSysInfo = false;
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::BAG_ANNOTATION_STATUS_GET_ALL)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QJsonArray array = obj.value("data").toArray();
                int nCount = ui->tableWidget->rowCount();
                for (int n = 0; n < nCount;n++)
                {
                    if(nullptr == ui->tableWidget->item(n,COLNAME::drawStatus) || nullptr == ui->tableWidget->item(n,COLNAME::UUID))
                        continue;

                    for(auto dataObj : array)
                    {
                        int id = dataObj.toObject().value("id").toInt();

                        if(ui->tableWidget->item(n,COLNAME::filePath)->text() != dataObj.toObject().value("file_path").toString())
                        {
                            continue;
                        }

                        if(ui->tableWidget->item(n,COLNAME::UUID)->text().toInt() == id)
                        {
                            //获取标注人
                            if(dataObj.toObject().value("annotation_status").toString() == "annotating")
                            {
                                QJsonObject sentryInfo = queryBagRecordSentry(ui->tableWidget->item(n,COLNAME::UUID)->text());
                                if(!sentryInfo.isEmpty())
                                {
                                     ui->tableWidget->item(n,COLNAME::drawPerson)->setText(sentryInfo.value("sentry_name").toString());
                                     ui->tableWidget->item(n,COLNAME::drawStartTime)->setText(sentryInfo.value("time").toString());
                                }
                            }

                             ui->tableWidget->item(n,COLNAME::drawStatus)->setText(value2AnnotationName(dataObj.toObject().value("annotation_status").toString()));

                             //解析或者标注状态
                             if(value2AnnotationName(dataObj.toObject().value("annotation_status").toString()) == "已标注")
                             {
                                 ui->tableWidget->item(n,COLNAME::drawStatus)->setForeground(QColor("#00DF82"));
                             }
                             else if(value2AnnotationName(dataObj.toObject().value("annotation_status").toString()) == "标注中")
                             {
                                 ui->tableWidget->item(n,COLNAME::drawStatus)->setForeground(QColor("#FFC86B"));
                             }

                             break;
                        }
                    }
                }
            }
        }
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::BAG_ANNOTATION_STATUS_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QString annotation_status = obj.value("data").toString();
                QString id = networkReply->property("uuid").toString();
                //未标注的时候，更新成已标注
                if(annotation_status == "unannotated")
                {
                    //1、迁移标记事件
                    m_mask.insertMask(getMainWindow(),"background-color:rgb(0,0,0,150)",0.5,"事件初始化中,请稍后...");
                    bool result = resetCurrentBagEvents(id);
                    m_mask.deleteMask(getMainWindow());
                    if(!result)
                    {
                        TipsDlgView* dlg = new TipsDlgView("初始化失败，请稍后重试", nullptr);
                        dlg->startTimer();
                        dlg->show();
                        return;
                    }

                    //2、更新标注状态
                    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
                    this->m_restFulApi.getPostData().clear();
                    QJsonObject post_data;
                    QJsonDocument document;
                    QByteArray post_param;
                    post_data.insert("status","annotating");
                    document.setObject(post_data);
                    post_param = document.toJson(QJsonDocument::Compact);
                    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_BAG_ANNOTATION_STATUS_SET).arg(id),
                                          VisitType::POST,ReplyType::BAG_ANNOTATION_STATUS_SET,"application/json",post_param,true,5000);
                    reply->setProperty("uuid",id);

                    //3、新增标注记录
                    inserBagRecord(id);
                }
                //标注中的时候
                else if(annotation_status == "annotating")
                {
                    //获取标注人，判断是否允许标注
                    if(AppDatabaseBase::getInstance()->m_userType != "1")
                    {
                        QJsonObject sentryObj = queryBagRecordSentry(id);
                        QString sentryId = sentryObj.value("sentry_id").toString();
                        QString sentryName = sentryObj.value("sentry_name").toString();
                        if(!sentryId.isEmpty() && sentryId != AppDatabaseBase::getInstance()->m_userId)
                        {
                            TipsDlgView* dlg = new TipsDlgView(QString("请等待[%1]标注完成").arg(sentryName), nullptr);
                            dlg->startTimer();
                            dlg->show();
                            return;
                        }
                    }

                    //直接跳转页面
                    //设置标注状态，并且跳转页面
                    QString id = networkReply->property("uuid").toString();
                    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
                    this->m_restFulApi.getPostData().clear();
                    QJsonObject post_data;
                    QJsonDocument document;
                    QByteArray post_param;
                    post_data.insert("status","annotating");
                    document.setObject(post_data);
                    post_param = document.toJson(QJsonDocument::Compact);
                    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_BAG_ANNOTATION_STATUS_SET).arg(id),
                                          VisitType::POST,ReplyType::BAG_ANNOTATION_STATUS_SET,"application/json",post_param,true,5000);
                    reply->setProperty("uuid",id);

                    //3、新增标注记录
                    inserBagRecord(id);
                }
                //已标注的时候
                else if(annotation_status == "annotated")
                {
                    //2、更新标注状态为标注中
                    QString id = networkReply->property("uuid").toString();
                    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
                    this->m_restFulApi.getPostData().clear();
                    QJsonObject post_data;
                    QJsonDocument document;
                    QByteArray post_param;
                    post_data.insert("status","annotating");
                    document.setObject(post_data);
                    post_param = document.toJson(QJsonDocument::Compact);
                    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_BAG_ANNOTATION_STATUS_SET).arg(id),
                                          VisitType::POST,ReplyType::BAG_ANNOTATION_STATUS_SET,"application/json",post_param,true,5000);
                    reply->setProperty("uuid",id);

                    //3、新增标注记录
                    inserBagRecord(id);
                }
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
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::BAG_ANNOTATION_STATUS_SET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                //跳转页面
                QString id = networkReply->property("uuid").toString();
                emit sig_turn2BagDetialPage(id);
                //重新刷新
                slt_refreshTableData();
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
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::IMAGES_LIST_GET_All)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QJsonObject objData = obj.value("data").toObject();
                QJsonArray array = objData.value("images").toArray();

                // 1. 准备下载任务 (URL + 保存路径)
                QList<QPair<QUrl, QString>> tasks;
                QString bagId = networkReply->property("bagId").toString();

                for(auto imageInfo : array)
                {
                    //测试代码
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

                    QString dirPath = QApplication::applicationDirPath() + "/" + bagId;
                    QDir dir(dirPath);
                    if (!dir.exists())  {
                        dir.mkpath(".");   // 创建路径及所有必要父目录
                    }

                    //获取bag文件的详情
                    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
                    tasks.append({
                        QUrl(requestUrl + QString(API_IMAGE_DETIAL_GET).arg(bagId).arg(filePath)),
                        QString("%1/%2").arg(dirPath).arg(filePath) // 分目录存储
                    });
                }

                // 2. 创建下载器实例
                BatchDownloader *downloader = new BatchDownloader;
                connect(downloader,&BatchDownloader::sig_save2LoacalStep,this,[=](int current, int total){
                   m_progressBar.setSaveStep(double(current)/double(total));
                },Qt::QueuedConnection);

                connect(downloader,&BatchDownloader::sig_allHandlefinished,this,[=](){
                    m_progressBar.setSaveStep(1);
                    m_progressBar.startStep4();
                   downloader->deleteLater();
                   slt_refreshTableData();
                },Qt::QueuedConnection);

                // 3. 开始下载
                downloader->downloadImages(tasks);
            }
            else
            {
               // ui->positionText->setText(obj.value("message").toString());
            }
        }
        else
        {
           // ui->positionText->setText("服务器连接失败");
        }
        networkReply->deleteLater();
    }
}

void DataManager::slt_operateBtnClicked()
{
    QPushButton* btn = dynamic_cast<QPushButton*>(sender());
    if(nullptr != btn && !btn->property("uuid").toString().isEmpty())
    {
        QString operateName = btn->text();
        QString id = btn->property("uuid").toString();
        if("解析" == operateName)
        {
            processData(id);
        }
        else if("标注" == operateName)
        {
            annotationData(id);
        }
        else if("删除" == operateName)
        {
            delData(id);
        }
        else if("下载" == operateName)
        {
            downLoadData(id);
        }
    }
}

void DataManager::slt_refreshTableData()
{
    searchBagsList();
    if(_recordInfoVec.size() > 0)
    {
        getBagCounts();
    }
}

void DataManager::getBagCounts()
{
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    m_restFulApi.visitUrl(requestUrl + API_BAG_LIST_GET,VisitType::GET,ReplyType::BAG_LIST_GET);
}

void DataManager::searchBagsList()
{
    m_allCount = 0;//全部数量
    m_allHandleCount = 0;//处理数量
    m_allunHandleCount = 0;//未处理数量
    ui->allCount->setText(QString::number(m_allCount));
    ui->handleCount->setText(QString::number(m_allHandleCount));
    ui->unhandleCount->setText(QString::number(m_allunHandleCount));

    clearTable();

    //查找所有的bag文件的路径
    QStringList folderPathList;
    folderPathList.append("/home/sysadmin/Desktop/");//测试代码
    QVector<DataManager::RecordInfo> allBasMap;
    for(auto folderPath : folderPathList)
    {
        findAllBagByFolder(folderPath,allBasMap);
    }

    //显示到表格
    _recordInfoVec = allBasMap;
    displayTable();

    //更新显示数量
    m_allCount = _recordInfoVec.size();
    ui->allCount->setText(QString::number(m_allCount));
    ui->handleCount->setText(QString::number(0));
    ui->unhandleCount->setText(QString::number(m_allCount));
}

void DataManager::getTableInfo()
{

}

void DataManager::getSysInfo()
{
    if(m_isGetSysInfo)
    {
        return;
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    m_restFulApi.visitUrl(requestUrl + API_PERCENT_INFO_GET,VisitType::GET,ReplyType::PERCENT_INFO_GET,"application/x-www-form-urlencoded",nullptr,false,15000);
    m_isGetSysInfo = true;
}

void DataManager::getAllAnnotationStatus()
{
    int nCount = ui->tableWidget->rowCount();
    if(0 == nCount)
    {
        return;
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    m_restFulApi.visitUrl(requestUrl + API_BAG_ANNOTATION_STATUS_GET_ALL,VisitType::GET,ReplyType::BAG_ANNOTATION_STATUS_GET_ALL);
}

void DataManager::clearTable()
{
    QString space = "";
    int nCount = ui->tableWidget->rowCount();
    int nClumn = ui->tableWidget->columnCount();
    for (int n = 0; n < nCount;n++)
    {
        for (int m = 0; m < nClumn ;m++)
        {
            QWidget* widget = ui->tableWidget->cellWidget(n,m);
            if(widget != nullptr)
            {
                ui->tableWidget->removeCellWidget(n,m);
                delete widget;
                widget = nullptr;
            }
        }
    }
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void DataManager::initTable()
{
    // 初始化表格
    QStringList labelList;
    labelList << "序号"<< "组名称"<< "文件名称"<< "文件路径"<< "创建时间"<< "解析状态"
              << "标注状态" << "下载状态"
              << "标注人"<< "标注时间"
              << "数据来源"<< "占用空间"<< "操作"<< "UUID";
    ui->tableWidget->setColumnCount(labelList.size());
    ui->tableWidget->setHorizontalHeaderLabels(labelList);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //隐藏列
    ui->tableWidget->hideColumn(COLNAME::UUID);
   // ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 设置表头样式
    ui->tableWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color:  #3F474F;"
        "    color: #FFFFFF;"
        "    border-bottom: 1px solid #3C4C5C;"
        "    padding: 5px;"
        "}"
    );

    // 启用样式渲染（必要时）
    ui->tableWidget->horizontalHeader()->setAttribute(Qt::WA_StyledBackground, true);
    QApplication::processEvents();

    QHeaderView* headerView = ui->tableWidget->horizontalHeader(); // 这个例子是列方向上的表头
    headerView->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    auto setTableProperty=[=](int column,int width)
    {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Fixed);
        ui->tableWidget->setColumnWidth(column, width);
    };
    //先平分表头，再设置各列宽：
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setTableProperty(COLNAME::ID,60);
    //setTableProperty(COLNAME::createTime,250);
    setTableProperty(COLNAME::filePath,300);
    setTableProperty(COLNAME::operation,160);
}

void DataManager::resetTableInfo(QJsonObject objResult)
{
    int nCount = ui->tableWidget->rowCount();
    for (int n = 0; n < nCount;n++)
    {
        if(!objResult.isEmpty())
        {
           QJsonArray array = objResult.value("data").toObject().value("bags").toArray();
           for(auto bagsObj : array)
           {
               if(ui->tableWidget->item(n,COLNAME::filePath)->text() != bagsObj.toObject().value("file_path").toString())
               {
                   continue;
               }

               RecordInfo recordInfo;
               recordInfo.ID = QString::number(bagsObj.toObject().value("id").toInt());
               recordInfo.fileGroup = "--";
               recordInfo.fileName = bagsObj.toObject().value("file_name").toString();
               recordInfo.filePath = bagsObj.toObject().value("file_path").toString();
               recordInfo.createTime = bagsObj.toObject().value("created_at").toString();
               QString status = bagsObj.toObject().value("status").toString();

               if("completed" == status)
               {
                  recordInfo.handleStatus = "已解析";
                  m_allHandleCount++;
               }
               else if("stopped" == status)
               {
                  recordInfo.handleStatus = "已停止";
               }
               else if("processing" == status)
               {
                  recordInfo.handleStatus = "正在处理";
               }
               else if("pending" == status)
               {
                  recordInfo.handleStatus = "待处理";
               }
               else if("failed" == status)
               {
                  recordInfo.handleStatus = "失败";
               }
               else
               {
                  recordInfo.handleStatus = status;
               }
               recordInfo.drawStatus = "--";;
               recordInfo.downStatus = "未下载";

               QString folderPath = QApplication::applicationDirPath() + "/" + recordInfo.ID;
               QFileInfo fileInfo(folderPath);
               if(fileInfo.exists() && fileInfo.isDir())
               {
                   recordInfo.downStatus = "已下载";
               }

               //更新解析列
               if(nullptr != ui->tableWidget->item(n,COLNAME::handleStatus))
               {
                   ui->tableWidget->item(n,COLNAME::handleStatus)->setText(recordInfo.handleStatus);
               }

               //更新下载列
               if(nullptr != ui->tableWidget->item(n,COLNAME::downStatus))
               {
                   ui->tableWidget->item(n,COLNAME::downStatus)->setText(recordInfo.downStatus);
               }

               //更新ID列
               if(nullptr != ui->tableWidget->item(n,COLNAME::UUID))
               {
                   ui->tableWidget->item(n,COLNAME::UUID)->setText(recordInfo.ID);
               }

               //更新标注人
               if(nullptr != ui->tableWidget->item(n,COLNAME::drawPerson))
               {
                   ui->tableWidget->item(n,COLNAME::drawPerson)->setText(recordInfo.drawPerson);
               }

               //更新标注时间
               if(nullptr != ui->tableWidget->item(n,COLNAME::drawStartTime))
               {
                   ui->tableWidget->item(n,COLNAME::drawStartTime)->setText(recordInfo.drawStartTime);
               }

               //添加操作列
               QWidget *widget2 = new QWidget(ui->tableWidget);
               widget2->setStyleSheet("QWidget{background:transparent;} QPushButton{color: #33B8FF}");
               QHBoxLayout *layout2 = new QHBoxLayout(widget2);
               if(recordInfo.handleStatus != "已解析")
               {
                   QPushButton* button = new QPushButton(widget2);
                   connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
                   button->setFocusPolicy(Qt::NoFocus);
                   button->setCursor(Qt::PointingHandCursor);
                   button->setFixedSize(40,30);
                   button->setText("解析");
                   button->setProperty("uuid",recordInfo.ID);
                   layout2->addWidget(button);
               }
               if(recordInfo.downStatus == "未下载" && recordInfo.handleStatus == "已解析")
               {
                   QPushButton* button = new QPushButton(widget2);
                   connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
                   button->setFocusPolicy(Qt::NoFocus);
                   button->setCursor(Qt::PointingHandCursor);
                   button->setFixedSize(40,30);
                   button->setText("下载");
                   button->setProperty("uuid",recordInfo.ID);
                   layout2->addWidget(button);
               }

               if(recordInfo.handleStatus == "已解析")
               {
                   QPushButton* button = new QPushButton(widget2);
                   connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
                   button->setFocusPolicy(Qt::NoFocus);
                   button->setCursor(Qt::PointingHandCursor);
                   button->setFixedSize(40,30);
                   button->setText("标注");
                   button->setProperty("uuid",recordInfo.ID);
                   layout2->addWidget(button);
               }

               if(recordInfo.handleStatus == "已解析")
               {
                   QPushButton* button = new QPushButton(widget2);
                   connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
                   button->setFocusPolicy(Qt::NoFocus);
                   button->setCursor(Qt::PointingHandCursor);
                   button->setFixedSize(40,30);
                   button->setText("删除");
                   button->setProperty("uuid",recordInfo.ID);
                   layout2->addWidget(button);
               }


               layout2->setContentsMargins(0,0,0,0);
               layout2->setAlignment(layout2,Qt::AlignLeft | Qt::AlignVCenter);
               widget2->setLayout(layout2);
               ui->tableWidget->setCellWidget(n,COLNAME::operation,widget2);
           }
           //从后台管理中获取解析的数据，存到容器中，并且排序
        }
    }

    //更新显示数量
    m_allunHandleCount = m_allCount - m_allHandleCount;
    ui->allCount->setText(QString::number(m_allCount));
    ui->handleCount->setText(QString::number(m_allHandleCount));
    ui->unhandleCount->setText(QString::number(m_allunHandleCount));

    //获取标注状态
    getAllAnnotationStatus();
}

void DataManager::processData(QString id)
{
    tipsdlgviewForSure box("是否解析当前数据？",nullptr);
    if(box.windowExec() == 1)
    {
        return;
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_BAG_FILE_HANDLE).arg(id),VisitType::POST,ReplyType::BAG_FILE_HANDLE,"application/x-www-form-urlencoded",nullptr,true,1000*60*60*3);//等待一个小时
    reply->setProperty("bagId",id);

    //切换单元格的文字为解析中
    int nCount = ui->tableWidget->rowCount();
    for (int n = 0; n < nCount;n++)
    {
        if(nullptr == ui->tableWidget->item(n,COLNAME::handleStatus) || ui->tableWidget->item(n,COLNAME::UUID)->text() != id)
            continue;

        ui->tableWidget->item(n,COLNAME::handleStatus)->setText("解析中");
        ui->tableWidget->item(n,COLNAME::handleStatus)->setForeground(QColor("#FFC86B"));
        break;
    }
}

void DataManager::annotationData(QString id)
{
    //判断是否解析完成
    int nCount = ui->tableWidget->rowCount();
    for (int n = 0; n < nCount;n++)
    {
        if(ui->tableWidget->item(n,COLNAME::UUID)->text() == id)
        {
            if(ui->tableWidget->item(n,COLNAME::handleStatus)->text() != "已解析")
            {
                TipsDlgView* dlg = new TipsDlgView("未解析的数据不允许标注", nullptr);
                dlg->startTimer();
                dlg->show();
                return;
            }
        }
    }

    tipsdlgviewForSure box("是否标注当前数据？",nullptr);
    if(box.windowExec() == 1)
    {
        return;
    }

    //跳转页面
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_BAG_ANNOTATION_STATUS_GET).arg(id),VisitType::GET,ReplyType::BAG_ANNOTATION_STATUS_GET);
    reply->setProperty("uuid",id);
}

void DataManager::delData(QString id)
{
    tipsdlgviewForSure box("是否删除当前数据？",nullptr);
    if(box.windowExec() == 1)
    {
        return;
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",id.toInt());
    post_data.insert("delete_parse_file",1);
    post_data.insert("delete_raw_file",1);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    m_restFulApi.visitUrl(requestUrl + QString(API_BAG_FILE_DEL).arg(id),VisitType::DELETE,ReplyType::BAG_FILE_DEL,"application/json",post_param,true,5000);
}

void DataManager::downLoadData(QString id)
{
    //判断是否解析完成
    int nCount = ui->tableWidget->rowCount();
    for (int n = 0; n < nCount;n++)
    {
        if(ui->tableWidget->item(n,COLNAME::UUID)->text() == id)
        {
            if(ui->tableWidget->item(n,COLNAME::handleStatus)->text() != "已解析")
            {
                TipsDlgView* dlg = new TipsDlgView("未解析的数据不允许下载", nullptr);
                dlg->startTimer();
                dlg->show();
                return;
            }
        }
    }


    //判断是否已经下载
    QString dirPath = QApplication::applicationDirPath() + "/" + id;
    QFileInfo dir(dirPath);
    if (dir.exists() && dir.isDir())  {
        tipsdlgviewForSure box("是否重新下载？",nullptr);
        if(box.windowExec() == 1)
        {
            return;
        }
    }

    m_progressBar.show();
    m_progressBar.startStep1();
    m_progressBar.startStep2();

    //获取bag文件对应的图片列表
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    QString bagId = id;
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_IMAGES_LIST_GET).arg(bagId),VisitType::GET,ReplyType::IMAGES_LIST_GET_All);
    reply->setProperty("bagId",bagId);
}

void DataManager::findAllBagByFolder(QString folderPath, QVector<DataManager::RecordInfo> &allBasMap)
{
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    this->m_restFulApi.getPostData().addQueryItem("path",folderPath);
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + API_FILE_SYSTEM_INFO,VisitType::GET,ReplyType::FILE_SYSTEM_INFO);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(3000);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    QStringList folderPathList;
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
        if(m_restFulApi.replyResultCheck(obj,reply))
        {
            //如果是文件的话插入
            //allBasMap.insert();

            //如果是文件夹的话，递归调用
            //findAllBagByFolder();

            QJsonArray array = obj.value("data").toObject().value("items").toArray();
            for(auto itemObj : array)
            {
                QString type = itemObj.toObject().value("type").toString();
                QString fileName = itemObj.toObject().value("name").toString();
                if(type == "directory")
                {
                    QString path = itemObj.toObject().value("path").toString();
                    findAllBagByFolder(path,allBasMap);
                }
                else if(type == "file" && fileName.endsWith(".bag"))
                {
                    RecordInfo info;

                    qlonglong fileSize = itemObj.toObject().value("size").toVariant().toLongLong();
                    info.memory = QString("%1G").arg(QString::number(fileSize/1024/1024/1024,'f',1));
                    info.fileName = itemObj.toObject().value("name").toString();
                    info.filePath = itemObj.toObject().value("path").toString();

                    QDateTime date = QDateTime::fromString(itemObj.toObject().value("modified_time").toString(), Qt::ISODate);
                    if(date.isValid())
                    {
                        info.createTime = date.toString("yyyy-MM-dd hh:mm:ss");
                    }
                    info.dataFrom = "服务器";
                    QFileInfo fileInfo(info.filePath);
                    info.fileGroup = fileInfo.dir().dirName();;
                    allBasMap.push_back(info);
                }
            }
        }
    }
    reply->deleteLater();
}

void DataManager::displayTable()
{
    //重置表格行数
    ui->tableWidget->setRowCount(_recordInfoVec.size());

    QString space = "";
    for(int rowIndex = 0; rowIndex < _recordInfoVec.size(); rowIndex++)
    {
        RecordInfo recordInfo = _recordInfoVec[rowIndex];
        ui->tableWidget->setItem(rowIndex,COLNAME::ID,new QTableWidgetItem(space + QString::number(rowIndex+1)/*recordInfo.ID*/));
        ui->tableWidget->setItem(rowIndex,COLNAME::fileGroup,new QTableWidgetItem(space + recordInfo.fileGroup));
        ui->tableWidget->setItem(rowIndex,COLNAME::fileName,new QTableWidgetItem(space + recordInfo.fileName));
        ui->tableWidget->setItem(rowIndex,COLNAME::filePath,new QTableWidgetItem(space + recordInfo.filePath));
        ui->tableWidget->setItem(rowIndex,COLNAME::createTime,new QTableWidgetItem(space + recordInfo.createTime));
        ui->tableWidget->setItem(rowIndex,COLNAME::handleStatus,new QTableWidgetItem(space + recordInfo.handleStatus));
        ui->tableWidget->setItem(rowIndex,COLNAME::drawStatus,new QTableWidgetItem(space + recordInfo.drawStatus));
        ui->tableWidget->setItem(rowIndex,COLNAME::drawPerson,new QTableWidgetItem(space + recordInfo.drawPerson));
        ui->tableWidget->setItem(rowIndex,COLNAME::drawStartTime,new QTableWidgetItem(space + recordInfo.drawStartTime));
//        ui->tableWidget->setItem(rowIndex,COLNAME::drawEndTime,new QTableWidgetItem(space + recordInfo.drawEndTime));
        ui->tableWidget->setItem(rowIndex,COLNAME::dataFrom,new QTableWidgetItem(space + recordInfo.dataFrom));
        ui->tableWidget->setItem(rowIndex,COLNAME::memory,new QTableWidgetItem(space + recordInfo.memory));
        ui->tableWidget->setItem(rowIndex,COLNAME::operation,new QTableWidgetItem(""));
        ui->tableWidget->setItem(rowIndex,COLNAME::UUID,new QTableWidgetItem(recordInfo.ID));
        ui->tableWidget->setItem(rowIndex,COLNAME::downStatus,new QTableWidgetItem(space + recordInfo.downStatus));

//        QWidget *widget2 = new QWidget(ui->tableWidget);
//        widget2->setStyleSheet("QWidget{background:transparent;} QPushButton{color: #33B8FF}");
//        QHBoxLayout *layout2 = new QHBoxLayout(widget2);
//        if(recordInfo.handleStatus != "已解析")
//        {
//            QPushButton* button = new QPushButton(widget2);
//            connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
//            button->setFocusPolicy(Qt::NoFocus);
//            button->setCursor(Qt::PointingHandCursor);
//            button->setFixedSize(40,30);
//            button->setText("解析");
//            button->setProperty("uuid",recordInfo.ID);
//            layout2->addWidget(button);
//        }
//        if(recordInfo.downStatus == "未下载" && recordInfo.handleStatus == "已解析")
//        {
//            QPushButton* button = new QPushButton(widget2);
//            connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
//            button->setFocusPolicy(Qt::NoFocus);
//            button->setCursor(Qt::PointingHandCursor);
//            button->setFixedSize(40,30);
//            button->setText("下载");
//            button->setProperty("uuid",recordInfo.ID);
//            layout2->addWidget(button);
//        }
//        {
//            QPushButton* button = new QPushButton(widget2);
//            connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
//            button->setFocusPolicy(Qt::NoFocus);
//            button->setCursor(Qt::PointingHandCursor);
//            button->setFixedSize(40,30);
//            button->setText("标注");
//            button->setProperty("uuid",recordInfo.ID);
//            layout2->addWidget(button);
//        }
//        {
//            QPushButton* button = new QPushButton(widget2);
//            connect(button,&QPushButton::clicked,this,&DataManager::slt_operateBtnClicked);
//            button->setFocusPolicy(Qt::NoFocus);
//            button->setCursor(Qt::PointingHandCursor);
//            button->setFixedSize(40,30);
//            button->setText("删除");
//            button->setProperty("uuid",recordInfo.ID);
//            layout2->addWidget(button);
//        }


//        layout2->setContentsMargins(0,0,0,0);
//        layout2->setAlignment(layout2,Qt::AlignLeft | Qt::AlignVCenter);
//        widget2->setLayout(layout2);
//        ui->tableWidget->setCellWidget(rowIndex,COLNAME::operation,widget2);
    }

    //居中显示
    int nCount = ui->tableWidget->rowCount();
    int nClumn = ui->tableWidget->columnCount();
    for (int n = 0; n < nCount;n++)
    {
        for (int m = 0; m < nClumn ;m++)
        {
            if(nullptr == ui->tableWidget->item(n,m))
                continue;

            ui->tableWidget->item(n,m)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            ui->tableWidget->item(n,m)->setToolTip(ui->tableWidget->item(n,m)->text());

            //解析或者标注状态
            if((COLNAME::handleStatus == m && ui->tableWidget->item(n,m)->text() == "已解析") ||
                    (COLNAME::drawStatus == m && ui->tableWidget->item(n,m)->text() == "已标注"))
            {
                ui->tableWidget->item(n,m)->setForeground(QColor("#00DF82"));
            }
            else if((COLNAME::handleStatus == m && ui->tableWidget->item(n,m)->text() == "解析中") ||
                    (COLNAME::drawStatus == m && ui->tableWidget->item(n,m)->text() == "标注中"))
            {
                ui->tableWidget->item(n,m)->setForeground(QColor("#FFC86B"));
            }
        }
    }

    for(int i = 0; i< ui->tableWidget->rowCount(); i++)
    {
       ui->tableWidget->resizeRowToContents(i);
    }
}

bool DataManager::resetCurrentBagEvents(QString bagId)
{
    //获取bag文件对应的图片列表
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_IMAGES_LIST_GET).arg(bagId),VisitType::GET,ReplyType::IMAGES_LIST_GET_All_loop);
    reply->setProperty("bagId",bagId);
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(10000);  // 设置超时时间 3 秒
    timer.setSingleShot(true);  // 单次触发
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    QJsonObject returnObj;
    QStringList requestUrlList;
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
        if(m_restFulApi.replyResultCheck(obj,reply))
        {
            QJsonObject objData = obj.value("data").toObject();
            QJsonArray array = objData.value("images").toArray();
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

                //获取所有的事件
                filePath = filePath.replace(".jpg","");
                filePath = filePath.replace(".png","");
                requestUrlList.append(filePath);
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    reply->deleteLater();

    //获取所有的事件
    QJsonArray insertArray;
    {
        for(auto imageId : requestUrlList)
        {
            this->m_restFulApi.getPostData().clear();
            QString url = requestUrl + QString(API_EVENT_IMAGE_DETIAL_GET).arg(bagId).arg(imageId);
            QNetworkReply* reply = m_restFulApi.visitUrl(url,VisitType::GET,ReplyType::EVENT_IMAGE_DETIAL_GET,"application/x-www-form-urlencoded",nullptr,true,8000,QNetworkRequest::Priority::HighPriority);
            QEventLoop loop;
            QTimer timer;
            timer.setInterval(10000);  // 设置超时时间 3 秒
            timer.setSingleShot(true);  // 单次触发
            connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            timer.start();
            loop.exec();

            QJsonObject eventObj;
            QStringList requestUrlList;
            if(reply->error()==QNetworkReply::NoError)
            {
                auto obj=QJsonDocument::fromJson(reply->readAll()).object();
                if(m_restFulApi.replyResultCheck(obj,reply))
                {
                    QJsonObject objData = obj.value("data").toObject();
                    QJsonArray array = objData.value("events").toArray();
                    if(array.size() > 0)
                    {
                        eventObj.insert("bag_id",bagId);
                        eventObj.insert("image_id",imageId);
                        QJsonArray dataArrayTemp;
                        for(auto eventObj : array)
                        {
                            QJsonObject objTemp;
                            objTemp.insert("x1",eventObj.toObject().value("bbox_left_top").toInt());
                            objTemp.insert("x2",eventObj.toObject().value("bbox_right_top").toInt());
                            objTemp.insert("y1",eventObj.toObject().value("bbox_right_bottom").toInt());
                            objTemp.insert("y2",eventObj.toObject().value("bbox_left_bottom").toInt());
                            objTemp.insert("event_type",eventObj.toObject().value("event_type").toString());
                            dataArrayTemp.push_back(objTemp);
                        }
                        QString byte = QJsonDocument(dataArrayTemp).toJson(QJsonDocument::Compact);
                        eventObj.insert("data",byte);
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            if(!eventObj.isEmpty())
            {
                insertArray.push_back(eventObj);
            }
            reply->deleteLater();
        }
    }

    //迁移数据存在的时候，插入到业务数据库中
    if(insertArray.size() > 0)
    {
        QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
        this->m_restFulApi.getPostData().clear();
        QJsonObject post_data;
        QJsonDocument document;
        QByteArray post_param;
        post_data.insert("data",insertArray);
        document.setObject(post_data);
        post_param = document.toJson(QJsonDocument::Compact);
        QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_ADD_EVENTS,
                              VisitType::POST,ReplyType::ANNOTATION_ADD_EVENTS,"application/json",post_param,true);

        QEventLoop loop;
        QTimer timer;
        timer.setInterval(10000);  // 设置超时时间 3 秒
        timer.setSingleShot(true);  // 单次触发
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();

        QJsonObject returnObj;
        QStringList requestUrlList;
        if(reply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(reply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,reply))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

void DataManager::inserBagRecord(QString bagId)
{
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",bagId);
    post_data.insert("sentry_id",AppDatabaseBase::getInstance()->m_userId);
    post_data.insert("time",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    post_data.insert("sentry_name",AppDatabaseBase::getInstance()->m_userName);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_ADD,
                          VisitType::POST,ReplyType::ANNOTATION_ADD,"application/json",post_param,true,5000);
}

QJsonObject DataManager::queryBagRecordSentry(QString bagId)
{
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",bagId);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_QUERY,
                          VisitType::POST,ReplyType::ANNOTATION_QUERY,"application/json",post_param,true,5000);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QJsonObject returnObj;
    QStringList folderPathList;
    if(reply->error()==QNetworkReply::NoError)
    {
        auto obj=QJsonDocument::fromJson(reply->readAll()).object();
        if(m_restFulApi.replyResultCheck(obj,reply))
        {
            QJsonArray array = obj.value("data").toArray();
            for(auto dataObj : array)
            {
                returnObj = dataObj.toObject();
                break;
            }
        }
    }
    reply->deleteLater();
    return  returnObj;
}

QWidget* DataManager::getMainWindow()
{
    foreach(QWidget *w, qApp->topLevelWidgets())
    {
        QWidget* mainWin = qobject_cast<QWidget*>(w);
        if (nullptr != mainWin && mainWin->objectName() == "MainWindowWidget")
            return mainWin;
    }
    return nullptr;
}