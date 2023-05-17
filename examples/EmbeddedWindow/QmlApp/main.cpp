/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>
#include <QFile>

#include <objbase.h>

#include "mainwindow.h"
#include "embeddedwindow.h"

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    MainWindow mainWindow;
    QObject::connect(&mainWindow, &MainWindow::closed, &app, &QCoreApplication::quit);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);

    EmbeddedWindow embeddedWindow(&engine, &mainWindow);

    QThread *wpfThread = QThread::create([&app, &mainWindow] {

        if (FAILED(CoInitialize(nullptr))) {
            app.quit();
            return;
        }

        QString runtimeConfig = R"[json](
{
  "runtimeOptions": {
    "tfm": "net6.0-windows",
    "rollForward": "LatestMinor",
    "framework": {
      "name": "Microsoft.WindowsDesktop.App",
      "version": "6.0.0"
    }
  }
}
)[json]";
        QFile wpfAppRuntimeConfig(QGuiApplication::applicationDirPath() + "/WpfApp.runtimeconfig.json");
        if (wpfAppRuntimeConfig.open(QFile::ReadOnly | QFile::Text))
            runtimeConfig = QString(wpfAppRuntimeConfig.readAll());

        QDotNetHost host;
        if (!host.load(runtimeConfig)) {
            app.quit();
            return;
        }
        QDotNetAdapter::init(&host);
        mainWindow.init();
    });
    wpfThread->start();
    return app.exec();
}
