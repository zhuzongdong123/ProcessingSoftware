#include "annotationdatapage.h"
#include "ui_annotationdatapage.h"
#include "appdatabasebase.h"
#include "tipsdlgview.h"
#include <QtConcurrent>
#include <QPainter>
#include <QDir>
#include "batchdownloader.h"
#include <QDir>
#include <QDebug>

#include <QPropertyAnimation>

void scrollToWidgetWithAnimation(QScrollArea* scrollArea, QWidget* widget, int duration = 100)
{
    if (!scrollArea || !widget) {
        return;
    }

    QPropertyAnimation* animation = new QPropertyAnimation(scrollArea->verticalScrollBar(), "value");
    animation->setDuration(duration);
    animation->setStartValue(scrollArea->verticalScrollBar()->value());

    // 计算目标位置（使 widget 位于视图中部）
    QPoint p = widget->pos();
    int widgetHeight = widget->height();
    int viewportHeight = scrollArea->viewport()->height();
    int targetValue = p.y() - (viewportHeight - widgetHeight) / 2;

    // 确保目标值在有效范围内
    targetValue = qMax(scrollArea->verticalScrollBar()->minimum(),
                      qMin(targetValue, scrollArea->verticalScrollBar()->maximum()));

    animation->setEndValue(targetValue);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

static QSize fixSize(315,231);
AnnotationDataPage::AnnotationDataPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnnotationDataPage)
{
    ui->setupUi(this);
    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &AnnotationDataPage::slt_requestFinishedSlot);
    connect(ui->pointsDisplayBtn, &QPushButton::clicked, ui->imagePreviewWidget, &ImagePreviewWidget::slt_setDisplayPointsItem);
    connect(ui->addHandleFlagBtn, &QPushButton::clicked, ui->imagePreviewWidget, &ImagePreviewWidget::slt_setPersonHandleEnd);
    connect(ui->deleteHandleFlagBtn, &QPushButton::clicked, ui->imagePreviewWidget, &ImagePreviewWidget::slt_setPersonHandleCancle);
    connect(ui->saveBtn, &QPushButton::clicked, this, &AnnotationDataPage::slt_btnClicked);
    connect(ui->returnBtn, &QPushButton::clicked, this, &AnnotationDataPage::slt_btnClicked);
    connect(ui->preImageBtn, &QPushButton::clicked, this, &AnnotationDataPage::slt_btnClicked);
    connect(ui->nextImageBtn, &QPushButton::clicked, this, &AnnotationDataPage::slt_btnClicked);
    m_watcher = new QFutureWatcher<QPixmap>();
    connect(m_watcher, &QFutureWatcher<QPixmap>::finished, this, &AnnotationDataPage::slt_watcherFinished,Qt::QueuedConnection);//队列连接
}

AnnotationDataPage::~AnnotationDataPage()
{
    delete ui;
}

void AnnotationDataPage::setBagId(QString id)
{
    ui->girdlayout->clearAll();
    m_currentSelectWidget = nullptr;
    ui->imagePreviewWidget->loadImage(QPixmap(),"");

    //从业务数据库中获取所有的事件
    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("bag_id",id);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_QUERY_EVENTS,
                          VisitType::POST,ReplyType::ANNOTATION_QUERY_EVENTS,"application/json",post_param,true);

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
            ui->imagePreviewWidget->readEvents2Cache(obj);
        }
    }

    QString bagFolderPath = QApplication::applicationDirPath() + "/" + id;
    QFileInfo fileInfo(bagFolderPath);
    if(fileInfo.exists() && fileInfo.isDir())
    {
        QStringList images = getImagePaths(bagFolderPath);
        QJsonArray array;
        int index = 1;
        for(auto path : images)
        {
            QJsonObject imageObj;
            ImageLoder* imageloder = new ImageLoder();
            connect(imageloder,&ImageLoder::sig_loadSuccessed,this,&AnnotationDataPage::slt_imageLoadSuccessed,Qt::QueuedConnection);
            connect(imageloder,&ImageLoder::sig_mousePressed,this,&AnnotationDataPage::sig_mousePressed,Qt::QueuedConnection);
            connect(imageloder,&ImageLoder::sig_mousePressedImage,this,&AnnotationDataPage::slt_mousePressedImage,Qt::QueuedConnection);
            connect(this,&AnnotationDataPage::sig_mousePressed,imageloder,&ImageLoder::slt_setImageSelected);
            connect(ui->imagePreviewWidget,&ImagePreviewWidget::sig_personHandleEnd,imageloder,&ImageLoder::slt_setImageHandled);
            imageObj.insert("id",index);
            index++;
            imageObj.insert("bag_id",id);
            imageObj.insert("file_path",path);
            QFileInfo fileInfoTemp(path);
            imageObj.insert("image_id",fileInfoTemp.baseName());
            imageloder->setImageInfo(imageObj);
            imageloder->setFixedSize(315,231);
            ui->girdlayout->pushBack(imageloder);
            QApplication::processEvents();
        }
    }
}

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

