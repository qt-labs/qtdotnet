/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "qchronometer.h"
#include "qlaprecorder.h"

#include <qdotnetevent.h>

struct QChronometerPrivate : QDotNetObject::IEventHandler
{
    QChronometerPrivate(QChronometer *q)
        :q(q)
    {}

    void handleEvent(const QString &eventName, QDotNetObject &sender, QDotNetObject &args) override
    {
        if (eventName != "PropertyChanged")
            return;

        if (args.type().fullName() != QDotNetPropertyEvent::FullyQualifiedTypeName)
            return;

        const auto propertyChangedEvent = args.cast<QDotNetPropertyEvent>();
        if (propertyChangedEvent.propertyName() == "Hours")
            emit q->hoursChanged();
        else if (propertyChangedEvent.propertyName() == "Minutes")
            emit q->minutesChanged();
        else if (propertyChangedEvent.propertyName() == "Seconds")
            emit q->secondsChanged();
        else if (propertyChangedEvent.propertyName() == "Day")
            emit q->dayChanged();
        else if (propertyChangedEvent.propertyName() == "Started")
            emit q->startedChanged();
        else if (propertyChangedEvent.propertyName() == "ElapsedHours")
            emit q->elapsedHoursChanged();
        else if (propertyChangedEvent.propertyName() == "ElapsedMinutes")
            emit q->elapsedMinutesChanged();
        else if (propertyChangedEvent.propertyName() == "ElapsedSeconds")
            emit q->elapsedSecondsChanged();
        else if (propertyChangedEvent.propertyName() == "ElapsedMilliseconds")
            emit q->elapsedMillisecondsChanged();
        else if (propertyChangedEvent.propertyName() == "AdjustDayMode")
            emit q->adjustDayModeChanged();
        else if (propertyChangedEvent.propertyName() == "AdjustTimeMode")
            emit q->adjustTimeModeChanged();
    }

    QChronometer *q = nullptr;

    QDotNetFunction<double> hours= nullptr;
    QDotNetFunction<double> minutes = nullptr;
    QDotNetFunction<double> seconds = nullptr;
    QDotNetFunction<int> day = nullptr;
    QDotNetFunction<double> elapsedHours = nullptr;
    QDotNetFunction<double> elapsedMinutes = nullptr;
    QDotNetFunction<double> elapsedSeconds = nullptr;
    QDotNetFunction<double> elapsedMilliseconds = nullptr;
    QDotNetFunction<bool> started, adjustDayMode, adjustTimeMode = nullptr;
    QDotNetFunction<void> startStop, reset, breakWatch = nullptr;
    QDotNetFunction<void, bool> setAdjustDayMode, setAdjustTimeMode = nullptr;
    QDotNetFunction<void, int> adjust = nullptr;
};


Q_DOTNET_OBJECT_IMPL(QChronometer, Q_DOTNET_OBJECT_INIT(d(new QChronometerPrivate(this))));


QChronometer::QChronometer(const ILapRecorder &lapRecorder)
    : d(new QChronometerPrivate(this))
{
    *this = constructor<QChronometer>().invoke(nullptr);
    method<void, ILapRecorder>("set_LapRecorder").invoke(*this, lapRecorder);
    subscribeEvent("PropertyChanged", d);
}

QChronometer::~QChronometer()
{
    if (isValid())
        unsubscribeEvent("PropertyChanged", d);
    delete d;
}

double QChronometer::hours() const
{
    return method("get_Hours", d->hours).invoke(*this);
}

double QChronometer::minutes() const
{
    return method("get_Minutes", d->minutes).invoke(*this);
}

double QChronometer::seconds() const
{
    return method("get_Seconds", d->seconds).invoke(*this);
}

int QChronometer::day() const
{
    return method("get_Day", d->day).invoke(*this);
}

bool QChronometer::started() const
{
    return method("get_Started", d->started).invoke(*this);
}

double QChronometer::elapsedHours() const
{
    return method("get_ElapsedHours", d->elapsedHours).invoke(*this);
}

double QChronometer::elapsedMinutes() const
{
    return method("get_ElapsedMinutes", d->elapsedMinutes).invoke(*this);
}

double QChronometer::elapsedSeconds() const
{
    return method("get_ElapsedSeconds", d->elapsedSeconds).invoke(*this);
}

double QChronometer::elapsedMilliseconds() const
{
    return method("get_ElapsedMilliseconds", d->elapsedMilliseconds).invoke(*this);
}

bool QChronometer::adjustDayMode() const
{
    return method("get_AdjustDayMode", d->adjustDayMode).invoke(*this);
}

void QChronometer::setAdjustDayMode(bool value)
{
    method("set_AdjustDayMode", d->setAdjustDayMode).invoke(*this, value);
}

bool QChronometer::adjustTimeMode() const
{
    return method("get_AdjustTimeMode", d->adjustTimeMode).invoke(*this);
}

void QChronometer::setAdjustTimeMode(bool value)
{
    method("set_AdjustTimeMode", d->setAdjustTimeMode).invoke(*this, value);
}

void QChronometer::adjust(int delta)
{
    method("Adjust", d->adjust).invoke(*this, delta);
}

void QChronometer::startStop()
{
    method("StartStop", d->startStop).invoke(*this);
}

void QChronometer::reset()
{
    method("Reset", d->reset).invoke(*this);
}
