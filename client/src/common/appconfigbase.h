/**
 * @file appconfigbase.h
 * @brief 配置文件维护类基类
 * 1. 格式可以是ini、xml、json等，小项目建议ini，怎么方便怎么来，相当于将配置文件的值映射到全局变量。
 * 2. 配置文件如果配置项较多建议分组存储方便查找，而不是全部放在一个大分组中。
 * 3. 读配置文件的时候可以判断配置文件是否存在、配置项是否缺失等情况，有问题则重新生成配置文件，避免恶意删除配置文件导致程序运行异常。
 * 4. 读配置文件的时候可以填入默认值（qt配置文件类QSettings的value方法的第二个参数，set.value(“Hardware”, App::Hardware)），避免初始时候读取不到节点而导致配置项值不符合预期值类型。
 * 5. 读配置文件完成后可以重新判断配置项的值是否符合要求，对值进行过滤和矫正，防止人为打开配置文件修改后填入了异常的值，比如定时器的间隔为0，要重新纠正设定为合法的值。
 * 6. 带中文的初始值用QString::fromUtf8包起来，比如QString::fromUtf8(“管理员”)。
 * 7. 带中文的配置项要设置配置文件编码为 utf-8，set.setIniCodec(“utf-8”)。
 * @author 朱宗冬
 * @date 2023-09-21
 */
#ifndef APPCONFIGBASE_H
#define APPCONFIGBASE_H

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QSettings>
#include <QTextCodec>
#include <QDebug>

class AppConfigBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief getInstance 获取单例类的实例
     * @return AppConfigBase* 返回类的实例指针
     */
    static AppConfigBase* getInstance();


    /**
    * @brief readConfig 读取配置文件
    * @param 入参(in): QString path 路径
    */
    void readConfig(QString path = "");

    /**
    * @brief readConfig 读取相机配置文件
    * @param 入参(in): QString path 路径
    */
    void readCameraConfig(QString path = "");

    /**
    * @brief readConfigSettings 读取某一个项值
    * @param 入参(in): QString section
    * @param 入参(in): QString key
    * @param 入参(in): QString default_value
    */
    QString readConfigSettings(QString section, QString key, QString default_value = "");

    /**
    * @brief readCameraSettings 读取某一个项值
    * @param 入参(in): QString section
    * @param 入参(in): QString key
    * @param 入参(in): QString default_value
    */
    QString readCameraSettings(QString section, QString key, QString default_value = "");

    /**
    * @brief updateConfigSetting 更新某一个项值
    * @param 入参(in): QString section
    * @param 入参(in): QString key
    * @param 入参(in): QString default_value
    */
    void updateConfigSetting(QSettings *setting, QString section, QString key, const char *value);
    void updateConfigSetting(QString section, QString key, QString value);

    QString m_httpIp; //数据库访问ip
    int m_httpPort; //数据库访问端口
    QString m_identityId; //登陆身份信息ID
private:
    /**
     * @brief AppConfigBase 构造函数
     * @param 入参(in)：QObject  parent 父亲
     */
    explicit AppConfigBase(QObject *parent = nullptr);

private:
    QSettings* m_configIniRead = nullptr;
    QSettings* m_cameraIniRead = nullptr;
};

#endif // APPCONFIGBASE_H