QStringList AnnotationDataPage::getImagePaths(const QString &folderPath)
{
    QStringList imagePaths;

    // 设置要查找的文件扩展名
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png";

    // 创建QDir对象并设置过滤器
    QDir dir(folderPath);
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files  | QDir::NoDotAndDotDot);

    // 获取所有匹配的文件信息
    QFileInfoList fileList = dir.entryInfoList();

    // 提取绝对路径
    foreach (const QFileInfo &fileInfo, fileList) {
        imagePaths.append(fileInfo.absoluteFilePath());
    }

    return imagePaths;
}

void AnnotationDataPage::saveCacheToServer()
{
    ui->imagePreviewWidget->saveToCache();
    QJsonArray insertArray;
    QMap<QString, QList<ImagePreviewWidget::STU_Annotation>> cacheMap = ui->imagePreviewWidget->getImageCache();
    QMap<QString, QList<ImagePreviewWidget::STU_Annotation>>::iterator ite;
    for(ite = cacheMap.begin(); ite != cacheMap.end(); ite++)
    {
        QJsonObject eventObj;
        {
            QList<ImagePreviewWidget::STU_Annotation> list = ite.value();
            QString key = ite.key();
            if(list.size() > 0 && key.split("-").size() >= 2)
            {
                eventObj.insert("bag_id",key.split("-")[0]);
                eventObj.insert("image_id",key.split("-")[1]);
                QJsonArray dataArrayTemp;
                for(auto eventObj : list)
                {
                    QJsonObject objTemp;
                    objTemp.insert("x1",eventObj.rect.x());
                    objTemp.insert("x2",eventObj.rect.y());
                    objTemp.insert("y1",eventObj.rect.x() + eventObj.rect.width());
                    objTemp.insert("y2",eventObj.rect.y() + eventObj.rect.height());
                    objTemp.insert("event_type",eventObj.text);
                    objTemp.insert("isHandle",eventObj.isHandle);
                    dataArrayTemp.push_back(objTemp);
                }
                QString byte = QJsonDocument(dataArrayTemp).toJson(QJsonDocument::Compact);
                eventObj.insert("data",byte);
            }
        }

        if(!eventObj.isEmpty())
        {
            insertArray.push_back(eventObj);
        }
    }

    QString requestUrl = AppDatabaseBase::getInstance()->getBusinessServerUrl();
    this->m_restFulApi.getPostData().clear();
    QJsonObject post_data;
    QJsonDocument document;
    QByteArray post_param;
    post_data.insert("data",insertArray);
    document.setObject(post_data);
    post_param = document.toJson(QJsonDocument::Compact);
    m_restFulApi.visitUrl(requestUrl + API_ANNOTATION_ADD_EVENTS,
                          VisitType::POST,ReplyType::ANNOTATION_ADD_EVENTS,"application/json",post_param,true);
}

