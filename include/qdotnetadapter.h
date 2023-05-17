/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnethost.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QMutexLocker>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

class QDotNetRef;

class QDotNetAdapter final
{
private:
    QDotNetAdapter() = default;
    QDotNetAdapter(const QDotNetAdapter &) = delete;
    QDotNetAdapter(QDotNetAdapter &&) = delete;
    QDotNetAdapter &operator=(const QDotNetAdapter &) = delete;
    QDotNetAdapter &operator=(QDotNetAdapter &&) = delete;

    ~QDotNetAdapter()
    {
        defaultHost.unload();
    }

    static void init()
    {
        if (instance().isValid())
            return;
        init(QDir(QCoreApplication::applicationDirPath())
            .filePath(defaultDllName), defaultAssemblyName, defaultTypeName);
    }

public:
    static void init(QDotNetHost *externalHost)
    {
        if (instance().isValid())
            return;
        init(QDir(QCoreApplication::applicationDirPath())
            .filePath(defaultDllName), defaultAssemblyName, defaultTypeName, externalHost);
    }

    static void init(const QString &assemblyPath, const QString &assemblyName,
        const QString &typeName, QDotNetHost *externalHost = nullptr)
    {
        if (instance().isValid())
            return;

        const QString typeFullName = QString("%1, %2").arg(typeName, assemblyName);
        const QString delegateName = QString("%1+Delegates+%3, %2")
            .arg(typeName, assemblyName, "%1");

        QDotNetHost *host = nullptr;
        if (externalHost != nullptr)
            host = externalHost;
        else
            host = &instance().defaultHost;
        if (!host->load()) {
            qCritical() << "QDotNetAdapter: error loading host";
            return;
        }

#define QDOTNETADAPTER_DELEGATE(d) \
    instance().d, assemblyPath, typeFullName, \
    QString(#d).sliced(2), \
    delegateName.arg(QString(#d).sliced(2))

        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnLoadAssembly));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnResolveStaticMethod));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnResolveConstructor));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnResolveInstanceMethod));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnResolveSafeMethod));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnAddEventHandler));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnRemoveEventHandler));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnRemoveAllEventHandlers));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnAddObjectRef));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnFreeDelegateRef));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnFreeObjectRef));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnFreeTypeRef));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnAddInterfaceProxy));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnSetInterfaceMethod));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnStats));
        host->resolveFunction(QDOTNETADAPTER_DELEGATE(fnGetObject));

#undef QDOTNETADAPTER_DELEGATE

        instance().host = host;
    }

    static QDotNetAdapter &instance()
    {
        static QDotNetAdapter adapter;
        return adapter;
    }

    bool isValid() const { return host != nullptr; }

