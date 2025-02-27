/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBUSDRIVERINTERFACE_H
#define DBUSDRIVERINTERFACE_H

#include "MacroDefinition.h"

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QDBusContext>

#include <mutex>

class DBusDriverInterface : public QObject
{
    Q_OBJECT
public:
    inline static DBusDriverInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusDriverInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusDriverInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief uninstallDriver 卸载驱动，通过dbus调用 com.deepin.devicemanager /com/deepin/drivermanager 里面的unInstallDriver接口
     * @param driver 需要卸载的驱动名称
     */
    void uninstallDriver(const QString &driver);
    void uninstallPrinter(const QString &vendor, const QString &model);

    /**
     * @brief installDriver 更新驱动，通过dbus调用 com.deepin.devicemanager /com/deepin/drivermanager 里面的installDriver接口
     * @param driver 需要安装的驱动文件名称
     */
    void installDriver(const QString &driver);

    /**
     * @brief installDriver 更新驱动，通过dbus调用 com.deepin.devicemanager /com/deepin/drivermanager 里面的installDriver接口
     * @param driver 需要安装的驱动文件名称
     */
    void installDriver(const QString &driverName, const QString &version);

    /**
     * @brief undoInstallDriver 取消当前正在安装的驱动过程
     */
    void undoInstallDriver();

    /**
     * @brief installDriver
     * @param driver
     */
    bool isDriverPackage(const QString &path);

    /**
     * @brief isArchMatched
     * @param path
     * @return
     */
    bool isArchMatched(const QString &path);

    /**
     * @brief isDebValid
     * @param path
     * @return
     */
    bool isDebValid(const QString &path);

signals:
    void processChange(qint32 value, QString details);
    void processEnd(bool sucess, QString msg);
    void downloadProgressChanged(QStringList msg);
    void downloadFinished();
    void installProgressChanged(int progress);
    void installProgressFinished(bool bsuccess, int err);

protected:
    explicit DBusDriverInterface(QObject *parent = nullptr);
    virtual ~DBusDriverInterface();

private slots:
    /**
     * @brief slotProcessChange 链接后台驱动处理过程中发送的信号
     * @param value 当前处理的进度
     * @param detail 发送过来的时时信息
     */
    void slotProcessChange(qint32 value, QString detail);

    /**
     * @brief slotProcessEnd 接收后台结束信号
     * @param success
     */
    void slotProcessEnd(bool success, QString msg);

    /**
     * @brief slotCallFinished 更新结束结束的回调
     */
    void slotCallFinished(QDBusPendingCallWatcher *watcher);

    /**
     * @brief slotDownloadProgressChanged 驱动下载时回调，返回驱动下载进度、速度、已下载大小信息
     */
    void slotDownloadProgressChanged(QStringList msg);

    /**
     * @brief slotDownloadFinished 驱动下载完成
     */
    void slotDownloadFinished();

    /**
     * @brief slotInstallProgressChanged 驱动安装时回调，返回安装进度信息
     */
    void slotInstallProgressChanged(int progress);

    /**
     * @brief slotInstallProgressFinished 驱动安装完成回调，返回错误信息
     */
    void slotInstallProgressFinished(bool bsuccess, int err);

private:
    /**
     * @brief init 初始化工作，连接dbus
     */
    void init();

private:
    static std::atomic<DBusDriverInterface *> s_Instance;
    static std::mutex                         m_mutex;
    QDBusInterface                            *mp_Iface;
};

#endif // DBUSDRIVERINTERFACE_H