void AnnotationDataPage::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::BAG_FILE_DETIAL)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QJsonObject objData = obj.value("data").toObject();
                QString fileName = objData.value("file_name").toString();
                ui->title->setText(fileName);

                //获取bag文件对应的图片列表
                QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
                this->m_restFulApi.getPostData().clear();
                m_restFulApi.visitUrl(requestUrl + QString(API_IMAGES_LIST_GET).arg(m_bagId),VisitType::GET,ReplyType::IMAGES_LIST_GET);
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
    else if(replyTypeMap.value(networkReply)==ReplyType::IMAGES_LIST_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                QJsonObject objData = obj.value("data").toObject();
                //ui->positionText->setText(QString("图片位置：1/%1").arg(objData.value("count").toInt()));

                //添加图片缩略图
                m_loadSuccessedImageMap.clear();
                QJsonArray array = objData.value("images").toArray();
                m_allImageCount = array.size();
                for(auto imageInfo : array)
                {
                    QJsonObject imageObj = imageInfo.toObject();
                    ImageLoder* imageloder = new ImageLoder();
                    connect(imageloder,&ImageLoder::sig_loadSuccessed,this,&AnnotationDataPage::slt_imageLoadSuccessed,Qt::QueuedConnection);
                    connect(imageloder,&ImageLoder::sig_mousePressed,this,&AnnotationDataPage::sig_mousePressed,Qt::QueuedConnection);
                    connect(imageloder,&ImageLoder::sig_mousePressedImage,this,&AnnotationDataPage::slt_mousePressedImage,Qt::QueuedConnection);
                    connect(this,&AnnotationDataPage::sig_mousePressed,imageloder,&ImageLoder::slt_setImageSelected);
                    connect(ui->imagePreviewWidget,&ImagePreviewWidget::sig_personHandleEnd,imageloder,&ImageLoder::slt_setImageHandled);

                    imageObj.insert("bag_id",m_bagId);
                    imageloder->setImageInfo(imageObj);
                    imageloder->setFixedSize(315,231);
                    ui->girdlayout->pushBack(imageloder);
                    QApplication::processEvents();
                }
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
    else if(replyTypeMap.value(networkReply)==ReplyType::CURRENT_IMAGE_DETIAL_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            QByteArray picArray = networkReply->readAll();
            QPixmap currentPicture;
            currentPicture.loadFromData(picArray);
            QString key = networkReply->property("key").toString();
            ui->imagePreviewWidget->loadImage(currentPicture,key);
        }
        else
        {
            TipsDlgView* dlg = new TipsDlgView("服务器连接失败", nullptr);
            dlg->startTimer();
            dlg->show();
        }
        networkReply->deleteLater();
        m_mask.deleteMask(ui->imagePreviewWidget);
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::POINT_IMAGE_DETIAL_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            QByteArray picArray = networkReply->readAll();
            QPixmap currentPicture;
            currentPicture.loadFromData(picArray);
            ui->imagePreviewWidget->loadPointImage(currentPicture);
        }
        networkReply->deleteLater();
    }
    else if(replyTypeMap.value(networkReply)==ReplyType::EVENT_IMAGE_DETIAL_GET)
    {
        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            auto obj=QJsonDocument::fromJson(networkReply->readAll()).object();
            if(m_restFulApi.replyResultCheck(obj,networkReply))
            {
                ui->imagePreviewWidget->displayRequestEvent(obj);
            }
        }
        networkReply->deleteLater();
    }
}

void AnnotationDataPage::slt_imageLoadSuccessed(QString filePath)
{
    if(m_loadSuccessedImageMap.find(filePath) == m_loadSuccessedImageMap.end())
    {
        m_loadSuccessedImageMap.insert(filePath,filePath);
    }

    int successedCount = m_loadSuccessedImageMap.size();
    //ui->process->setText(QString("加载进度：%1/%2").arg(successedCount).arg(m_allImageCount));
}

