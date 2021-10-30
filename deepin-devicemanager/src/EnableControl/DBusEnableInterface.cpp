#include "DBusEnableInterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusEnableInterface *> DBusEnableInterface::s_Instance;
std::mutex DBusEnableInterface::m_mutex;

const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString ENABLE_SERVICE_PATH = "/com/deepin/enablemanager";

DBusEnableInterface::DBusEnableInterface()
    : mp_Iface(nullptr)
{
    // 初始化dbus
    init();
}

bool DBusEnableInterface::getRemoveInfo(QString &info)
{
    // 调用dbus接口获取设备信息
    QDBusReply<QString> reply = mp_Iface->call("getRemoveInfo");
    if (reply.isValid()) {
        info = reply.value();
        return true;
    } else {
        return false;
    }
}

bool DBusEnableInterface::getAuthorizedInfo(QString& info)
{
    QDBusReply<QString> reply = mp_Iface->call("getAuthorizedInfo");
    if (reply.isValid()) {
        info = reply.value();
        return true;
    } else {
        info = "";
        return false;
    }
}

bool DBusEnableInterface::enable(const QString& hclass, const QString& name, const QString& path, const QString& value, bool enable_device)
{
    QDBusReply<QString> reply = mp_Iface->call("enable", hclass, name, path, value, enable_device);
    QString msg;
    if (reply.isValid()) {
        msg = reply.value();
    }

    if(msg == "1"){
        return true;
    }
    return false;
}

bool DBusEnableInterface::enablePrinter(const QString& hclass, const QString& name, const QString& path, bool enable_device)
{
    QDBusReply<QString> reply = mp_Iface->call("enablePrinter", hclass, name, path, enable_device);
    QString msg;
    if (reply.isValid()) {
        msg = reply.value();
    }

    if(msg == "1"){
        return true;
    }
    return false;
}

void DBusEnableInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, ENABLE_SERVICE_PATH, "", QDBusConnection::systemBus());
}
