/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
* Maintainer: xiaomei.ji <jixiaomei@uniontech.com>
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
#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>

#include <mutex>

class QDBusInterface;

class DBusInterface
{
public:
    inline static DBusInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief getInfo：获取信息
     * @param key：命令关键字
     * @param info：获取的设备信息
     * @return 信息是否有效
     */
    bool getInfo(const QString &key, QString &info);

    /**
     * @brief refreshInfo 用来通知后台刷新信息
     */
    void refreshInfo();

protected:
    DBusInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();

private:
    static std::atomic<DBusInterface *> s_Instance;
    static std::mutex m_mutex;

    QDBusInterface       *mp_Iface;
};

#endif // DBUSINTERFACE_H