void AnnotationDataPage::slt_mousePressedImage(QJsonObject obj)
{
    m_currentSelectWidget = dynamic_cast<QWidget*>(sender());
    m_currentSelectObj = obj;

    QString filePath = obj.value("file_path").toString();
    QString loadFilePath = filePath;
    QPixmap currentPicture(loadFilePath);
    if(!currentPicture.isNull())
    {
        QString key = QString("%1-%2").arg(obj.value("bag_id").toString()).arg(obj.value("image_id").toString());
        ui->imagePreviewWidget->loadImage(currentPicture,key);
    }
//    else
//    {
//        //获取bag文件的详情
//        QString requestUrl = AppDatabaseBase::getInstance()->getBagServerUrl();
//        this->m_restFulApi.getPostData().clear();
//        QNetworkReply* reply = m_restFulApi.visitUrl(requestUrl + QString(API_IMAGE_DETIAL_GET).arg(obj.value("bag_id").toString()).arg(filePath),
//                              VisitType::GET,ReplyType::CURRENT_IMAGE_DETIAL_GET,"application/x-www-form-urlencoded",nullptr,true,5000,QNetworkRequest::Priority::HighPriority);
//        reply->setProperty("path",filePath);
//        QString key = QString("%1-%2").arg(obj.value("bag_id").toString()).arg(obj.value("id").toInt());
//        reply->setProperty("key",key);
//        reply->setProperty("visitPointUrl",requestUrl + QString(API_POINT_IMAGE_DETIAL_GET).arg(obj.value("bag_id").toString()).arg(filePath));

//        filePath = filePath.replace(".jpg","");
//        filePath = filePath.replace(".png","");
//        reply->setProperty("visitEventUrl",requestUrl + QString(API_EVENT_IMAGE_DETIAL_GET).arg(obj.value("bag_id").toString()).arg(filePath));
//        m_mask.insertMask(ui->imagePreviewWidget,"background-color:rgb(0,0,0,200)",0.5,"加载中,请稍后");
//    }
}

void AnnotationDataPage::slt_watcherFinished()
{

}

void AnnotationDataPage::slt_btnClicked()
{
    //保存缓存到数据库
    saveCacheToServer();

    QPushButton* btn = dynamic_cast<QPushButton*>(sender());
    if(nullptr != btn)
    {
        if(btn == ui->returnBtn)
        {
            emit sig_return();
        }
        else if(btn == ui->preImageBtn && nullptr != m_currentSelectWidget)
        {
            QList<QWidget *> *widgets = ui->girdlayout->getCurWidgetList();
            int index = widgets->indexOf(m_currentSelectWidget);
            if (index <= 0) { // 第一个或无效
                return;
            }
            QWidget* preWidget = widgets->at(index  - 1);
            if(preWidget != nullptr)
            {
                ImageLoder* imageLoder = dynamic_cast<ImageLoder*>(preWidget);
                if(nullptr != imageLoder)
                {
                    emit sig_mousePressed(imageLoder->getImageInfo().value("id").toString());
                    emit imageLoder->sig_mousePressedImage(imageLoder->getImageInfo());
                    m_currentSelectWidget = imageLoder;
                    imageLoder->setSelected(true);
                    QApplication::processEvents();
                    scrollToWidgetWithAnimation(ui->girdlayout->getScrollArea(),imageLoder);
                }
            }
        }
        else if(btn == ui->nextImageBtn && nullptr != m_currentSelectWidget)
        {
            QList<QWidget *> *widgets = ui->girdlayout->getCurWidgetList();
            int index = widgets->indexOf(m_currentSelectWidget);
            if (index < 0 || index >= (widgets->size()  - 1)) { // 最后一个或无效
                return;
            }

            QWidget* nextWidget = widgets->at(index  + 1);
            if(nextWidget != nullptr)
            {
                ImageLoder* imageLoder = dynamic_cast<ImageLoder*>(nextWidget);
                if(nullptr != imageLoder)
                {
                    emit sig_mousePressed(imageLoder->getImageInfo().value("id").toString());
                    emit imageLoder->sig_mousePressedImage(imageLoder->getImageInfo());
                    m_currentSelectWidget = imageLoder;
                    imageLoder->setSelected(true);
                    QApplication::processEvents();
                    scrollToWidgetWithAnimation(ui->girdlayout->getScrollArea(),imageLoder);
                }
            }
        }
    }
}

