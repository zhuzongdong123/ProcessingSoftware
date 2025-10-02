#include "appglobal.h"
#include "dao.h"
#include <QUuid>
AppGlobal::AppGlobal(QObject *parent) : QObject(parent)
{
    m_timerDbStatus = new QTimer();
    connect(m_timerDbStatus,&QTimer::timeout,this,&AppGlobal::slt_timerDbStatus);
}

AppGlobal::~AppGlobal()
{
    m_timerDbStatus->deleteLater();
}

AppGlobal &AppGlobal::Instance()
{
    static AppGlobal intstance;
    return intstance;
}

bool AppGlobal::dbOpenStatus()
{
    QMutexLocker locker(&m_muStatus);
    return m_bdbOpen;
}

void AppGlobal::setOpenStatus(bool status)
{
    QMutexLocker locker(&m_muStatus);
    m_bdbOpen = status;
}

void AppGlobal::initDbStatus()
{
    DAO dao;
    m_dbId = QUuid::createUuid().toString().remove("-").remove("{").remove("}");
    m_bdbOpen = dao.openConnection(m_dbId);
    m_timerDbStatus->start(3000);
}

void AppGlobal::slt_timerDbStatus()
{
    DAO dao;
    bool status = dao.openConnection(m_dbId);
    setOpenStatus(status);
}

