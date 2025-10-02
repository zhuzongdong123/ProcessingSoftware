#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H
#include <QDialog>
#include <QMouseEvent>
#include <QPushButton>
#include <QWidget>
#define BOUNDARY_WIDTH 4		//触发宽度
namespace Ui {
class MyMessageBox;
}

class MyMessageBox : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief MyMessageBox 自定义消息弹框
     * @param 入参(in): QString header 标题
     * @param 入参(in): QString tip 提示语
     * @param 入参(in): QString okText 确定按钮上的文字
     * @param 入参(in): QString cancelText 取消按钮上的文字
     * @param 入参(in): QWidget *parent 父亲
     */
    explicit MyMessageBox(QString header = "操作提示", QString tip = "", QString okText = "确定", QString cancelText = "取消",QWidget *parent = nullptr);
    ~MyMessageBox();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::MyMessageBox *ui;
};

#endif // MYMESSAGEBOX_H
