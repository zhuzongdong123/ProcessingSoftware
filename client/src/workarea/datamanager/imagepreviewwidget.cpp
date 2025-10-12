#include "imagepreviewwidget.h"
#include <QGraphicsScene>
#include <QScrollBar>
#include <QPen>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QJsonDocument>
#include "appdatabasebase.h"
static int originalFontSize = 50;
static QMap<QString, QList<ImagePreviewWidget::STU_Annotation>> g_allImageCache;
ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    // 场景设置
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    // 视图优化设置
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 初始化图像项
    m_pixmapItem = new QGraphicsPixmapItem();
    m_pixmapItem->setZValue(-10);
    scene->addItem(m_pixmapItem);

    //点云图片，默认不显示
    m_pixmapPointsItem = new QGraphicsPixmapItem();
    m_pixmapPointsItem->setZValue(-9);
    scene->addItem(m_pixmapPointsItem);
    m_pixmapPointsItem->setVisible(false);

    //监听事件
    parentWidget()->installEventFilter(this);
}

bool ImagePreviewWidget::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // 处理键盘事件
        handleKeyPress(keyEvent);
        return true; // 表示事件已处理
    }
    return QWidget::eventFilter(watched, event);
}

QMap<QString, QList<ImagePreviewWidget::STU_Annotation>> ImagePreviewWidget::getImageCache()
{
    return g_allImageCache;
}

void ImagePreviewWidget::showEvent(QShowEvent *event)
{
    showTipLabel("预览模式");
    this->setFocus();

    // 重置视图
    fitToView();
    scene()->setSceneRect(m_pixmapItem->boundingRect());

    // 清除已有标注
    for(auto &annotation : m_rectAnnotations) {
        scene()->removeItem(annotation.rect);
        scene()->removeItem(annotation.text);
        delete annotation.rect;
        delete annotation.text;
    }
    m_rectAnnotations.clear();
    m_currentAnnotation = nullptr;
}

void ImagePreviewWidget::closeEvent(QCloseEvent *event)
{

}

void ImagePreviewWidget::loadImage(const QString &path)
{
    QPixmap pixmap(path);
    if(pixmap.isNull())  {
        qWarning() << "Failed to load image:" << path;
        return;
    }

    m_imageSize = pixmap.size();
    m_pixmapItem->setPixmap(pixmap);

    // 重置视图
    fitToView();
    scene()->setSceneRect(m_pixmapItem->boundingRect());

    // 清除已有标注
    for(auto &annotation : m_rectAnnotations) {
        scene()->removeItem(annotation.rect);
        scene()->removeItem(annotation.text);
        delete annotation.rect;
        delete annotation.text;
    }
    m_rectAnnotations.clear();
    m_currentAnnotation = nullptr;
}

void ImagePreviewWidget::loadImage(QPixmap pixmap, QString key)
{
    //上一次的处理结果保存到缓存中
    saveToCache();

    if(!key.isEmpty())
     m_imageKey = key;

    if(pixmap.isNull())  {
        qWarning() << "Failed to load image:";
        m_pixmapItem->setPixmap(QPixmap());
        return;
    }

    m_imageSize = pixmap.size();
    m_pixmapItem->setPixmap(pixmap);

    // 重置视图
    fitToView();
    scene()->setSceneRect(m_pixmapItem->boundingRect());

    // 清除已有标注
    for(auto &annotation : m_rectAnnotations) {
        scene()->removeItem(annotation.rect);
        scene()->removeItem(annotation.text);
        delete annotation.rect;
        delete annotation.text;
    }
    m_rectAnnotations.clear();
    m_currentAnnotation = nullptr;

    //缓存中如果处理过了，就显示出来
    drawFromCache();

    //初始化的时候不显示点云数据
    m_pixmapPointsItem->setPixmap(QPixmap());
}

void ImagePreviewWidget::loadPointImage(QPixmap pixmap)
{
    m_pixmapItem->setPixmap(pixmap);
}