public:
    bool loadAssembly(const QString &assemblyName) const
    {
        init();
        return fnLoadAssembly(assemblyName);
    }

    void *resolveStaticMethod(const QString &typeName, const QString &methodName,
        const QList<QDotNetParameter> &params) const
    {
        init();
        if (typeName.isEmpty() || methodName.isEmpty())
            return nullptr;
        return fnResolveStaticMethod(typeName, methodName,
            static_cast<qint32>(params.size()), params);
    }

    void *resolveConstructor(const QList<QDotNetParameter> &params) const
    {
        init();
        return fnResolveConstructor(static_cast<qint32>(params.size()), params);
    }

    void *resolveInstanceMethod(const QDotNetRef &objectRef, const QString &methodName,
        const QList<QDotNetParameter> &params) const
    {
        init();
        if (QtDotNet::isNull(objectRef) || methodName.isEmpty())
            return nullptr;
        return fnResolveInstanceMethod(
            objectRef, methodName, static_cast<qint32>(params.size()), params);
    }

    using EventCallback = void(QDOTNETFUNCTION_CALLTYPE *)(void *, void *, void *, void *);

    void *resolveSafeMethod(void *funcPtr, const QList<QDotNetParameter> &params) const
    {
        init();
        if (!funcPtr)
            return nullptr;
        return fnResolveSafeMethod(
            funcPtr, static_cast<qint32>(params.size()), params);
    }

    void addEventHandler(const QDotNetRef &eventSource, const QString &eventName,
        void *context, EventCallback eventCallback) const
    {
        init();
        if (QtDotNet::isNull(eventSource) || eventName.isEmpty() || !eventCallback)
            return;
        fnAddEventHandler(eventSource, eventName, context, eventCallback);
    }

    void addEventHandler(QDotNetRef &eventSource, const QString &eventName,
        EventCallback eventCallback) const
    {
        init();
        if (QtDotNet::isNull(eventSource) || eventName.isEmpty() || !eventCallback)
            return;
        fnAddEventHandler(eventSource, eventName, &eventSource, eventCallback);
    }

    void removeEventHandler(const QDotNetRef &eventSource, const QString &eventName,
        void *context) const
    {
        init();
        if (QtDotNet::isNull(eventSource) || eventName.isEmpty())
            return;
        fnRemoveEventHandler(eventSource, eventName, context);
    }

    void removeEventHandler(QDotNetRef &eventSource, const QString &eventName) const
    {
        init();
        if (QtDotNet::isNull(eventSource) || eventName.isEmpty())
            return;
        fnRemoveEventHandler(eventSource, eventName, &eventSource);
    }

    void removeAllEventHandlers(const QDotNetRef &eventSource) const
    {
        init();
        if (QtDotNet::isNull(eventSource))
            return;
        fnRemoveAllEventHandlers(eventSource);
    }

    void *addObjectRef(const QDotNetRef &objectRef, bool weakRef = false) const
    {
        init();
        if (QtDotNet::isNull(objectRef))
            return nullptr;
        return fnAddObjectRef(objectRef, weakRef);
    }

    void freeDelegateRef(void *delegateRef) const
    {
        init();
        if (!delegateRef)
            return;
        fnFreeDelegateRef(delegateRef);
    }

    void freeObjectRef(const QDotNetRef &objectRef) const
    {
        init();
        if (QtDotNet::isNull(objectRef))
            return;
        fnFreeObjectRef(objectRef);
    }

    void freeTypeRef(const QString &typeName) const
    {
        init();
        if (typeName.isEmpty())
            return;
        fnFreeTypeRef(typeName);
    }

    void *addInterfaceProxy(const QString &interfaceName) const
    {
        init();
        if (interfaceName.isEmpty())
            return nullptr;
        return fnAddInterfaceProxy(interfaceName);
    }

    void setInterfaceMethod(const QDotNetRef &obj, const QString &methodName,
        const QList<QDotNetParameter> &params, void *callback, void *cleanUp, void *context) const
    {
        init();
        if (QtDotNet::isNull(obj) || methodName.isEmpty() || !callback)
            return;
        return fnSetInterfaceMethod(obj, methodName, static_cast<qint32>(params.size()), params,
            callback, cleanUp, context);
    }

    struct Stats
    {
        qint32 refCount;
        qint32 staticCount;
        qint32 eventCount;
        bool allZero() const
        {
            return refCount == 0
                && staticCount == 0
                && eventCount == 0;
        }
    };

    Stats stats() const
    {
        Stats s{ };
        init();
        fnStats(&s.refCount, &s.staticCount, &s.eventCount);
        return s;
    }

    void *object(const QDotNetRef &obj, const QString &path)
    {
        init();
        return fnGetObject(obj, path);
    }

private:
    QDotNetHost defaultHost;
    mutable QDotNetHost *host = nullptr;
    mutable QDotNetFunction<bool, QString> fnLoadAssembly;
    mutable QDotNetFunction<void *, QString, QString, qint32, QList<QDotNetParameter>>
        fnResolveStaticMethod;
    mutable QDotNetFunction<void *, qint32, QList<QDotNetParameter>> fnResolveConstructor;
    mutable QDotNetFunction<void *, QDotNetRef, QString, qint32, QList<QDotNetParameter>>
        fnResolveInstanceMethod;
    mutable QDotNetFunction<void *, void *, qint32, QList<QDotNetParameter>> fnResolveSafeMethod;
    mutable QDotNetFunction<void, QDotNetRef, QString, void *, EventCallback> fnAddEventHandler;
    mutable QDotNetFunction<void, QDotNetRef, QString, void *> fnRemoveEventHandler;
    mutable QDotNetFunction<void, QDotNetRef> fnRemoveAllEventHandlers;
    mutable QDotNetFunction<void *, QDotNetRef, bool> fnAddObjectRef;
    mutable QDotNetFunction<void, void *> fnFreeDelegateRef;
    mutable QDotNetFunction<void, QDotNetRef> fnFreeObjectRef;
    mutable QDotNetFunction<void, QString> fnFreeTypeRef;
    mutable QDotNetFunction<void *, QString> fnAddInterfaceProxy;
    mutable QDotNetFunction<void, QDotNetRef, QString, qint32, QList<QDotNetParameter>,
        void *, void *, void *> fnSetInterfaceMethod;
    mutable QDotNetFunction<void, qint32 *, qint32 *, qint32 *> fnStats;
    mutable QDotNetFunction<void *, QDotNetRef, QString> fnGetObject;

    static inline const QString defaultDllName = QLatin1String("Qt.DotNet.Adapter.dll");
    static inline const QString defaultAssemblyName = QLatin1String("Qt.DotNet.Adapter");
    static inline const QString defaultTypeName = QLatin1String("Qt.DotNet.Adapter");
};
