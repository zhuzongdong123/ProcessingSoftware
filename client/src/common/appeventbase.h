/**
 * @file appeventbase.h
 * @brief 公共事件维护单例类基类
 * 1. 此类必须是全局单例类，便于全局统一使用。
 * 2. 比如类a的父类是b，类b的父类是c，现在有个信号要发给类d，
 *     在没有事件中转处理的情况下的做法是将a信号发给b，b再发给c，c再发给d，如果父类嵌套层级越多越复杂，代码越难管理。
 * 3. 将类a的信号发给appevent类，然后类d直接关联appevent类进行处理就行。
 * 4. 项目越大，会越发现事件中转处理的必要性，代码清晰，管理方便。
 * @author 朱宗冬
 * @date 2023-09-21
 */
#ifndef APPEVENTBASE_H
#define APPEVENTBASE_H

#include <QObject>
#include <QDebug>
#include <QJsonObject>
#include <QDateTime>

enum DEV_TYPE
{
    CAMERA0,
    CAMERA1,
    CAMERA3,
    GPS,
    accelerometer, //加速度
    flatnessTester, //平整度仪
};

class AppEventBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief getInstance 获取单例类的实例
     * @return AppEventBase* 返回类的实例指针
     */
    static AppEventBase* getInstance();
signals:
    void sig_sendServerStatus(bool status);
};

#endif // APPEVENTBASE_H
