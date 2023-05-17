/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include <QObject>
#include <QDotNetObject>

class MainWindowPrivate;

class MainWindow : public QObject, public QDotNetObject
{
    Q_OBJECT
    Q_PROPERTY(double hostWidth READ hostWidth NOTIFY hostWidthChanged)
    Q_PROPERTY(double hostHeight READ hostHeight NOTIFY hostHeightChanged)
    Q_PROPERTY(double cameraPositionX READ cameraPositionX NOTIFY cameraPositionXChanged)
    Q_PROPERTY(double cameraPositionY READ cameraPositionY NOTIFY cameraPositionYChanged)
    Q_PROPERTY(double cameraPositionZ READ cameraPositionZ NOTIFY cameraPositionZChanged)
    Q_PROPERTY(double cameraRotationX READ cameraRotationX NOTIFY cameraRotationXChanged)
    Q_PROPERTY(double cameraRotationY READ cameraRotationY NOTIFY cameraRotationYChanged)
    Q_PROPERTY(double cameraRotationZ READ cameraRotationZ NOTIFY cameraRotationZChanged)
    Q_PROPERTY(double animationDuration READ animationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(double framesPerSecond READ framesPerSecond WRITE setFramesPerSecond)
public:
    Q_DOTNET_OBJECT(MainWindow, "WpfApp.MainWindow, WpfApp");
    MainWindow();
    ~MainWindow();
    void init();
    void *hostHandle();
    int hostWidth();
    int hostHeight();
    double cameraPositionX();
    double cameraPositionY();
    double cameraPositionZ();
    double cameraRotationX();
    double cameraRotationY();
    double cameraRotationZ();
    double animationDuration();
    double framesPerSecond();
    QString backgroundColor();
signals:
    void contentRendered();
    void hostWidthChanged();
    void hostHeightChanged();
    void cameraPositionXChanged();
    void cameraPositionYChanged();
    void cameraPositionZChanged();
    void cameraRotationXChanged();
    void cameraRotationYChanged();
    void cameraRotationZChanged();
    void animationDurationChanged();
    void backgroundColorChanged();
    void closed();
public slots:
    void setFramesPerSecond(double fps);
private:
    MainWindowPrivate *d;
};
