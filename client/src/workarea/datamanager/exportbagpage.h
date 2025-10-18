#ifndef EXPORTBAGPAGE_H
#define EXPORTBAGPAGE_H

#include <QWidget>
#include <QSet>
#include <QSettings>
#include "mymask.h"
#include "restfulapi.h"

namespace Ui {
class exportBagPage;
}

class ExportBagPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExportBagPage(QWidget *parent = nullptr);
    ~ExportBagPage();

    //设置选择的bag数据列表
    void setBagIds(QSet<QString> bagIdList);

    static ExportBagPage *getInstance();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::exportBagPage *ui;
    QSettings settings;
    bool m_isRunning = false;
    QSet<QString> m_bagIdList;
    MyMask m_mask;
    RestFulApi m_restFulApi;

private slots:
    void slt_startExport();

private:
    QWidget *getMainWindow();
};

#endif // EXPORTBAGPAGE_H
