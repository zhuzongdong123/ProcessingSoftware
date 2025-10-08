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

class ImagePreviewWidget : public QGraphicsView
{
    Q_OBJECT
public:
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

    void loadImage(QPixmap pixmap, QString key);
    void loadPointImage(QPixmap pixmap);


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

public slots:
    //控制是否显示点云数据
    void slt_setDisplayPointsItem(bool isDisplay);
    //人工标记处理完成
    void slt_setPersonHandleEnd();
    void slt_setPersonHandleCancle();

signals:
    void sig_personHandleEnd(QString id, bool isHandle);

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
    QColor m_rectColor = Qt::red;
    QString m_labelText = "";

    // 图片原始尺寸
    QSize m_imageSize;
    QString m_imageKey;
};

#endif // IMAGEPREVIEWWIDGET_H