void ImagePreviewWidget::saveToCache()
{
    if(!m_imageKey.isEmpty() && m_rectAnnotations.size() > 0)
    {
        QList<ImagePreviewWidget::STU_Annotation> list;
        for(auto &annotation : m_rectAnnotations) {
            QRectF rect = annotation.rect->rect();
            QString text = annotation.text->toPlainText();
            STU_Annotation stuAnnotation;
            stuAnnotation.rect = rect;
            stuAnnotation.text = text;
            list.push_back(stuAnnotation);
        }

        if(g_allImageCache.find(m_imageKey) == g_allImageCache.end())
        {
            g_allImageCache.insert(m_imageKey,list);
        }
        else
        {
            g_allImageCache.find(m_imageKey).value() = list;
        }
    }
}

void ImagePreviewWidget::drawFromCache()
{
    if(g_allImageCache.find(m_imageKey) != g_allImageCache.end())
    {
        QList<ImagePreviewWidget::STU_Annotation> stuAnnotation = g_allImageCache.find(m_imageKey).value();
        for(auto &temp : stuAnnotation) {
            QRectF rect = temp.rect;
            QString text = temp.text;

            // 创建矩形项
            Annotation annotation;
            annotation.rect  = new QGraphicsRectItem(rect);
            QPen pen(m_rectColor,  2);
            pen.setCosmetic(true);
            annotation.rect->setPen(pen);
            //annotation.rect->setFlag(QGraphicsItem::ItemIsSelectable);
            scene()->addItem(annotation.rect);

            // 创建文本项
            annotation.text  = new ScalableTextItem(text);
            annotation.text->setDefaultTextColor(m_rectColor);
            annotation.text->setPos(QPointF(rect.x(),rect.y()-20));
            annotation.text->setZValue(1);  // 确保文本在矩形上方
            QFont font;
            font.setPointSize(originalFontSize);   // 设置字体大小（单位：点）
            annotation.text->setFont(font); // 应用到 QGraphicsTextItem
            scene()->addItem(annotation.text);
            // 添加到列表并设置当前标注
            m_rectAnnotations.append(annotation);
        }
    }
}

void ImagePreviewWidget::displayRequestEvent(QJsonObject obj)
{
    //todo 展示所有的事件
    QJsonArray array = obj.value("data").toObject().value("events").toArray();
    for(auto temp : array) {
        
        double x1 = temp.toObject().value("bbox_left_top").toDouble();
        double y1 = temp.toObject().value("bbox_right_top").toDouble();
        double x2 = temp.toObject().value("bbox_right_bottom").toDouble();
        double y2 = temp.toObject().value("bbox_left_bottom").toDouble();
        QRectF rect(x1,y1,x2-x1,y2-y1);

        QString text = temp.toObject().value("event_type").toString();

        // 创建矩形项
        Annotation annotation;
        annotation.rect  = new QGraphicsRectItem(rect);
        QPen pen(m_rectColor,  2);
        pen.setCosmetic(true);
        annotation.rect->setPen(pen);
        //annotation.rect->setFlag(QGraphicsItem::ItemIsSelectable);
        scene()->addItem(annotation.rect);

        // 创建文本项
        annotation.text  = new ScalableTextItem(text);
        annotation.text->setDefaultTextColor(m_rectColor);
        annotation.text->setPos(QPointF(rect.x(),rect.y()-20));
        annotation.text->setZValue(1);  // 确保文本在矩形上方
        QFont font;
        font.setPointSize(originalFontSize);   // 设置字体大小（单位：点）
        annotation.text->setFont(font); // 应用到 QGraphicsTextItem
        scene()->addItem(annotation.text);
        // 添加到列表并设置当前标注
        m_rectAnnotations.append(annotation);
    }
}

