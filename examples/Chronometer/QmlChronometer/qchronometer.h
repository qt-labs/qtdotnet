/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include <qdotnetinterface.h>
#include <qdotnetobject.h>

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QObject>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

struct ILapRecorder;
struct QChronometerPrivate;

class QChronometer : public QObject, public QDotNetObject
{
    Q_OBJECT
    Q_PROPERTY(double hours READ hours NOTIFY hoursChanged)
    Q_PROPERTY(double minutes READ minutes NOTIFY minutesChanged)
    Q_PROPERTY(double seconds READ seconds NOTIFY secondsChanged)
    Q_PROPERTY(int day READ day NOTIFY dayChanged)
    Q_PROPERTY(bool started READ started NOTIFY startedChanged)
    Q_PROPERTY(double elapsedHours READ elapsedHours NOTIFY elapsedHoursChanged)
    Q_PROPERTY(double elapsedMinutes READ elapsedMinutes NOTIFY elapsedMinutesChanged)
    Q_PROPERTY(double elapsedSeconds READ elapsedSeconds NOTIFY elapsedSecondsChanged)
    Q_PROPERTY(double elapsedMilliseconds
        READ elapsedMilliseconds NOTIFY elapsedMillisecondsChanged)
    Q_PROPERTY(bool adjustDayMode
        READ adjustDayMode WRITE setAdjustDayMode NOTIFY adjustDayModeChanged)
    Q_PROPERTY(bool adjustTimeMode
        READ adjustTimeMode WRITE setAdjustTimeMode NOTIFY adjustTimeModeChanged)

public:
    Q_DOTNET_OBJECT(QChronometer, "WatchModels.Chronometer, ChronometerModel");

    QChronometer(const ILapRecorder &lapRecorder);
    ~QChronometer() override;

    double hours() const;
    double minutes() const;
    double seconds() const;
    int day() const;
    bool started() const;
    double elapsedHours() const;
    double elapsedMinutes() const;
    double elapsedSeconds() const;
    double elapsedMilliseconds() const;
    bool adjustDayMode() const;
    bool adjustTimeMode() const;

public slots:
    void startStop();
    void reset();
    void setAdjustDayMode(bool value);
    void setAdjustTimeMode(bool value);
    void adjust(int delta);


signals:
    void hoursChanged();
    void minutesChanged();
    void secondsChanged();
    void dayChanged();
    void startedChanged();
    void elapsedHoursChanged();
    void elapsedMinutesChanged();
    void elapsedSecondsChanged();
    void elapsedMillisecondsChanged();
    void adjustDayModeChanged();
    void adjustTimeModeChanged();
    void lap(int hours, int minutes, int seconds, int milliseconds);

private:
    QChronometerPrivate *d = nullptr;
};
