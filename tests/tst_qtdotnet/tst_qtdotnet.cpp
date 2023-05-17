/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "foo.h"
#include "stringbuilder.h"
#include "uri.h"

#include <qdotnetadapter.h>
#include <qdotnetarray.h>
#include <qdotnetcallback.h>
#include <qdotnethost.h>
#include <qdotnetmarshal.h>
#include <qdotnetobject.h>
#include <qdotnetsafemethod.h>
#include <qdotnettype.h>

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QChar>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSignalSpy>
#include <QString>

#include <QtTest>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

class tst_qtdotnet : public QObject
{
    Q_OBJECT

public:
    tst_qtdotnet() = default;

private slots:
    void loadHost();
    void runtimeProperties();
    void resolveFunction();
    void callFunction();
    void callFunctionWithCustomMarshaling();
    void callDefaultEntryPoint();
    void callWithComplexArg();
    void adapterInit();
    void callStaticMethod();
    void handleException();
    void createObject();
    void callInstanceMethod();
    void useWrapperClass();
    void emitSignalFromEvent();
    void propertyBinding();
    void implementInterface();
    void arrayOfInts();
    void arrayOfStrings();
    void arrayOfObjects();
    void unloadHost();
};

QDotNetHost dotNetHost;

void tst_qtdotnet::loadHost()
{
    QVERIFY(!dotNetHost.isLoaded());
    QVERIFY(dotNetHost.load());
    QVERIFY(dotNetHost.isLoaded());
}

void tst_qtdotnet::runtimeProperties()
{
    QVERIFY(dotNetHost.isLoaded());
    QMap<QString, QString> runtimeProperties = dotNetHost.runtimeProperties();
    QVERIFY(!runtimeProperties.isEmpty());
    for (auto prop = runtimeProperties.constBegin(); prop != runtimeProperties.constEnd(); ++prop) {
        qInfo() << prop.key() << "=" << QString("%1%2")
            .arg(prop.value().left(100)).arg(prop.value().length() > 100 ? "..." : "");
    }
}

QDotNetFunction<QString, QString, int> formatNumber;

void tst_qtdotnet::resolveFunction()
{
    QVERIFY(dotNetHost.isLoaded());
    QVERIFY(!formatNumber.isValid());
    QVERIFY(dotNetHost.resolveFunction(formatNumber,
        QDir(QCoreApplication::applicationDirPath()).filePath("FooLib.dll"),
        Foo::FullyQualifiedTypeName, "FormatNumber", "FooLib.Foo+FormatNumberDelegate, FooLib"));

    QVERIFY(formatNumber.isValid());
}

void tst_qtdotnet::callFunction()
{
    QVERIFY(dotNetHost.isLoaded());
    QVERIFY(formatNumber.isValid());

    const QString formattedText = formatNumber("[{0}]", 42);

    QCOMPARE(formattedText, "[42]");
}

struct DoubleAsInt {};

template<>
struct QDotNetOutbound<DoubleAsInt>
{
    using SourceType = double;
    using OutboundType = int;
    static OutboundType convert(SourceType arg)
    {
        return qRound(arg);
    }
};

struct QUpperCaseString
{};

template<>
struct QDotNetNull<QUpperCaseString>
{
    static QString value() { return {}; }
    static bool isNull(const QString& s) { return  s.isNull() || s.isEmpty(); }
};

template<>
struct QDotNetInbound<QUpperCaseString>
{
    using InboundType = QChar*;
    using TargetType = QString;
    static TargetType convert(InboundType inboundValue)
    {
        return QString(inboundValue).toUpper();
    }
};

void tst_qtdotnet::callFunctionWithCustomMarshaling()
{
    QVERIFY(dotNetHost.isLoaded());

    QDotNetFunction<QUpperCaseString, QString, DoubleAsInt> formatDouble;
    QVERIFY(dotNetHost.resolveFunction(formatDouble,
        QDir(QCoreApplication::applicationDirPath()).filePath("FooLib.dll"),
        Foo::FullyQualifiedTypeName, "FormatNumber", "FooLib.Foo+FormatNumberDelegate, FooLib"));

    QVERIFY(formatDouble.isValid());

    const QString formattedText = formatDouble("result = [{0}]", 41.5);

    QCOMPARE(formattedText, "RESULT = [42]");
}

void tst_qtdotnet::callDefaultEntryPoint()
{
    QVERIFY(dotNetHost.isLoaded());

    QDotNetFunction<quint32, void*, qint32> entryPoint;
    QVERIFY(dotNetHost.resolveFunction(entryPoint,
        QDir(QCoreApplication::applicationDirPath()).filePath("FooLib.dll"),
        Foo::FullyQualifiedTypeName, "EntryPoint"));

    QVERIFY(entryPoint.isValid());

    QString fortyTwo("42");
    const qint32 returnValue = entryPoint(fortyTwo.data(), static_cast<qint32>(fortyTwo.length()));

    QCOMPARE(returnValue, 42);
}

