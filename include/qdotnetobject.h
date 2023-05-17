/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnettype.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QList>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

class QDotNetObject : public QDotNetRef
{
private:
    // nullptr and void* ctors
#define Q_DOTNET_OBJECT_REF(T)\
    T(nullptr_t);\
    T(const void *)

    // Copy ctor and assignment
#define Q_DOTNET_OBJECT_COPY(T)\
    T(const T &);\
    T &operator =(const T &)

    // Move ctor and assignment
#define Q_DOTNET_OBJECT_MOVE(T)\
    T(T &&) noexcept;\
    T &operator=(T &&) noexcept

    // Fully qualified .NET class name
#define Q_DOTNET_OBJECT_TYPE(T,type_name)\
    static inline const QString &FullyQualifiedTypeName = QString(type_name)

    // All required declarations
#define Q_DOTNET_OBJECT(T,type_name)\
    Q_DOTNET_OBJECT_REF(T);\
    Q_DOTNET_OBJECT_COPY(T);\
    Q_DOTNET_OBJECT_MOVE(T);\
    Q_DOTNET_OBJECT_TYPE(T, type_name)

    // nullptr and void* ctors
#define Q_DOTNET_OBJECT_REF_IMPL(T,...)\
    T::T(nullptr_t) \
        : QDotNetObject(nullptr) __VA_ARGS__ \
    {}\
    T::T(const void *objectRef) \
        : QDotNetObject(objectRef) __VA_ARGS__ \
    {}

    // Copy ctor and assignment
#define Q_DOTNET_OBJECT_COPY_IMPL(T,...)\
    T::T(const T &cpySrc) \
        : QDotNetObject(cpySrc) __VA_ARGS__ \
    {}\
    T &T::operator=(const T &cpySrc)\
    {\
        QDotNetObject::operator=(cpySrc);\
        return *this;\
    }

    // Move ctor and assignment
#define Q_DOTNET_OBJECT_MOVE_IMPL(T,...)\
    T::T(T &&movSrc) noexcept \
        : QDotNetObject(std::move(movSrc)) __VA_ARGS__\
    {}\
    T &T::operator=(T &&movSrc) noexcept\
    {\
        QDotNetObject::operator=(std::move(movSrc));\
        return *this;\
    }

    // All required ctor and assignment implementations
#define Q_DOTNET_OBJECT_IMPL(T,...)\
    Q_DOTNET_OBJECT_REF_IMPL(T, __VA_ARGS__)\
    Q_DOTNET_OBJECT_COPY_IMPL(T, __VA_ARGS__)\
    Q_DOTNET_OBJECT_MOVE_IMPL(T, __VA_ARGS__)

    // nullptr and void* ctors
#define Q_DOTNET_OBJECT_REF_INLINE(T,...)\
    T(nullptr_t)\
        : QDotNetObject(nullptr) __VA_ARGS__\
    {}\
    T(const void *objectRef)\
        : QDotNetObject(objectRef) __VA_ARGS__\
    {}

    // Copy ctor and assignment
#define Q_DOTNET_OBJECT_COPY_INLINE(T,...)\
    T(const T &cpySrc)\
        : QDotNetObject(cpySrc) __VA_ARGS__\
    {}\
    T &operator=(const T &cpySrc)\
    {\
        QDotNetObject::operator=(cpySrc);\
        return *this;\
    }

    // Move ctor and assignment
#define Q_DOTNET_OBJECT_MOVE_INLINE(T,...)\
    T(T &&movSrc) noexcept\
        : QDotNetObject(std::move(movSrc)) __VA_ARGS__\
    {}\
    T &operator=(T &&movSrc) noexcept\
    {\
        QDotNetObject::operator=(std::move(movSrc));\
        return *this;\
    }

    // All required ctor and assignment implementations
#define Q_DOTNET_OBJECT_INLINE(T,type_name,...)\
    Q_DOTNET_OBJECT_REF_INLINE(T, __VA_ARGS__)\
    Q_DOTNET_OBJECT_COPY_INLINE(T, __VA_ARGS__)\
    Q_DOTNET_OBJECT_MOVE_INLINE(T, __VA_ARGS__)\
    Q_DOTNET_OBJECT_TYPE(T, type_name)

    // Member initialization list for derived class
#define Q_DOTNET_OBJECT_INIT(...) , __VA_ARGS__

public:
    static inline const QString &FullyQualifiedTypeName = QStringLiteral("System.Object");

    QDotNetObject(const void *objectRef = nullptr)
        : QDotNetRef(objectRef)
    {}

    QDotNetObject(const QDotNetObject &cpySrc)
        : QDotNetRef(cpySrc)
    {}

    QDotNetObject &operator =(const QDotNetObject &cpySrc)
    {
        QDotNetRef::operator=(cpySrc);
        return *this;
    }

    QDotNetObject(QDotNetObject &&movSrc) noexcept
        : QDotNetRef(std::move(movSrc))
    {}

    QDotNetObject &operator=(QDotNetObject &&movSrc) noexcept
    {
        QDotNetRef::operator=(std::move(movSrc));
        return *this;
    }

    const QDotNetType &type() const
    {
        if (!fnGetType.isValid()) {
            method("GetType", fnGetType);
            objType = fnGetType.invoke(*this);
        }
        return objType;
    }

    QString toString() const
    {
        return method("ToString", fnToString).invoke(*this);
    }

    bool equals(const QDotNetRef &obj) const
    {
        return method("Equals", fnEquals).invoke(*this, obj);
    }