void ImagePreviewWidget::readEvents2Cache(QJsonObject obj)
{
    QJsonArray dataArray = obj.value("data").toArray();
    for(auto data : dataArray)
    {
        QString bagId = data.toObject().value("bag_id").toString();
        QString imageId = data.toObject().value("image_id").toString();
        QString key = bagId + "-" + imageId;
        QString strData = data.toObject().value("data").toString();
        QJsonDocument doc = QJsonDocument::fromJson(strData.toUtf8());
        auto eventArray = doc.array();

        QList<ImagePreviewWidget::STU_Annotation> list;
        for(auto event : eventArray)
        {
            STU_Annotation stuAnnotation;
            double x1 = event.toObject().value("x1").toDouble();
            double y1 = event.toObject().value("x2").toDouble();
            double x2 = event.toObject().value("y1").toDouble();
            double y2 = event.toObject().value("y2").toDouble();
            QRectF rect(x1,y1,x2-x1,y2-y1);
            stuAnnotation.rect = rect;
            stuAnnotation.text = event.toObject().value("event_type").toString();
            list.push_back(stuAnnotation);
        }

        //插入到缓存
        if(g_allImageCache.find(key) == g_allImageCache.end())
        {
            g_allImageCache.insert(key,list);
        }
        else
        {
            g_allImageCache.find(key).value() = list;
        }
    }
}

void ImagePreviewWidget::slt_setDisplayPointsItem(bool isDisplay)
{
    m_pixmapPointsItem->setVisible(isDisplay);
}

void ImagePreviewWidget::slt_setPersonHandleEnd()
{
    if(m_imageKey.split("-").size() == 2)
     emit sig_personHandleEnd(m_imageKey.split("-")[1],true);
}

void ImagePreviewWidget::slt_setPersonHandleCancle()
{
    if(m_imageKey.split("-").size() == 2)
        emit sig_personHandleEnd(m_imageKey.split("-")[1],false);
}

void ImagePreviewWidget::slt_btnClicked()
{

}

// =============== 坐标转换系统 ===============
QPointF ImagePreviewWidget::sceneToImagePos(const QPointF &scenePos) const
{
    if(!m_pixmapItem || m_pixmapItem->pixmap().isNull())
        return QPointF();

    QTransform itemTransform = m_pixmapItem->sceneTransform().inverted();
    return itemTransform.map(scenePos);
}

QRectF ImagePreviewWidget::imageToSceneRect(const QRectF &imageRect) const
{
    if(!m_pixmapItem || m_pixmapItem->pixmap().isNull())
        return QRectF();

    return m_pixmapItem->mapRectToScene(imageRect);
}

void ImagePreviewWidget::centerOnImagePoint(const QPointF &imagePos)
{
    centerOn(m_pixmapItem->mapToScene(imagePos));
}

bool ImagePreviewWidget::isPointInImage(const QPointF &scenePos) const
{
    if(!m_pixmapItem || m_pixmapItem->pixmap().isNull())
        return false;

    return m_pixmapItem->contains(m_pixmapItem->mapFromScene(scenePos));
}

void ImagePreviewWidget::handleKeyPress(QKeyEvent *event)
{
    // 确保是数字键盘的0键
    if (event->key() == Qt::Key_0 &&
        (event->modifiers() & Qt::KeypadModifier) &&
        (event->modifiers() & Qt::ControlModifier))
    {
        // 重置视图
        fitToView();
        scene()->setSceneRect(m_pixmapItem->boundingRect());
    }
//    else if (event->key() == Qt::Key_1 &&
//        (event->modifiers() & Qt::KeypadModifier) &&
//        (event->modifiers() & Qt::ControlModifier))
//    {
//        // 重置视图
//        if(m_pixmapPointsItem->pixmap().isNull())
//        {
//            //请求点云数据
//        }

//        m_pixmapPointsItem->setVisible(!m_pixmapPointsItem->isVisible());
//    }
//    else if (event->key() == Qt::Key_Enter &&
//        (event->modifiers() & Qt::KeypadModifier) &&
//        (event->modifiers() & Qt::ControlModifier))
//    {
//       if(m_imageKey.split("-").size() == 2)
//        emit sig_personHandleEnd(m_imageKey.split("-")[1],true);
//    }
    //control事件
    else if(event->key() == Qt::Key_Control) {
        m_ctrlPressed = true;
        if(!m_drawEnabled) {
            setCursor(Qt::CrossCursor);
        }
    }
    //esc键切换模式
    else if(event->key() == Qt::Key_Escape) {
        setDrawEnabled(false);
        showTipLabel("预览模式");
    }
    //F1-F9切换模式
    else if(event->key() == Qt::Key_F1) {
        setDrawEnabled(true);
        showTipLabel("标绘模式");

        setLabelText("事件1");
    }
    else if(event->key() == Qt::Key_F2) {
        setDrawEnabled(true);
        showTipLabel("标绘模式");

        setLabelText("事件2");
    }
    else if(event->key() == Qt::Key_F3) {
        setDrawEnabled(true);
        showTipLabel("标绘模式");

        setLabelText("事件3");
    }
    else if(event->key() == Qt::Key_F4) {
        setDrawEnabled(true);
        showTipLabel("标绘模式");

        setLabelText("事件4");
    }
    else if(event->key() == Qt::Key_F5) {
        setDrawEnabled(true);
        showTipLabel("标绘模式");

        setLabelText("事件5");
    }
}

