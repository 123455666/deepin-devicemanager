/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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

#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DLog>
#include <stdio.h>
#include <stdlib.h>
#include "DTitlebar"
#include "DApplicationSettings"
#include "deviceinfoparser.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();
    DApplication app(argc, argv);

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-devicemanager");
    app.setApplicationDisplayName(DApplication::translate("Main", "Device Manager"));

    //app.setApplicationVersion(VERSION);
    app.setApplicationVersion(DApplication::buildVersion("3.0"));

    QIcon appIcon = QIcon::fromTheme("deepin-devicemanager");

    app.setProductIcon(appIcon);
    app.setWindowIcon(appIcon);

    app.setProductName(DApplication::translate("Main", "Device Manager"));

    app.setApplicationDescription(DApplication::translate("Main", "Device Manager is a handy tool for viewing hardware information and managing the devices.") + "\n");

    const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
    app.setApplicationAcknowledgementPage(acknowledgementLink);

    DApplicationSettings settinAgs;

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    //app.setStyle("chameleon");
    //    if (!DGuiApplicationHelper::instance()->setSingleInstance("deepin-devicemanager"))
    //            return 0;

    MainWindow w(nullptr);
    w.titlebar()->setTitle("");
    w.titlebar()->setIcon(appIcon);

    w.show();
    Dtk::Widget::moveToCenter(&w);

    if(appIcon.isNull() || DeviceInfoParserInstance.getLsbRelease().contains("uos", Qt::CaseInsensitive) ==  false)
    {
        QIcon localIcon(":images/icons/deepin-devicemanager64.svg");
        app.setProductIcon(localIcon);
        app.setWindowIcon(localIcon);
        w.titlebar()->setIcon(localIcon);
    }

    return app.exec();
}
