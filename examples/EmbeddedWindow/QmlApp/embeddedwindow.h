/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include <QObject>

class QQmlEngine;
class QQuickView;
class QWindow;
class MainWindow;

class EmbeddedWindow : public QObject
{
    Q_OBJECT
public:
    EmbeddedWindow(QQmlEngine *qmlEngine, MainWindow *mainWindow);
    ~EmbeddedWindow();

public slots:
    void show();
    void close();

private:
    QQmlEngine *qmlEngine = nullptr;
    QQuickView *quickView = nullptr;
    MainWindow *mainWindow = nullptr;
    QWindow *embeddedWindow = nullptr;
};
