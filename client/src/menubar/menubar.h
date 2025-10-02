#ifndef MENUBAR_H
#define MENUBAR_H

#include <QWidget>

namespace Ui {
class MenuBar;
}

class MenuBar : public QWidget
{
    Q_OBJECT

public:
    //菜单类型
    enum MenuType
    {
        DATA_MANAGER,//数据管理
        USER_MANAGER,//用户管理
        SYS_MANAGER,//系统管理
    };

    explicit MenuBar(QWidget *parent = nullptr);
    ~MenuBar();

    //控制那个按钮选中
    void setMenuCheck(MenuType type);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::MenuBar *ui;
};

#endif // MENUBAR_H
