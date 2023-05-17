/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "qlaprecorder.h"

struct QLapRecorderPrivate
{
    QLapRecorderPrivate() = default;

    int lapCount = 0;
    int lastHours = 0;
    int lastMinutes = 0;
    int lastSeconds = 0;
    int lastMilliseconds = 0;
    int bestHours = 0;
    int bestMinutes = 0;
    int bestSeconds = 0;
    int bestMilliseconds = 0;
};


ILapRecorder::ILapRecorder() : QDotNetInterface(FullyQualifiedTypeName)
{
    setCallback<void, int, int, int, int>(
        "Mark", [this](int hours, int minutes, int seconds, int milliseconds)
        {
            mark(hours, minutes, seconds, milliseconds);
        });
}


QLapRecorder::QLapRecorder(QObject *parent)
    : QObject(parent), d(new QLapRecorderPrivate())
{}

QLapRecorder::~QLapRecorder()
{
    delete d;
}

int QLapRecorder::lapCount() const { return d->lapCount; }
int QLapRecorder::lastHours() const { return d->lastHours; }
int QLapRecorder::lastMinutes() const { return d->lastMinutes; }
int QLapRecorder::lastSeconds() const { return d->lastSeconds; }
int QLapRecorder::lastMilliseconds() const { return d->lastMilliseconds; }
int QLapRecorder::bestHours() const { return d->bestHours; }
int QLapRecorder::bestMinutes() const { return d->bestMinutes; }
int QLapRecorder::bestSeconds() const { return d->bestSeconds; }
int QLapRecorder::bestMilliseconds() const { return d->bestMilliseconds; }

void QLapRecorder::mark(int hours, int minutes, int seconds, int milliseconds)
{
    d->lastHours = hours;
    emit lastHoursChanged();

    d->lastMinutes = minutes;
    emit lastMinutesChanged();

    d->lastSeconds = seconds;
    emit lastSecondsChanged();

    d->lastMilliseconds = milliseconds;
    emit lastMillisecondsChanged();

    d->lapCount++;
    emit lapCountChanged();

    if (d->lapCount > 1
        && (d->lastHours > d->bestHours
            || (d->lastHours == d->bestHours
                && d->lastMinutes > d->bestMinutes)
            || (d->lastHours == d->bestHours
                && d->lastMinutes == d->bestMinutes
                && d->lastSeconds > d->bestSeconds)
            || (d->lastHours == d->bestHours
                && d->lastMinutes == d->bestMinutes
                && d->lastSeconds == d->bestSeconds
                && d->lastMilliseconds > d->bestMilliseconds))) {
        return;
    }

    d->bestHours = hours;
    emit bestHoursChanged();

    d->bestMinutes = minutes;
    emit bestMinutesChanged();

    d->bestSeconds = seconds;
    emit bestSecondsChanged();

    d->bestMilliseconds = milliseconds;
    emit bestMillisecondsChanged();

    if (d->lapCount > 1)
        emit newBestLap();
}

void QLapRecorder::reset()
{
    d->lastHours = 0;
    emit lastHoursChanged();

    d->lastMinutes = 0;
    emit lastMinutesChanged();

    d->lastSeconds = 0;
    emit lastSecondsChanged();

    d->lastMilliseconds = 0;
    emit lastMillisecondsChanged();

    d->lapCount = 0;
    emit lapCountChanged();

    d->bestHours = 0;
    emit bestHoursChanged();

    d->bestMinutes = 0;
    emit bestMinutesChanged();

    d->bestSeconds = 0;
    emit bestSecondsChanged();

    d->bestMilliseconds = 0;
    emit bestMillisecondsChanged();
}
