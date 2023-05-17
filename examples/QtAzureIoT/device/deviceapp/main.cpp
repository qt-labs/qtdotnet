/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include <qdotnetobject.h>
#include <qdotnetevent.h>

class Backoffice : public QDotNetObject
{
public:
    Q_DOTNET_OBJECT_INLINE(Backoffice, "QtAzureIoT.Device.Backoffice, DeviceToBackoffice");
    Backoffice() : QDotNetObject(getConstructor<Backoffice>().invoke(nullptr))
    {}
    void setTelemetry(QString name, double value)
    {
        getMethod("SetTelemetry", fnSetTelemetryDouble).invoke(*this, name, value);
    }
    void setTelemetry(QString name, bool value)
    {
        getMethod("SetTelemetry", fnSetTelemetryBool).invoke(*this, name, value);
    }
public:
    void startPolling() {
        getMethod("StartPolling", fnStartPolling).invoke(*this);
    }
    void stopPolling() {
        getMethod("StopPolling", fnStopPolling).invoke(*this);
    }
private:
    mutable QDotNetFunction<void, QString, double> fnSetTelemetryDouble;
    mutable QDotNetFunction<void, QString, bool> fnSetTelemetryBool;
    mutable QDotNetFunction<void> fnStartPolling;
    mutable QDotNetFunction<void> fnStopPolling;
};

class SensorData : public QObject, public QDotNetObject, public QDotNetObject::IEventHandler
{
    Q_OBJECT
        Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
        Q_PROPERTY(double pressure READ pressure NOTIFY pressureChanged)
        Q_PROPERTY(double humidity READ humidity NOTIFY humidityChanged)
public:
    Q_DOTNET_OBJECT_INLINE(SensorData, "QtAzureIoT.Device.SensorData, SensorData");
    SensorData() : QDotNetObject(getConstructor<SensorData>().invoke(nullptr))
    {
        subscribeEvent("PropertyChanged", this);
    }
    double temperature() const
    {
        return getMethod("get_Temperature", fnGet_Temperature).invoke(*this);
    }
    double pressure() const
    {
        return getMethod("get_Pressure", fnGet_Pressure).invoke(*this);
    }
    double humidity() const
    {
        return getMethod("get_Humidity", fnGet_Humidity).invoke(*this);
    }
public slots:
    void startPolling() {
        getMethod("StartPolling", fnStartPolling).invoke(*this);
    }
    void stopPolling() {
        getMethod("StopPolling", fnStopPolling).invoke(*this);
    }
signals:
    void temperatureChanged();
    void pressureChanged();
    void humidityChanged();
private:
    void handleEvent(const QString& evName, QDotNetObject& evSrc, QDotNetObject& evArgs) override
    {
        if (evName == "PropertyChanged") {
            if (evArgs.type().fullName() == QDotNetPropertyEvent::FullyQualifiedTypeName) {
                auto propertyChangedEvent = evArgs.cast<QDotNetPropertyEvent>();
                if (propertyChangedEvent.propertyName() == "Temperature")
                    emit temperatureChanged();
                else if (propertyChangedEvent.propertyName() == "Pressure")
                    emit pressureChanged();
                else if (propertyChangedEvent.propertyName() == "Humidity")
                    emit humidityChanged();
            }
        }
    }
    mutable QDotNetFunction<double> fnGet_Temperature;
    mutable QDotNetFunction<double> fnGet_Pressure;
    mutable QDotNetFunction<double> fnGet_Humidity;
    mutable QDotNetFunction<void> fnStartPolling;
    mutable QDotNetFunction<void> fnStopPolling;
};

class CardReader : public QObject, public QDotNetObject, public QDotNetObject::IEventHandler
{
    Q_OBJECT
        Q_PROPERTY(bool cardInReader READ cardInReader NOTIFY cardInReaderChanged)
public:
    Q_DOTNET_OBJECT_INLINE(CardReader, "QtAzureIoT.Device.CardReader, CardReader");
    CardReader() : QDotNetObject(getConstructor<CardReader>().invoke(nullptr))
    {
        subscribeEvent("PropertyChanged", this);
    }
    bool cardInReader() const
    {
        return getMethod("get_CardInReader", fnGet_CardInReader).invoke(*this);
    }
public slots:
    void startPolling() {
        getMethod("StartPolling", fnStartPolling).invoke(*this);
    }
    void stopPolling() {
        getMethod("StopPolling", fnStopPolling).invoke(*this);
    }
signals:
    void cardInReaderChanged();
private:
    void handleEvent(const QString& evName, QDotNetObject& evSrc, QDotNetObject& evArgs) override
    {
        if (evName == "PropertyChanged") {
            if (evArgs.type().fullName() == QDotNetPropertyEvent::FullyQualifiedTypeName) {
                auto propertyChangedEvent = evArgs.cast<QDotNetPropertyEvent>();
                if (propertyChangedEvent.propertyName() == "CardInReader")
                    emit cardInReaderChanged();
            }
        }
    }
    mutable QDotNetFunction<bool> fnGet_CardInReader;
    mutable QDotNetFunction<void> fnStartPolling;
    mutable QDotNetFunction<void> fnStopPolling;
};

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    CardReader card;
    card.startPolling();
    engine.rootContext()->setContextProperty("card", &card);

    SensorData sensor;
    sensor.startPolling();
    engine.rootContext()->setContextProperty("sensor", &sensor);

    Backoffice backoffice;
    QObject::connect(&card, &CardReader::cardInReaderChanged,
        [&backoffice, &card]()
        {
            backoffice.setTelemetry("card", card.cardInReader());
        });

    QObject::connect(&sensor, &SensorData::temperatureChanged,
        [&backoffice, &sensor]()
        {
            backoffice.setTelemetry("temperature", sensor.temperature());
        });

    QObject::connect(&sensor, &SensorData::pressureChanged,
        [&backoffice, &sensor]()
        {
            backoffice.setTelemetry("pressure", sensor.pressure());
        });

    QObject::connect(&sensor, &SensorData::humidityChanged,
        [&backoffice, &sensor]()
        {
            backoffice.setTelemetry("humidity", sensor.humidity());
        });
    backoffice.startPolling();

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

#include "main.moc"