ImageLoder::ImageLoder(QWidget *parent) : QWidget(parent)
{
    connect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &ImageLoder::slt_requestFinishedSlot);
    connect(this, &ImageLoder::sig_loadFinished, this, &ImageLoder::slt_loadFinished,Qt::QueuedConnection);//队列连接
    m_watcher = new QFutureWatcher<QPixmap>();
    connect(m_watcher, &QFutureWatcher<QPixmap>::finished, this, &ImageLoder::slt_watcherFinished,Qt::QueuedConnection);//队列连接
}

ImageLoder::~ImageLoder()
{
    disconnect(m_watcher, &QFutureWatcher<QPixmap>::finished, this, &ImageLoder::slt_watcherFinished);//队列连接
    disconnect(&this->m_restFulApi.getAccessManager(), &QNetworkAccessManager::finished, this, &ImageLoder::slt_requestFinishedSlot);
    disconnect(this, &ImageLoder::sig_loadFinished, this, &ImageLoder::slt_loadFinished);//队列连接

    if (m_watcher->isRunning())  {
        m_watcher->waitForFinished();  // 阻塞等待
    }
}

void ImageLoder::setImageInfo(QJsonObject obj)
{
    m_obj = obj;

    QString loadFilePath = obj.value("file_path").toString();
    m_watcher->setFuture(QtConcurrent::run([=](){
        //获取字节流构造 QPixmap 对象
        QPixmap currentPicture(loadFilePath);
        //currentPicture.loadFromData(picArray);
        if(currentPicture.isNull())
        {
            return QPixmap();
        }
        QImage image = currentPicture.toImage();
        try {
            if (!image.isNull())  {
                return QPixmap::fromImage(image.scaled(
                    fixSize,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                ));
            }
            qDebug() << "QPixmap()";
            return QPixmap();
        } catch (...) {
            return QPixmap();
        }
    }));
}

QJsonObject ImageLoder::getImageInfo()
{
    return m_obj;
}

void ImageLoder::setSelected(bool value)
{
    if(m_isSelected != value)
    {
        m_isSelected = value;
        update();
    }
}

void ImageLoder::slt_setImageHandled(QString id, bool isHandle)
{
    if(m_obj.value("id").toInt() == id.toInt())
    {
        m_isHandled = isHandle;
        update();
    }
}

void ImageLoder::slt_setImageSelected(QString id)
{
    if(m_obj.value("id").toInt() == id.toInt())
    {
        if(m_isSelected != true)
        {
            m_isSelected = true;
            update();
        }
    }
    else
    {
        if(m_isSelected != false)
        {
            m_isSelected = false;
            update();
        }
    }

    QApplication::processEvents();
}

void ImageLoder::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.fillRect(rect(),QColor(200,200,200));

    if(!m_errorInfo.isEmpty())
    {
        painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿
        // 设置字体（可选）
        QFont font("微软雅黑", 18, QFont::Bold);
        painter.setFont(font);

        // 要绘制的文字
        QString text = QString("%1(%2.jpg)").arg(m_errorInfo).arg(m_obj.value("id").toInt());

        // 计算文字在窗口中的居中位置
        QFontMetrics metrics(font);
        int textWidth = metrics.horizontalAdvance(text);  // 文字宽度
        int textHeight = metrics.height();                // 文字高度

        // 计算绘制起点（居中）
        int x = (width() - textWidth) / 2;
        int y = (height() + textHeight / 2) / 2; // 垂直居中

        // 绘制文字
        painter.drawText(x,  y, text);
    }

    if (!m_scaledPixMap.isNull())  {
        // 缩放图片以适应窗口大小
        QPixmap scaled = m_scaledPixMap.scaled(this->size(),  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, scaled);

        painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿
        // 设置字体（可选）
        QFont font("微软雅黑", 12, QFont::Bold);
        painter.setPen(QColor(255,0,0));
        painter.setFont(font);

        // 要绘制的文字
        QString text = QString("%1.jpg").arg(m_obj.value("id").toInt());

        // 计算文字在窗口中的居中位置
        QFontMetrics metrics(font);
        int textWidth = metrics.horizontalAdvance(text);  // 文字宽度
        int textHeight = metrics.height();                // 文字高度

        // 计算绘制起点（居中）
        int x = (width() - textWidth) / 2;
        int y = height() - 10; // 垂直居中

        // 绘制文字
        painter.drawText(x,  y, text);
    }

    if(m_isHandled)
    {
        QPainter painter(this);
        // 1. 绘制绿色圆角矩形背景
        QRect rect(width()-50-5, 5, 50, 30); // 矩形位置和大小
        int radius = 4;             // 圆角半径

        painter.setPen(Qt::NoPen);    // 无边框
        painter.setBrush(QColor("#33B8FF")); // 绿色填充
        painter.drawRoundedRect(rect,  radius, radius);

        // 2. 绘制白色文字
        painter.setPen(Qt::white);    // 白色文字
        QFont font("Microsoft YaHei", 12, QFont::Bold); // 字体设置
        painter.setFont(font);

        // 计算文字居中位置
        QString text = "已标";
        QRect textRect = painter.fontMetrics().boundingRect(text);
        int x = rect.x() + (rect.width()  - textRect.width())  / 2;
        int y = rect.y() + (rect.height()  + textRect.height()  / 2) / 2;

        painter.drawText(x,  y, text);
    }

    if(m_isSelected)
    {
        QPainter painter(this);
        // 1. 定义矩形参数
        QRect rect(0, 0, width(), height()); // 矩形位置和大小
        int radius = 1;             // 圆角半径
        qreal borderWidth = 6.0;        // 线框粗细

        // 2. 设置绘制样式
        QPen pen(QColor("#33B8FF"), borderWidth); // 蓝色边框，指定粗细
        pen.setJoinStyle(Qt::RoundJoin);  // 设置连接处为圆角
        painter.setPen(pen);

        // 3. 绘制圆角矩形
        painter.drawRoundedRect(rect,  radius, radius);
    }
}