struct Date
{
    QString year;
    QString month;
    QString day;
};

struct DateOutbound
{
    const QChar* year;
    const QChar* month;
    const QChar* day;
};

template<>
struct QDotNetOutbound<Date>
{
    using SourceType = const Date&;
    using OutboundType = const DateOutbound;
    static DateOutbound convert(SourceType arg)
    {
        return { arg.year.data(), arg.month.data(), arg.day.data() };
    }
};

void tst_qtdotnet::callWithComplexArg()
{
    QVERIFY(dotNetHost.isLoaded());
    QDotNetFunction<QString, QString, Date> formatDate;
    QVERIFY(dotNetHost.resolveFunction(formatDate,
        QDir(QCoreApplication::applicationDirPath()).filePath("FooLib.dll"),
        Foo::FullyQualifiedTypeName, "FormatDate", "FooLib.Foo+FormatDateDelegate, FooLib"));

    QVERIFY(formatDate.isValid());

    const Date xmas{ "2022", "12", "25" };
    const QString formattedText = formatDate("Today is {0}-{1}-{2}", xmas);

    QCOMPARE(formattedText, "Today is 2022-12-25");
}

void tst_qtdotnet::adapterInit()
{
    QVERIFY(!QDotNetAdapter::instance().isValid());
    QDotNetAdapter::instance().init(
        QDir(QCoreApplication::applicationDirPath()).filePath("Qt.DotNet.Adapter.dll"),
        "Qt.DotNet.Adapter", "Qt.DotNet.Adapter", &dotNetHost);
    QVERIFY(QDotNetAdapter::instance().isValid());
}

void tst_qtdotnet::callStaticMethod()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        const QDotNetType environment = QDotNetType::find("System.Environment");
        const auto getEnvironmentVariable
            = environment.staticMethod<QString, QString>("GetEnvironmentVariable");
        const QString path = getEnvironmentVariable("PATH");
        QVERIFY(path.length() > 0);
        const QString samePath = QtDotNet::call<QString, QString>(
            "System.Environment", "GetEnvironmentVariable", "PATH");
        QVERIFY(path == samePath);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::createObject()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        const auto newStringBuilder = QDotNetObject::constructor("System.Text.StringBuilder");
        QDotNetObject stringBuilder = newStringBuilder();
        QVERIFY(QDotNetAdapter::instance().stats().refCount == 1);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::callInstanceMethod()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        const auto newStringBuilder = QDotNetObject::constructor("System.Text.StringBuilder");
        const auto stringBuilder = newStringBuilder();
        const auto append = stringBuilder.method<QDotNetObject, QString>("Append");
        std::ignore = append("Hello");
        std::ignore = append(" World!");
        const QString helloWorld = stringBuilder.toString();
        QVERIFY(helloWorld == "Hello World!");
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::useWrapperClass()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        StringBuilder sb;
        QVERIFY(QDotNetAdapter::instance().stats().refCount == 1);
        QVERIFY(sb.isValid());
        sb.append("Hello").append(" ");
        StringBuilder sbCpy(sb);
        QVERIFY(QDotNetAdapter::instance().stats().refCount == 2);
        QVERIFY(sbCpy.isValid());
        sbCpy.append("World");
        sb = StringBuilder(std::move(sbCpy));
        QVERIFY(QDotNetAdapter::instance().stats().refCount == 1);
        sb.append("!");
        QCOMPARE(sb.toString(), "Hello World!");
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        const Uri uri(QStringLiteral(
            "https://user:password@www.contoso.com:80/Home/Index.htm?q1=v1&q2=v2#FragmentName"));
        QVERIFY(uri.segments().length() == 3);
        QVERIFY(uri.segments()[0]->compare("/") == 0);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::handleException()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        StringBuilder stringBuilder(5, 5);
        QString helloWorld;
        try {
            stringBuilder.append("Hello");
            QVERIFY(stringBuilder.toString() == "Hello");
            stringBuilder.append(" World!");
            helloWorld = stringBuilder.toString();
        }
        catch (QDotNetException&) {
            helloWorld = "<ERROR>";
        }
        QVERIFY(helloWorld == "<ERROR>");
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

class Ping final : public QObject, public QDotNetObject, public QDotNetObject::IEventHandler
{
    Q_OBJECT

public:
    Q_DOTNET_OBJECT_INLINE(Ping, "System.Net.NetworkInformation.Ping, System", );

    Ping()
        : QDotNetObject(QDotNetSafeMethod(constructor<Ping>()).invoke(nullptr))
    {
        subscribeEvent("PingCompleted", this);
    }
    ~Ping() override = default;

    void sendAsync(const QString& hostNameOrAddress)
    {
        method("SendAsync", safeSendAsync).invoke(*this, hostNameOrAddress, nullptr);
    }

