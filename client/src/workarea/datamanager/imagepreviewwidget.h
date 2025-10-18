#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QList>
#include <QPair>
#include <QPixmap>
#include "scaletextitem.h"
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include "restfulapi.h"
#include <QHash>
#include <QKeySequence>
#include <QString>
#include <QGraphicsItemGroup>

class ExtendedKeyMapper {
private:
    //组合键对应的文字映射
    QHash<QKeySequence, QString> keySequenceMap;
    QHash<int, QString> keyMap;

    //文字对应的颜色映射
    QHash<QString, QColor> keyColorMap;

    //英文事件和中文事件的映射
    QHash<QString, QString> en2CMap;
public:
    ExtendedKeyMapper() {
        // 初始化基本映射
        addMapping(Qt::Key_F1, "裂缝");
        addMapping(Qt::Key_F2, "坑槽");
        addMapping(Qt::Key_F3, "洒落物");
        addMapping(Qt::Key_F4, "修补");
        addMapping(Qt::Key_F5, "标线缺损");
        addMapping(Qt::Key_F6, "标志牌_变形");
        addMapping(Qt::Key_F7, "标志牌_遮挡");
        addMapping(Qt::Key_F8, "里程桩缺损");
        addMapping(Qt::Key_F9, "轮廓标缺损");
        addMapping(Qt::Key_F10, "防眩板缺损");
        addMapping(Qt::Key_F11, "护栏损坏");
        addMapping(Qt::Key_F12, "中分带苗木缺失");

        //添加其他组合键
        addMapping(Qt::Key_Q, "伸缩缝");
        addMapping(Qt::Key_W, "标志牌");
        addMapping(Qt::Key_E, "百米桩");
        addMapping(Qt::Key_R, "公里桩");
        addMapping(Qt::Key_T, "轮廓标");
        addMapping(Qt::Key_Y, "防眩板");
        addMapping(Qt::Key_U, "其他");

        //添加颜色映射
        addColorMapping("裂缝",QColor("#DA4452"));
        addColorMapping("坑槽",QColor("#FEA5C9"));
        addColorMapping("洒落物",QColor("#FFCC55"));
        addColorMapping("修补",QColor("#47CEAE"));
        addColorMapping("标线缺损",QColor("#5D9BEA"));
        addColorMapping("伸缩缝",QColor("#FC7AC3"));
        addColorMapping("标志牌",QColor("#AB91EA"));
        addColorMapping("标志牌_变形",QColor("#AB91EA"));
        addColorMapping("标志牌_遮挡",QColor("#AB91EA"));
        addColorMapping("百米桩",QColor("#8CBD4F"));
        addColorMapping("公里桩",QColor("#8CBD4F"));
        addColorMapping("里程桩缺损",QColor("#8CBD4F"));
        addColorMapping("轮廓标",QColor("#F97546"));
        addColorMapping("轮廓标缺损",QColor("#F97546"));
        addColorMapping("防眩板",QColor("#6DC7EA"));
        addColorMapping("防眩板缺损",QColor("#6DC7EA"));
        addColorMapping("护栏损坏",QColor("#FE9743"));
        addColorMapping("中分带苗木缺失",QColor("#1E9593"));
        addColorMapping("其他",QColor("#5C7090"));

        //英文转中文的映射
        adden2CMapping("pit","坑槽");
        adden2CMapping("litter","洒落物");
        adden2CMapping("strip_patch","修补");
        adden2CMapping("lane_gap","标线缺损");
        adden2CMapping("expansion_gap","伸缩缝");
        adden2CMapping("signboard","标志牌");
        adden2CMapping("hundred_pile","百米桩");
        adden2CMapping("km_pile","公里桩");
        adden2CMapping("yellow_circle_outline","轮廓标");
        adden2CMapping("unglare_plate","防眩板");
    }


    QString mapToString(const QKeyEvent *event) {
        QKeySequence seq(event->key() | event->modifiers());

        if (keySequenceMap.contains(seq))  {
            return keySequenceMap[seq];
        }

        // 回退到简单映射
        return keyEventToString(event);
    }

    QColor mapToColor(QString text)
    {
        if (keyColorMap.contains(text))  {
            return keyColorMap[text];
        }
        return QColor(255,0,0);
    }

    QString en2C(QString text)
    {
        if (en2CMap.contains(text))  {
            return en2CMap[text];
        }
        return text;
    }

private:
    QString keyEventToString(const QKeyEvent *event)
    {
        int key = event->key();

        // 从映射表中查找
        if (keyMap.contains(key))  {
            return keyMap[key];
        }

        // 默认返回空字符串（表示不支持的键）
        return "";
    }

    void addMapping(const QKeySequence &seq, const QString &value) {
        keySequenceMap[seq] = value;
    }

