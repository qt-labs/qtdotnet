/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "embeddedwindow.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QWindow>

#include "mainwindow.h"

EmbeddedWindow::EmbeddedWindow(QQmlEngine *qmlEngine, MainWindow *mainWindow)
    : qmlEngine(qmlEngine), mainWindow(mainWindow)
{
    connect(mainWindow, &MainWindow::contentRendered, this, &EmbeddedWindow::show);
    connect(mainWindow, &MainWindow::closed, this, &EmbeddedWindow::close);
}

EmbeddedWindow::~EmbeddedWindow()
{
    delete quickView;
}

void EmbeddedWindow::show()
{
    embeddedWindow = QWindow::fromWinId((WId)mainWindow->hostHandle());
    quickView = new QQuickView(qmlEngine, embeddedWindow);
    qmlEngine->rootContext()->setContextProperty("window", quickView);
    quickView->setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    quickView->show();
}

void EmbeddedWindow::close()
{
    embeddedWindow->close();
}