void ImagePreviewWidget::showTipLabel(QString msg)
{
    if(nullptr == m_tipLabel)
    {
        m_tipLabel = new QLabel(this);
        m_tipLabel->setFixedSize(80,40);
        m_tipLabel->setStyleSheet("background-color: rgba(51, 184, 255,100);border:none;border-radius:4px;color:rgb(0,0,0)");
        m_tipLabel->setAlignment(Qt::AlignCenter);
        m_tipLabel->setAttribute(Qt::WA_TransparentForMouseEvents,true);
        m_tipLabel->setGeometry(10,10,m_tipLabel->width(),m_tipLabel->height());
    }
    m_tipLabel->setText(msg);

    m_tipLabel->raise();
    m_tipLabel->show();
}

// =============== 缩放控制 ===============
void ImagePreviewWidget::zoomAtPosition(qreal factor, const QPointF &fixedScenePos)
{
    // 仅在非绘制模式下允许缩放
    if(m_drawEnabled) return;

    // 保存当前鼠标位置在图像中的坐标
    QPointF imagePosBefore = sceneToImagePos(fixedScenePos);

    // 执行缩放
    scale(factor, factor);

    // 计算缩放后应移动的距离
    QPointF imagePosAfter = sceneToImagePos(fixedScenePos);
    QPointF delta = imagePosAfter - imagePosBefore;

    // 调整视图位置
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x() * transform().m11());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y() * transform().m22());
}

void ImagePreviewWidget::fitToView()
{
    if(!m_pixmapItem || m_pixmapItem->pixmap().isNull())
        return;

    // 计算最佳缩放比例
    qreal hScale = static_cast<qreal>(viewport()->width()) / m_imageSize.width();
    qreal vScale = static_cast<qreal>(viewport()->height()) / m_imageSize.height();
    qreal scaleFactor = qMin(hScale, vScale);

    // 重置变换
    resetTransform();
    scale(scaleFactor, scaleFactor);

    // 居中显示
    centerOn(m_pixmapItem);
}

// =============== 事件处理 ===============
void ImagePreviewWidget::wheelEvent(QWheelEvent *event)
{
    // 绘制模式下禁止缩放
    if(m_drawEnabled) {
        event->ignore();
        return;
    }

    // 计算缩放因子
    const qreal zoomFactor = 1.15;
    qreal factor = (event->angleDelta().y() > 0) ? zoomFactor : 1.0 / zoomFactor;

    // 以鼠标位置为中心缩放
    zoomAtPosition(factor, mapToScene(event->position().toPoint()));
    event->accept();
}

void ImagePreviewWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        // 检查点是否在图片范围内
        bool inImage = isPointInImage(scenePos);

        if(m_drawEnabled && inImage) {
            // 绘制模式：创建新矩形
            m_dragStartPos = scenePos;
            m_isDragging = true;
            createRectangle(m_dragStartPos);
        }
        else if(!m_drawEnabled) {
            if(m_ctrlPressed && inImage) {
                // 删除模式：创建临时矩形
                m_dragStartPos = scenePos;
                m_isDragging = true;
                m_tempRectItem = new QGraphicsRectItem();
                QPen pen(QColor(255, 200, 107), 2, Qt::DashLine);
                pen.setCosmetic(true);
                m_tempRectItem->setPen(pen);
                m_tempRectItem->setBrush(QBrush(QColor(255, 200, 107, 60)));
                scene()->addItem(m_tempRectItem);
                m_tempRectItem->setRect(QRectF(m_dragStartPos, QSizeF()));
            } else {
                // 平移模式：记录起始位置
                m_panning = true;
                m_lastPanPoint = event->pos();
                setCursor(Qt::ClosedHandCursor);
            }
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void ImagePreviewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_panning && !m_drawEnabled) {
        // 处理平移
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }
    else if(m_isDragging) {
        QPointF currentPos = mapToScene(event->pos());

        // 确保坐标在图片范围内
        if(!isPointInImage(currentPos)) {
            // 如果超出图片范围，则限制在图片边界
            QRectF imageRect = m_pixmapItem->boundingRect();
            currentPos.setX(qBound(imageRect.left(),  currentPos.x(), imageRect.right()));
            currentPos.setY(qBound(imageRect.top(),  currentPos.y(), imageRect.bottom()));
        }

        if(m_tempRectItem) {
            // 更新删除区域矩形
            QRectF rect(m_dragStartPos, currentPos);
            m_tempRectItem->setRect(rect.normalized());
        } else if(m_currentAnnotation) {
            // 更新当前正在绘制的矩形
            QRectF rect(m_dragStartPos, currentPos);
            m_currentAnnotation->rect->setRect(rect.normalized());

            // 更新文本位置
            m_currentAnnotation->text->setPos(rect.topLeft());
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ImagePreviewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        if(m_panning) {
            m_panning = false;
            setCursor(Qt::ArrowCursor);
        }

        if(m_isDragging) {
            m_isDragging = false;

            if(m_tempRectItem) {
                // 执行矩形删除
                deleteRectanglesInArea(m_tempRectItem->rect());
                scene()->removeItem(m_tempRectItem);
                delete m_tempRectItem;
                m_tempRectItem = nullptr;
            } else if(m_currentAnnotation) {
                //将矩形框的文字统一显示到左上角
                m_currentAnnotation->text->setPos(QPointF(m_currentAnnotation->rect->rect().x(),m_currentAnnotation->rect->rect().y()));

                // 完成当前矩形的绘制
                m_currentAnnotation = nullptr;
            }
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ImagePreviewWidget::keyPressEvent(QKeyEvent *event)
{
//    if(event->key() == Qt::Key_Control) {
//        m_ctrlPressed = true;
//        if(!m_drawEnabled) {
//            setCursor(Qt::CrossCursor);
//        }
//    }

//    //esc键切换模式
//    if(event->key() == Qt::Key_Escape) {
//        setDrawEnabled(!m_drawEnabled);
//    }
    QGraphicsView::keyPressEvent(event);
}

void ImagePreviewWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        m_ctrlPressed = false;
        if(!m_drawEnabled) {
            setCursor(Qt::ArrowCursor);
        }
    }
    QGraphicsView::keyReleaseEvent(event);
}

void ImagePreviewWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    // 只绘制图片区域背景（确保图片外不绘制）
    painter->fillRect(rect, QColor(50, 50, 50));
    if(m_pixmapItem && !m_pixmapItem->pixmap().isNull()) {
        painter->fillRect(m_pixmapItem->boundingRect(), Qt::white);
    }
}

// =============== 标注管理 ===============
void ImagePreviewWidget::createRectangle(const QPointF &startPos)
{
    if(m_labelText.isEmpty())
    {
        m_currentAnnotation = nullptr;
        return;
    }

    // 创建矩形项
    Annotation annotation;
    annotation.rect  = new QGraphicsRectItem(QRectF(startPos, QSizeF()));
    QPen pen(m_rectColor,  2);
    pen.setCosmetic(true);
    annotation.rect->setPen(pen);
    //annotation.rect->setFlag(QGraphicsItem::ItemIsSelectable);
    scene()->addItem(annotation.rect);

    // 创建文本项
    annotation.text  = new ScalableTextItem(m_labelText);
    annotation.text->setDefaultTextColor(m_rectColor);
    annotation.text->setPos(QPointF(startPos.x(),startPos.y()-20));
    annotation.text->setZValue(1);  // 确保文本在矩形上方
    QFont font;
    font.setPointSize(originalFontSize);   // 设置字体大小（单位：点）
    annotation.text->setFont(font); // 应用到 QGraphicsTextItem
    scene()->addItem(annotation.text);

    // 添加到列表并设置当前标注
    m_rectAnnotations.append(annotation);
    m_currentAnnotation = &m_rectAnnotations.last();
}