    void addMapping(int key, const QString &value) {
        keyMap[key] = value;
    }

    void addColorMapping(QString text, QColor value) {
        keyColorMap[text] = value;
    }

    void adden2CMapping(QString en,  QString chinese) {
        en2CMap[en] = chinese;
    }
};

class ImagePreviewWidget : public QGraphicsView
{
    Q_OBJECT
public:
    //绘制类型
    enum DRAW_TYPE
    {
        unkonwn = 0,        //未知
        coordinatePicking,  //坐标拾取
        ranging,            //测距
        events,             //绘制事件
    };


    explicit ImagePreviewWidget(QWidget *parent = nullptr);
    void loadImage(const QString& path);

    // 标注功能控制
    void setDrawEnabled(bool enabled);
    bool isDrawEnabled() const;

    // 矩形标注设置
    void setRectColor(const QColor& color);
    void setLabelText(const QString& text);

    // 获取所有标注信息
    QList<QRectF> getAnnotations() const;

    void loadImage(QPixmap pixmap, QString key = "");
    void loadPointImage(QPixmap pixmap, QString key);


    // 标注数据结构优化（解决QMap排序问题）
    struct Annotation {
        QGraphicsRectItem* rect;
        ScalableTextItem* text;

        bool operator==(const Annotation& other) const {
                return rect == other.rect  && text == other.text;
            }
    };

    //文字
    struct STU_Annotation {
        QRectF rect;
        QString text;
        bool isHandle = false;
    };

    //保存到缓存中
    void saveToCache();
    //从缓存中读取事件
    void drawFromCache();
    //显示后台记录的所有的事件
    void displayRequestEvent(QJsonObject obj);
    void readEvents2Cache(QJsonObject obj);

    //清屏
    void clearEvents();

    QMap<QString, QList<ImagePreviewWidget::STU_Annotation>> getImageCache();

    //设置绘制模式
    bool setDrawType(DRAW_TYPE type);

public slots:
    //控制是否显示点云数据
    void slt_setDisplayPointsItem(bool isDisplay);
    //人工标记处理完成
    void slt_setPersonHandleEnd();
    void slt_setPersonHandleCancle();
    void slt_btnClicked();

signals:
    void sig_personHandleEnd(QString id, bool isHandle);
    void sig_cancleCoordinatePickingSelected();
    void sig_cancleRangingSelected();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    bool eventFilter(QObject *watched, QEvent *event);
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent* event);

private slots:
    void slt_requestFinishedSlot(QNetworkReply *networkReply);

private:
    // 坐标转换系统
    QPointF sceneToImagePos(const QPointF &scenePos) const;
    QRectF imageToSceneRect(const QRectF &imageRect) const;
    void centerOnImagePoint(const QPointF &imagePos);
    bool isPointInImage(const QPointF &scenePos) const;
    void handleKeyPress(QKeyEvent *event);
    void showTipLabel(QString msg);

    // 缩放控制
    void zoomAtPosition(qreal factor, const QPointF &fixedScenePos);
    void fitToView();

    // 标注管理
    void createRectangle(const QPointF &startPos);
    void deleteRectanglesInArea(const QRectF &sceneRect);

    // 图形项
    QGraphicsPixmapItem *m_pixmapItem = nullptr;
    QGraphicsPixmapItem *m_pixmapPointsItem = nullptr;//点云图像
    QGraphicsRectItem *m_tempRectItem = nullptr;

    QList<Annotation> m_rectAnnotations;  // 保持创建顺序
    Annotation* m_currentAnnotation = nullptr;  // 当前正在绘制的标注

    // 状态变量
    bool m_isDragging = false;
    bool m_ctrlPressed = false;
    bool m_drawEnabled = false;
    bool m_panning = false;
    QPointF m_dragStartPos;
    QPoint m_lastPanPoint;
    QLabel *m_tipLabel = nullptr;

    // 标注配置
    //QColor m_rectColor = Qt::red;
    QString m_labelText = "";

    // 图片原始尺寸
    QSize m_imageSize;
    QString m_imageKey;

    ExtendedKeyMapper m_extendedKeyMapper;//键值对映射
    QPixmap m_pointPixmap;//点云图片
    bool m_isDisplayPointsPixmap = false;
    DRAW_TYPE m_drawType = DRAW_TYPE::unkonwn;

    //测距
    QGraphicsItemGroup m_rangeItemsGroup;
    //坐标拾取
    QGraphicsItemGroup m_coorItemsGroup;
    RestFulApi m_restFulApi;

private:
    QPointF getLonLatFromServer(QPointF pos);
    QString getScaleFromServer(QPointF pos1, QPointF pos2);
};

#endif // IMAGEPREVIEWWIDGET_H