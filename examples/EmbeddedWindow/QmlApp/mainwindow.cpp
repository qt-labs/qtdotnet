/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "mainwindow.h"

class HwndHost : public QDotNetObject
{
public:
    Q_DOTNET_OBJECT_INLINE(HwndHost, "System.Windows.Interop.HwndHost, PresentationFramework");
    void *handle() { return method("get_Handle", fnGetHandle).invoke(*this); }
    double width() { return method("get_Width", fnGetWidth).invoke(*this); }
    double height() { return method("get_Height", fnGetHeight).invoke(*this); }
private:
    QDotNetFunction<void *> fnGetHandle = nullptr;
    QDotNetFunction<double> fnGetWidth = nullptr;
    QDotNetFunction<double> fnGetHeight = nullptr;
};

class MouseEventArgs : public QDotNetObject
{
public:
    Q_DOTNET_OBJECT_INLINE(MouseEventArgs, "System.Windows.Input.MouseEventArgs, PresentationCore");
};

class MainWindowPrivate : public QDotNetObject::IEventHandler
{
public:
    MainWindowPrivate(MainWindow *q) : q(q)
    {}
    void handleEvent(const QString &evName, QDotNetObject &evSource, QDotNetObject &evArgs) override
    {
        if (evName == "ContentRendered") {
            emit q->contentRendered();
        } else if (evName == "SizeChanged") {
            double width = evArgs.object("NewSize").call<double>("get_Width");
            double height = evArgs.object("NewSize").call<double>("get_Height");
            if (width != hostWidth) {
                hostWidth = width;
                emit q->hostWidthChanged();
            }
            if (height != hostHeight) {
                hostHeight = height;
                emit q->hostHeightChanged();
            }
        } else if (evName == "Closed") {
            emit q->closed();
        } else if (evName == "PropertyChanged") {
            QString propertyName = evArgs.call<QString>("get_PropertyName");
            if (propertyName == "CameraPositionX")
                emit q->cameraPositionXChanged();
            else if (propertyName == "CameraPositionY")
                emit q->cameraPositionYChanged();
            else if (propertyName == "CameraPositionZ")
                emit q->cameraPositionZChanged();
            else if (propertyName == "CameraRotationX")
                emit q->cameraRotationXChanged();
            else if (propertyName == "CameraRotationY")
                emit q->cameraRotationYChanged();
            else if (propertyName == "CameraRotationZ")
                emit q->cameraRotationZChanged();
            else if (propertyName == "AnimationDuration")
                emit q->animationDurationChanged();
        }
    };

    HwndHost hwndHost = nullptr;
    double hostWidth = 0.0, hostHeight = 0.0;
    QDotNetFunction<void, double> fnSetEmbeddedFps = nullptr;
    QDotNetFunction<double> fnGetCameraPositionX = nullptr;
    QDotNetFunction<double> fnGetCameraPositionY = nullptr;
    QDotNetFunction<double> fnGetCameraPositionZ = nullptr;
    QDotNetFunction<double> fnGetCameraRotationX = nullptr;
    QDotNetFunction<double> fnGetCameraRotationY = nullptr;
    QDotNetFunction<double> fnGetCameraRotationZ = nullptr;
    QDotNetFunction<double> fnGetAnimationDuration = nullptr;
    QDotNetFunction<double> fnGetFramesPerSecond = nullptr;
    QDotNetFunction<QString> fnGetBackgroundColor = nullptr;

private:
    MainWindow *q;
};

Q_DOTNET_OBJECT_IMPL(MainWindow, Q_DOTNET_OBJECT_INIT(d(new MainWindowPrivate(this))));

MainWindow::MainWindow() : QDotNetObject(nullptr), d(new MainWindowPrivate(this))
{}

MainWindow::~MainWindow()
{}

void MainWindow::init()
{
    *this = constructor<MainWindow>().invoke(nullptr);
    d->hwndHost = method<HwndHost>("get_HwndHost").invoke(*this);
    subscribeEvent("ContentRendered", d);
    subscribeEvent("Closed", d);
    subscribeEvent("PropertyChanged", d);
    d->hwndHost.subscribeEvent("SizeChanged", d);

    QtDotNet::call<void, MainWindow>("WpfApp.Program, WpfApp", "set_MainWindow", *this);
    QtDotNet::call<int>("WpfApp.Program, WpfApp", "Main");
}

void *MainWindow::hostHandle()
{
    return d->hwndHost.handle();
}

int MainWindow::hostWidth()
{
    return d->hostWidth;
}

int MainWindow::hostHeight()
{
    return d->hostHeight;
}


double MainWindow::cameraPositionX()
{
    return method("get_CameraPositionX", d->fnGetCameraPositionX).invoke(*this);
}

double MainWindow::cameraPositionY()
{
    return method("get_CameraPositionY", d->fnGetCameraPositionY).invoke(*this);
}

double MainWindow::cameraPositionZ()
{
    return method("get_CameraPositionZ", d->fnGetCameraPositionZ).invoke(*this);
}

double MainWindow::cameraRotationX()
{
    return method("get_CameraRotationX", d->fnGetCameraRotationX).invoke(*this);
}

double MainWindow::cameraRotationY()
{
    return method("get_CameraRotationY", d->fnGetCameraRotationY).invoke(*this);
}

double MainWindow::cameraRotationZ()
{
    return method("get_CameraRotationZ", d->fnGetCameraRotationZ).invoke(*this);
}

double MainWindow::animationDuration()
{
    return method("get_AnimationDuration", d->fnGetAnimationDuration).invoke(*this);
}

double MainWindow::framesPerSecond()
{
    return method("get_FramesPerSecond", d->fnGetFramesPerSecond).invoke(*this);
}

QString MainWindow::backgroundColor()
{
    return method("get_BackgroundColor", d->fnGetBackgroundColor).invoke(*this);
}

void MainWindow::setFramesPerSecond(double fps)
{
    method("set_FramesPerSecond", d->fnSetEmbeddedFps).invoke(*this, fps);
}