    void sendAsyncCancel()
    {
        method("SendAsyncCancel", safeSendAsyncCancel).invoke(*this);
    }

signals:
    void pingCompleted(const QString& address, qint64 roundtripMsecs);
    void pingError();

private:
    void handleEvent(const QString& evName, QDotNetObject& evSrc, QDotNetObject& evArgs) override
    {
        if (evName != "PingCompleted")
            return;
        if (evArgs.type().fullName() != "System.Net.NetworkInformation.PingCompletedEventArgs")
            return;
        const auto getReply = evArgs.method<QDotNetObject>("get_Reply");
        const auto reply = getReply();
        if (reply.isValid()) {
            const auto replyAddress = reply.method<QDotNetObject>("get_Address");
            const auto replyRoundtrip = reply.method<qint64>("get_RoundtripTime");
            emit pingCompleted(replyAddress().toString(), replyRoundtrip());
        }
        else {
            emit pingError();
        }
    }
    QDotNetSafeMethod<void, QString, QtDotNet::Null> safeSendAsync;
    QDotNetSafeMethod<void> safeSendAsyncCancel;
};

void tst_qtdotnet::emitSignalFromEvent()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        Ping ping;
        bool waiting = true;
        int signalCount = 0;
        connect(&ping, &Ping::pingCompleted,
            [&waiting, &signalCount](const QString& address, qint64 roundtripMsecs) {
                qInfo() << "Reply from" << address << "in" << roundtripMsecs << "msecs";
        signalCount++;
        waiting = false;
            });
        connect(&ping, &Ping::pingError,
            [&waiting, &signalCount] {
                qInfo() << "Ping error";
        signalCount++;
        waiting = false;
            });
        qInfo() << "Pinging www.qt.io:";
        QElapsedTimer waitTime;
        for (int i = 0; i < 4; ++i) {
            waitTime.restart();
            waiting = true;
            ping.sendAsync("www.qt.io");
            while (waiting) {
                QCoreApplication::processEvents();
                if (waitTime.elapsed() > 3000) {
                    ping.sendAsyncCancel();
                    waiting = false;
                    qInfo() << "Ping timeout";
                }
            }
        }
        QVERIFY(signalCount == 4);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::propertyBinding()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        Foo foo;
        const QSignalSpy spy(&foo, &Foo::barChanged);
        for (int i = 0; i < 1000; ++i)
            foo.setBar(QString("hello x %1").arg(i + 1));
        QVERIFY(foo.bar() == "hello x 1000");
        QVERIFY(spy.count() == 1000);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

struct ToUpper : IBarTransformation
{
    QString transform(const QString& bar) override
    {
        return bar.toUpper();
    }
};

void tst_qtdotnet::implementInterface()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        const ToUpper transfToUpper;
        Foo foo(transfToUpper);
        const QSignalSpy spy(&foo, &Foo::barChanged);
        for (int i = 0; i < 1000; ++i)
            foo.setBar(QString("hello x %1").arg(i + 1));
        QVERIFY(foo.bar() == "HELLO X 1000");
        QVERIFY(spy.count() == 1000);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::arrayOfInts()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        QDotNetArray<qint32> a(11);
        a[0] = 0;
        a[1] = 1;
        for (int i = 2; i < a.length(); ++i)
            a[i] = a[i - 1] + a[i - 2];
        QVERIFY(a[10] == 55);
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::arrayOfStrings()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        QDotNetArray<QString> a(8);
        a[0] = "Lorem";
        a[1] = "ipsum";
        a[2] = "dolor";
        a[3] = "sit";
        a[4] = "amet,";
        a[5] = "consectetur";
        a[6] = "adipiscing";
        a[7] = "elit.";
        const auto stringType = QDotNetType::find("System.String");
        const auto join = stringType.staticMethod<QString, QString, QDotNetArray<QString>>("Join");
        const auto loremIpsum = join(" ", a);
        QVERIFY(loremIpsum == "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::arrayOfObjects()
{
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
    {
        QDotNetArray<StringBuilder> a(8);
        for (int i = 0; i < a.length(); ++i)
            a[i] = StringBuilder();
        a[0]->append("Lorem");
        a[1]->append(a[0]->toString()).append(" ipsum");
        a[2]->append(a[1]->toString()).append(" dolor");
        a[3]->append(a[2]->toString()).append(" sit");
        a[4]->append(a[3]->toString()).append(" amet,");
        a[5]->append(a[4]->toString()).append(" consectetur");
        a[6]->append(a[5]->toString()).append(" adipiscing");
        a[7]->append(a[6]->toString()).append(" elit.");
        QVERIFY(a[7]->toString() == "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    }
    QVERIFY(QDotNetAdapter::instance().stats().refCount == 0);
}

void tst_qtdotnet::unloadHost()
{
    QVERIFY(dotNetHost.isLoaded());

    dotNetHost.unload();

    QVERIFY(!dotNetHost.isLoaded());
}

QTEST_MAIN(tst_qtdotnet)
#include "tst_qtdotnet.moc"
