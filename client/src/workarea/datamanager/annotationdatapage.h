#ifndef ANNOTATIONDATAPAGE_H
#define ANNOTATIONDATAPAGE_H

#include <QWidget>
#include "restfulapi.h"
#include <QJsonObject>
#include <QMutex>
#include <QFutureWatcher>
#include "mymask.h"

class ImageLoder : public QWidget
{
    Q_OBJECT

public:
    explicit ImageLoder(QWidget *parent = nullptr);
    ~ImageLoder();

    //设置图片的信息
    void setImageInfo(QJsonObject obj);

public slots:
    //哪个图片被选中了
    void slt_setImageSelected(QString id);
    //标记哪个图片被标记了
    void slt_setImageHandled(QString id, bool isHandle);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void sig_loadFinished();
    void sig_loadSuccessed(QString filePath);
    void sig_mousePressed(QString id);
    void sig_mousePressedImage(QJsonObject obj);

private slots:
    void slt_requestFinishedSlot(QNetworkReply *networkReply);
    void slt_loadFinished();
    void slt_watcherFinished();

private:
    QJsonObject m_obj;
    RestFulApi m_restFulApi;
    QString m_errorInfo = "加载中";
//    QMutex m_mutex;
    QPixmap m_scaledPixMap;
    QFutureWatcher<QPixmap> *m_watcher = nullptr;
    bool m_isHandled = false;
    bool m_isSelected = false;
};


namespace Ui {
class AnnotationDataPage;
}

class AnnotationDataPage : public QWidget
{
    Q_OBJECT

public:
    explicit AnnotationDataPage(QWidget *parent = nullptr);
    ~AnnotationDataPage();

    //设置bag文件的id
    void setBagId(QString id);

signals:
    void sig_mousePressed(QString id);

private:
    Ui::AnnotationDataPage *ui;
    RestFulApi m_restFulApi;
    QString m_bagId;
    int m_allImageCount;
    QMap<QString,QString> m_loadSuccessedImageMap;
    QJsonObject m_currentSelectObj;
    MyMask m_mask;

private slots:
    void slt_requestFinishedSlot(QNetworkReply *networkReply);
    void slt_imageLoadSuccessed(QString filePath);
    void slt_mousePressedImage(QJsonObject obj);
};

#endif // ANNOTATIONDATAPAGE_H