    template<typename TResult, typename ...TArg>
    QDotNetFunction<TResult, TArg...> method(const QString &methodName) const
    {
        const QList<QDotNetParameter> parameters
        {
            QDotNetInbound<TResult>::Parameter,
            QDotNetOutbound<TArg>::Parameter...
        };
        return adapter().resolveInstanceMethod(*this, methodName, parameters);
    }

    template<typename TResult, typename ...TArg>
    QDotNetFunction<TResult, TArg...> &method(const QString &methodName,
        QDotNetFunction<TResult, TArg...> &func) const
    {
        if (!func.isValid())
            func = method<TResult, TArg...>(methodName);
        return func;
    }

    template<typename TResult, typename ...TArg>
    QDotNetSafeMethod<TResult, TArg...> &method(const QString &methodName,
        QDotNetSafeMethod<TResult, TArg...> &func) const
    {
        if (!func.isValid())
            func = method<TResult, TArg...>(methodName);
        return func;
    }

    template<typename TResult, typename ...TArg>
    QDotNetFunction<TResult, TArg...> staticMethod(const QString &methodName) const
    {
        return type().staticMethod<TResult, TArg...>(methodName);
    }

    template<typename TResult, typename ...TArg>
    QDotNetFunction<TResult, TArg...> &staticMethod(const QString &methodName,
        QDotNetFunction<TResult, TArg...> &func) const
    {
        if (!func.isValid())
            func = type().staticMethod<TResult, TArg...>(methodName);
        return func;
    }

    template<typename TResult, typename ...TArg>
    QDotNetSafeMethod<TResult, TArg...> &staticMethod(const QString &methodName,
        QDotNetSafeMethod<TResult, TArg...> &func) const
    {
        if (!func.isValid())
            func = type().staticMethod<TResult, TArg...>(methodName);
        return func;
    }

    template<typename ...TArg>
    static QDotNetFunction<QDotNetObject, TArg...> constructor(const QString &typeName)
    {
        return QDotNetType::constructor<QDotNetObject, TArg...>(typeName);
    }

    template<typename ...TArg>
    static QDotNetFunction<QDotNetObject, TArg...> &constructor(const QString &typeName,
        QDotNetFunction<QDotNetObject, TArg...> &ctor)
    {
        return QDotNetType::constructor(typeName, ctor);
    }

    template<typename ...TArg>
    static QDotNetSafeMethod<QDotNetObject, TArg...> &constructor(const QString &typeName,
        QDotNetSafeMethod<QDotNetObject, TArg...> &ctor)
    {
        return QDotNetType::constructor(typeName, ctor);
    }

    struct IEventHandler
    {
        virtual ~IEventHandler() = default;
        virtual void handleEvent(const QString &eventName, QDotNetObject &eventSource,
            QDotNetObject &eventArgs) = 0;
    };

    void subscribeEvent(const QString &eventName, IEventHandler *eventHandler)
    {
        adapter().addEventHandler(*this, eventName, eventHandler, eventCallback);
    }

    void unsubscribeEvent(const QString &eventName, IEventHandler *eventHandler)
    {
        adapter().removeEventHandler(*this, eventName, eventHandler);
    }

    QDotNetObject object(const QString &path)
    {
        return adapter().object(*this, path);
    }

    template<typename TResult, typename ...TArg>
    TResult call(const QString &methodName, TArg... arg) const
    {
        return method<TResult, TArg...>(methodName).invoke(*this, arg...);
    }

    template<typename ...TArg>
    void call(const QString &methodName, TArg... arg) const
    {
        method<void, TArg...>(methodName).invoke(*this, arg...);
    }

protected:
    template<typename T, typename ...TArg>
    static QDotNetFunction<T, TArg...> constructor()
    {
        return QDotNetType::constructor<T, TArg...>(T::FullyQualifiedTypeName);
    }

    template<typename T, typename ...TArg>
    static QDotNetFunction<T, TArg...> &constructor(QDotNetFunction<T, TArg...> &ctor)
    {
        return QDotNetType::constructor(T::FullyQualifiedTypeName, ctor);
    }

    template<typename T, typename ...TArg>
    static QDotNetSafeMethod<T, TArg...> &constructor(QDotNetSafeMethod<T, TArg...> &ctor)
    {
        return QDotNetType::constructor(T::FullyQualifiedTypeName, ctor);
    }

private:
    static void QDOTNETFUNCTION_CALLTYPE eventCallback(void *context, void *eventNameChars,
        void *eventSourceRef, void *eventArgsRef)
    {
        auto *receiver = static_cast<IEventHandler *>(context);
        const QString eventName(static_cast<const QChar *>(eventNameChars));
        QDotNetObject eventSource(eventSourceRef);
        QDotNetObject eventArgs(eventArgsRef);
        receiver->handleEvent(eventName, eventSource, eventArgs);
    }

    mutable QDotNetFunction<QDotNetType> fnGetType;
    mutable QDotNetType objType = nullptr;
    mutable QDotNetFunction<QString> fnToString;
    mutable QDotNetFunction<bool, QDotNetRef> fnEquals;
};

template<typename T>
struct QDotNetTypeOf<T, std::enable_if_t<std::is_base_of_v<QDotNetObject, T>>>
{
    static inline const QString TypeName = T::FullyQualifiedTypeName;
    static inline UnmanagedType MarshalAs = UnmanagedType::ObjectRef;
};
