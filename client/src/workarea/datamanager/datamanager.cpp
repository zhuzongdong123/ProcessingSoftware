#include "datamanager.h"
#include "ui_datamanager.h"
#include "appdatabasebase.h"
#include "tipsdlgviewForSure.h"
#include "tipsdlgview.h"
#include "appeventbase.h"

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
    //1、获取总数、解析数量、和未解析数量

    //2、获取表格内容
    getBagCounts();

    //3、获取系统占用信息
    getSysInfo();
}

void DataManager::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::BAG_LIST_GET)
    {
        //清空表格
        clearTable();

        //测试代码 todo
        resetTableInfo();
        getAllAnnotationStatus();

        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                resetTableInfo(obj);

                //获取标注状态
                getAllAnnotationStatus();
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
            TipsDlgView* dlg = new TipsDlgView(obj.value("message").toString(), nullptr);
            dlg->startTimer();
            dlg->show();
        }
        else
        {
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }

        //重新刷新
        QApplication::processEvents();
        getBagCounts();
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
                getBagCounts();
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
                        if(ui->tableWidget->item(n,COLNAME::UUID)->text().toInt() == id)
                        {
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
//                if("annotating" == annotation_status)
//                {
//                    //判断当前用户是否为管理员或者标注人



//                    TipsDlgView* dlg = new TipsDlgView("标注中的数据不允许操作", nullptr);
//                    dlg->startTimer();
//                    dlg->show();
//                }
//                else
                {
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
                getBagCounts();
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
    }
}

void DataManager::getBagCounts()
{
    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    m_restFulApi.visitUrl(requestUrl + API_BAG_LIST_GET,VisitType::GET,ReplyType::BAG_LIST_GET);
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
              << "标注状态"
//              << "标注人"<< "标注开始时间"<< "标注结束时间"
              << "数据来源"<< "占用空间"<< "操作"<< "UUID";
    ui->tableWidget->setColumnCount(labelList.size());
    ui->tableWidget->setHorizontalHeaderLabels(labelList);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    setTableProperty(COLNAME::createTime,250);
    setTableProperty(COLNAME::filePath,300);
    setTableProperty(COLNAME::operation,160);
}

void DataManager::resetTableInfo(QJsonObject objResult)
{
    _recordInfoVec.clear();
    if(!objResult.isEmpty())
    {
       QJsonArray array = objResult.value("data").toObject().value("bags").toArray();
       for(auto bagsObj : array)
       {
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
           recordInfo.dataFrom = "服务器";
           qlonglong fileSize = bagsObj.toObject().value("file_size").toVariant().toLongLong();
           recordInfo.memory = QString("%1G").arg(QString::number(fileSize/1024/1024/1024,'f',1));
           _recordInfoVec.push_back(recordInfo);
       }
       //从后台管理中获取解析的数据，存到容器中，并且排序
    }
    else
    {
       RecordInfo recordInfo;
       recordInfo.ID = "1";
       recordInfo.fileGroup = "济南绕城高速段";
       recordInfo.fileName = "new_lsd2";;
       recordInfo.filePath = "/home/sysadmin/Desktop/bags/new_lsd2.bag";;
       recordInfo.createTime = "2025-10-05 15:00:00";;
       recordInfo.handleStatus = "已解析";;
       recordInfo.drawStatus = "--";;
//       recordInfo.drawPerson = "测试用户";;
//       recordInfo.drawStartTime = "2025-10-05 22:00:00";;
//       recordInfo.drawEndTime = "2025-10-05 23:00:00";;
       recordInfo.dataFrom = "服务器";;
       qlonglong fileSize = 3753402523;
        recordInfo.memory = QString("%1G").arg(QString::number(fileSize/1024/1024/1024,'f',1));
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
       _recordInfoVec.push_back(recordInfo);
    }

    //获取和设置统计数量
    {
        m_allCount = _recordInfoVec.size();
        m_allHandleCount = _recordInfoVec.size();
        m_allunHandleCount = m_allCount - m_allHandleCount;
        ui->allCount->setText(QString::number(m_allCount));
        ui->handleCount->setText(QString::number(m_allHandleCount));
        ui->unhandleCount->setText(QString::number(m_allunHandleCount));
    }

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
//        ui->tableWidget->setItem(rowIndex,COLNAME::drawPerson,new QTableWidgetItem(space + recordInfo.drawPerson));
//        ui->tableWidget->setItem(rowIndex,COLNAME::drawStartTime,new QTableWidgetItem(space + recordInfo.drawStartTime));
//        ui->tableWidget->setItem(rowIndex,COLNAME::drawEndTime,new QTableWidgetItem(space + recordInfo.drawEndTime));
        ui->tableWidget->setItem(rowIndex,COLNAME::dataFrom,new QTableWidgetItem(space + recordInfo.dataFrom));
        ui->tableWidget->setItem(rowIndex,COLNAME::memory,new QTableWidgetItem(space + recordInfo.memory));
        ui->tableWidget->setItem(rowIndex,COLNAME::operation,new QTableWidgetItem(""));
        ui->tableWidget->setItem(rowIndex,COLNAME::UUID,new QTableWidgetItem(recordInfo.ID));

        QWidget *widget2 = new QWidget(ui->tableWidget);
        widget2->setStyleSheet("QWidget{background:transparent;} QPushButton{color: #33B8FF}");
        QHBoxLayout *layout2 = new QHBoxLayout(widget2);
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
        ui->tableWidget->setCellWidget(rowIndex,COLNAME::operation,widget2);
    }

    //隐藏列
    ui->tableWidget->hideColumn(COLNAME::UUID);

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

void DataManager::processData(QString id)
{
    tipsdlgviewForSure box("是否解析当前数据？",nullptr);
    if(box.windowExec() == 1)
    {
        return;
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
    this->m_restFulApi.getPostData().clear();
    m_restFulApi.visitUrl(requestUrl + QString(API_BAG_FILE_HANDLE).arg(id),VisitType::POST,ReplyType::BAG_FILE_HANDLE,"application/x-www-form-urlencoded",nullptr,true,1000*60*60*3);//等待一个小时

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