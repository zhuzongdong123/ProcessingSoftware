#ifndef SCALETEXTITEM_H
#define SCALETEXTITEM_H

#include <QGraphicsTextItem>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>

class ScalableTextItem : public QGraphicsTextItem {
    Q_OBJECT

public:
    ScalableTextItem(const QString& text, QGraphicsItem* parent = nullptr)
        : QGraphicsTextItem(text, parent) {}

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemSceneHasChanged) {
            // 当项被添加到场景时，连接场景的变换信号
            if (scene()) {
                connect(scene(), &QGraphicsScene::sceneRectChanged, this, &ScalableTextItem::updateScale);
            }
        }
        return QGraphicsTextItem::itemChange(change, value);
    }

private:
    void updateScale() {
        if (!scene()) return;
        // 获取视图的变换矩阵
        QTransform viewTransform = scene()->views().first()->transform();
        // 设置项的变换矩阵
        setTransform(viewTransform);
    }
};
#endif // SCALETEXTITEM_H
