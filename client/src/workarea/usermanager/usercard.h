#ifndef USERCARD_H
#define USERCARD_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
class UserCard;
}

class UserCard : public QWidget
{
    Q_OBJECT

public:
    explicit UserCard(QWidget *parent = nullptr);
    ~UserCard();

    void showCardInfo(QJsonObject infoObj);

signals:
    void sig_addBtnClicked();
    void sig_editBtnClicked(QJsonObject record);
    void sig_delBtnClicked(QJsonObject record);

private:
    Ui::UserCard *ui;
    QJsonObject m_obj;
};

#endif // USERCARD_H
