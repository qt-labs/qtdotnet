/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qchronometer.h"

#include <QObject>

struct QLapRecorderPrivate;

struct ILapRecorder : QDotNetInterface
{
    static inline const QString& FullyQualifiedTypeName =
        QStringLiteral("WatchModels.ILapRecorder, ChronometerModel");
    ILapRecorder();

    virtual void mark(int hours, int minutes, int seconds, int milliseconds) = 0;
    virtual void reset() = 0;
};

class QLapRecorder : public QObject, public ILapRecorder
{
    Q_OBJECT
    Q_PROPERTY(int lastHours READ lastHours NOTIFY lastHoursChanged)
    Q_PROPERTY(int lastMinutes READ lastMinutes NOTIFY lastMinutesChanged)
    Q_PROPERTY(int lastSeconds READ lastSeconds NOTIFY lastSecondsChanged)
    Q_PROPERTY(int lastMilliseconds READ lastMilliseconds NOTIFY lastMillisecondsChanged)
    Q_PROPERTY(int bestHours READ bestHours NOTIFY bestHoursChanged)
    Q_PROPERTY(int bestMinutes READ bestMinutes NOTIFY bestMinutesChanged)
    Q_PROPERTY(int bestSeconds READ bestSeconds NOTIFY bestSecondsChanged)
    Q_PROPERTY(int bestMilliseconds READ bestMilliseconds NOTIFY bestMillisecondsChanged)
    Q_PROPERTY(int lapCount READ lapCount NOTIFY lapCountChanged)

public:
    QLapRecorder(QObject *parent = nullptr);
    ~QLapRecorder() override;

    [[nodiscard]] int lapCount() const;
    [[nodiscard]] int lastHours() const;
    [[nodiscard]] int lastMinutes() const;
    [[nodiscard]] int lastSeconds() const;
    [[nodiscard]] int lastMilliseconds() const;
    [[nodiscard]] int bestHours() const;
    [[nodiscard]] int bestMinutes() const;
    [[nodiscard]] int bestSeconds() const;
    [[nodiscard]] int bestMilliseconds() const;

public slots:
    void mark(int hours, int minutes, int seconds, int milliseconds) override;
    void reset() override;

signals:
    void lapCountChanged();
    void lastHoursChanged();
    void lastMinutesChanged();
    void lastSecondsChanged();
    void lastMillisecondsChanged();
    void bestHoursChanged();
    void bestMinutesChanged();
    void bestSecondsChanged();
    void bestMillisecondsChanged();
    void newBestLap();

private:
    QLapRecorderPrivate *d = nullptr;
};