void ImagePreviewWidget::deleteRectanglesInArea(const QRectF &sceneRect)
{
    // 使用场景坐标进行精确相交检测
    for(auto it = m_rectAnnotations.begin();  it != m_rectAnnotations.end();  )
    {
        // 关键修改1：使用场景边界框(sceneBoundingRect)代替局部坐标
        QRectF itemSceneRect = it->rect->sceneBoundingRect();
        if(sceneRect.contains(itemSceneRect))
        {
            // 关键修改2：安全删除检测
            if(m_currentAnnotation &&
               it->rect == m_currentAnnotation->rect &&
               it->text == m_currentAnnotation->text)
            {
                m_currentAnnotation = nullptr;
            }

            // 从场景移除图形项
            scene()->removeItem(it->rect);
            scene()->removeItem(it->text);

            // 关键修改3：安全删除对象
            if(it->rect) {
                delete it->rect;
                it->rect = nullptr;
            }
            if(it->text) {
                delete it->text;
                it->text = nullptr;
            }

            // 从容器移除
            it = m_rectAnnotations.erase(it);

            // 关键修改4：立即触发场景更新
            scene()->update(sceneRect);  // 更新特定区域
        }
        else
        {
            ++it;
        }
    }

    // 关键修改5：强制视图刷新
    viewport()->update();  // 更新整个视口
    qApp->processEvents(); // 处理未完成的事件
}

// =============== 标注配置与状态控制 ===============
void ImagePreviewWidget::setDrawEnabled(bool enabled)
{
    if(m_drawEnabled == enabled) return;

    m_drawEnabled = enabled;
    if(m_drawEnabled) {
        setCursor(Qt::CrossCursor);
        setDragMode(QGraphicsView::NoDrag);
        // 确保没有进行中的操作
        m_panning = false;
        m_ctrlPressed = false;
    } else {
        setCursor(Qt::ArrowCursor);
    }
    //emit drawModeChanged(enabled);
}

bool ImagePreviewWidget::isDrawEnabled() const
{
    return m_drawEnabled;
}

void ImagePreviewWidget::setRectColor(const QColor &color)
{
    if(m_rectColor == color) return;

    m_rectColor = color;
//    for(auto &annotation : m_rectAnnotations) {
//        annotation.rect->setPen(QPen(color,  2));
//        annotation.text->setDefaultTextColor(color);
//    }
}

void ImagePreviewWidget::setLabelText(const QString &text)
{
    if(m_labelText == text) return;

    m_labelText = text;
//    for(auto &annotation : m_rectAnnotations) {
//        annotation.text->setPlainText(text);
//    }
}

QList<QRectF> ImagePreviewWidget::getAnnotations() const
{
    QList<QRectF> annotations;
    if(!m_pixmapItem || m_pixmapItem->pixmap().isNull())
        return annotations;

    for(const auto &annotation : m_rectAnnotations) {
        // 转换为相对于图片的坐标
        QRectF sceneRect = annotation.rect->rect();
        QRectF imageRect = m_pixmapItem->mapRectFromScene(sceneRect);

        // 规范化坐标并确保在图片范围内
        imageRect = imageRect.normalized();
        imageRect.setX(qMax(0.0,  imageRect.x()));
        imageRect.setY(qMax(0.0,  imageRect.y()));
        imageRect.setWidth(qMin(1.0  - imageRect.x(), imageRect.width()));
        imageRect.setHeight(qMin(1.0  - imageRect.y(), imageRect.height()));

        annotations.append(imageRect);
    }
    return annotations;
}