void ImageLoder::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit sig_mousePressed(QString::number(m_obj.value("id").toInt()));
    emit sig_mousePressedImage(m_obj);
}

void ImageLoder::slt_requestFinishedSlot(QNetworkReply *networkReply)
{
    if(replyTypeMap.value(networkReply)==ReplyType::IMAGE_DETIAL_GET)
    {
        QString path = networkReply->property("path").toString();
        QString filePath = m_obj.value("file_path").toString();
        if(!filePath.endsWith(path))
        {
            return;
        }

        //如果请求无错
        if(networkReply->error()==QNetworkReply::NoError)
        {
            QByteArray picArray = networkReply->readAll();
            m_watcher->setFuture(QtConcurrent::run([=](){
                //获取字节流构造 QPixmap 对象
                QPixmap currentPicture;
                currentPicture.loadFromData(picArray);
                if(currentPicture.isNull())
                {
                    return QPixmap();
                }
                QImage image = currentPicture.toImage();

//                //保存图片
//                {
//                    QString saveFolderPath = QApplication::applicationDirPath() + "/" + m_obj.value("bag_id").toString();
//                    QDir dir(saveFolderPath);
//                    if(!dir.exists())
//                    {
//                        dir.mkpath(".");
//                    }
//                    QString saveFilePath = saveFolderPath + "/" + path;
//                    QImage imageTemp = image.copy();
//                    imageTemp.detach();
//                    imageTemp.save(saveFilePath);
//                }

                try {
                    if (!image.isNull())  {
                        return QPixmap::fromImage(image.scaled(
                            fixSize,
                            Qt::KeepAspectRatio,
                            Qt::SmoothTransformation
                        ));
                    }
                    qDebug() << "QPixmap()";
                    return QPixmap();
                } catch (...) {
                    return QPixmap();
                }
            }));
        }
        else
        {
            m_errorInfo = "加载失败";
            emit sig_loadFinished();
        }
        networkReply->deleteLater();
    }
}

void ImageLoder::slt_loadFinished()
{
    update();
}

void ImageLoder::slt_watcherFinished()
{
    QFuture<QPixmap> future = m_watcher->future();
    if (future.result().isNull())
    {
        m_errorInfo = "加载失败";
        emit sig_loadFinished();
        return;
    }
    this->m_scaledPixMap = future.result();
    //watcher->deleteLater();
    emit sig_loadFinished();
    //emit sig_loadSuccessed(filePath);
}
