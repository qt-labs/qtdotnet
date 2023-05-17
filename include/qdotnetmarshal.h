/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetparameter.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QChar>
#include <QList>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

#include <type_traits>

class QDotNetException;
class QDotNetRef;
class QDotNetType;

namespace QtDotNet::TypeNames::System
{
    static constexpr char Void[]{ "System.Void" };
    static constexpr char Boolean[]{ "System.Boolean" };
    static constexpr char Single[]{ "System.Single" };
    static constexpr char Double[]{ "System.Double" };
    static constexpr char Byte[]{ "System.Byte" };
    static constexpr char SByte[]{ "System.SByte" };
    static constexpr char Int16[]{ "System.Int16" };
    static constexpr char UInt16[]{ "System.UInt16" };
    static constexpr char Int32[]{ "System.Int32" };
    static constexpr char UInt32[]{ "System.UInt32" };
    static constexpr char Int64[]{ "System.Int64" };
    static constexpr char UInt64[]{ "System.UInt64" };
    static constexpr char IntPtr[]{ "System.IntPtr" };
    static constexpr char UIntPtr[]{ "System.UIntPtr" };
    static constexpr char Char[]{ "System.Char" };
    static constexpr char String[]{ "System.String" };
    static constexpr char Object[]{ "System.Object" };
    static constexpr char Type[]{ "System.Type" };
}

template<typename T, typename Enable = void>
struct QDotNetTypeOf;

template<typename T, typename Enable = void>
struct QDotNetOutbound;

template<typename T, typename Enable = void>
struct QDotNetInbound;

template<typename T, typename Enable = void>
struct QDotNetNull
{
    static T value() { return T(); }
    static bool isNull(const T &obj) { return obj == value(); }
};

template<typename T>
struct QDotNetOutbound<QDotNetNull<T>>
{
    using SourceType = typename QDotNetOutbound<T>::SourceType;
    using OutboundType = typename QDotNetOutbound<T>::OutboundType;
    static inline const QDotNetParameter Parameter = QDotNetOutbound<T>::Parameter;
    static OutboundType convert(SourceType sourceValue)
    {
        return QDotNetNull<OutboundType>::value();
    }
};

template<typename T>
struct QDotNetInbound<QDotNetNull<T>>
{
    using InboundType = typename QDotNetInbound<T>::InboundType;
    using TargetType = typename QDotNetInbound<T>::TargetType;
    static inline const QDotNetParameter Parameter = QDotNetInbound<T>::Parameter;
    static TargetType convert(InboundType inboundValue)
    {
        return QDotNetNull<InboundType>::value();
    }
};

#define Q_DOTNET_TYPEOF(T,typeName,marshalAs)\
template<>\
struct QDotNetTypeOf<T>\
{\
    static inline const QString TypeName = QString(QtDotNet::TypeNames::typeName);\
    static inline UnmanagedType MarshalAs = marshalAs;\
}

Q_DOTNET_TYPEOF(void, System::Void, UnmanagedType::Void);
Q_DOTNET_TYPEOF(bool, System::Boolean, UnmanagedType::Bool);
Q_DOTNET_TYPEOF(qint8, System::SByte, UnmanagedType::I1);
Q_DOTNET_TYPEOF(quint8, System::Byte, UnmanagedType::U1);
Q_DOTNET_TYPEOF(qint16, System::Int16, UnmanagedType::I2);
Q_DOTNET_TYPEOF(quint16, System::UInt16, UnmanagedType::U2);
Q_DOTNET_TYPEOF(qint32, System::Int32, UnmanagedType::I4);
Q_DOTNET_TYPEOF(quint32, System::UInt32, UnmanagedType::U4);
Q_DOTNET_TYPEOF(qint64, System::Int64, UnmanagedType::I8);
Q_DOTNET_TYPEOF(quint64, System::UInt64, UnmanagedType::U8);
Q_DOTNET_TYPEOF(void *, System::IntPtr, UnmanagedType::SysInt);
Q_DOTNET_TYPEOF(nullptr_t, System::IntPtr, UnmanagedType::SysInt);
Q_DOTNET_TYPEOF(float, System::Single, UnmanagedType::R4);
Q_DOTNET_TYPEOF(double, System::Double, UnmanagedType::R8);

template<typename T>
struct QDotNetOutbound<T, std::enable_if_t<std::is_fundamental_v<T>>>
{
    using SourceType = T;
    using OutboundType = T;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<T>::TypeName, QDotNetTypeOf<T>::MarshalAs);
    static OutboundType convert(SourceType sourceValue)
    {
        return sourceValue;
    }
};

template<typename T>
struct QDotNetInbound<T, std::enable_if_t<std::is_fundamental_v<T>>>
{
    using InboundType = T;
    using TargetType = T;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<T>::TypeName, QDotNetTypeOf<T>::MarshalAs);
    static TargetType convert(InboundType inboundValue)
    {
        return inboundValue;
    }
};

template<typename T>
struct QDotNetNull<T, std::enable_if_t<std::is_fundamental_v<T>>>
{
    static T value() { return T(0); }
    static bool isNull(const T &obj) { return obj == value(); }
};

template<typename T>
struct QDotNetOutbound<T, std::enable_if_t<std::is_pointer_v<T>>>
{
    using SourceType = T;
    using OutboundType = void*;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<void *>::TypeName, QDotNetTypeOf<void *>::MarshalAs);
    static OutboundType convert(SourceType srvValue)
    {
        return reinterpret_cast<void *>(srvValue);
    }
};

template<typename T>
struct QDotNetInbound<T, std::enable_if_t<std::is_pointer_v<T>>>
{
    using InboundType = T;
    using TargetType = T;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<T>::TypeName, QDotNetTypeOf<T>::MarshalAs);
    static TargetType convert(InboundType inboundValue)
    {
        return reinterpret_cast<T>(inboundValue);
    }
};

template<typename T>
struct QDotNetNull<T, std::enable_if_t<std::is_pointer_v<T>>>
{
    static T value() { return nullptr; }
    static bool isNull(const T &ptr) { return ptr == value(); }
};

template<>
struct QDotNetOutbound<void>
{
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<void>::TypeName, QDotNetTypeOf<void>::MarshalAs);
};

template<>
struct QDotNetInbound<void>
{
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<void>::TypeName, QDotNetTypeOf<void>::MarshalAs);
};

template<>
struct QDotNetTypeOf<QString>
{
    static inline const QString TypeName = QStringLiteral("System.String");
    static inline UnmanagedType MarshalAs = UnmanagedType::LPWStr;
};

template<>
struct QDotNetOutbound<QString>
{
    using SourceType = const QString&;
    using OutboundType = const QChar*;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<QString>::TypeName, QDotNetTypeOf<QString>::MarshalAs);
    static OutboundType convert(SourceType sourceValue)
    {
        return sourceValue.data();
    }
};

template<>
struct QDotNetInbound<QString>
{
    using InboundType = const QChar*;
    using TargetType = QString;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<QString>::TypeName, QDotNetTypeOf<QString>::MarshalAs);
    static TargetType convert(InboundType inboundValue)
    {
        return QString(inboundValue);
    }
};

template<>
struct QDotNetNull<QString>
{
    static QString value() { return {}; }
    static bool isNull(const QString &str) { return str.isNull() || str.isEmpty(); }
};

template<typename T>
struct QDotNetOutbound<QList<T>, void>
{
    using SourceType = const QList<T>&;
    using OutboundType = const T*;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(UnmanagedType::LPArray);
    static OutboundType convert(SourceType srvValue)
    {
        return srvValue.constData();
    }
};

template<>
struct QDotNetTypeOf<QDotNetRef>
{
    static inline const QString TypeName = QStringLiteral("System.Object");
    static inline UnmanagedType MarshalAs = UnmanagedType::ObjectRef;
};

template<>
struct QDotNetTypeOf<QDotNetType>
{
    static inline const QString TypeName = QStringLiteral("System.Type");
    static inline UnmanagedType MarshalAs = UnmanagedType::ObjectRef;
};

template<>
struct QDotNetTypeOf<QDotNetException>
{
    static inline const QString TypeName = QStringLiteral("System.Exception");
    static inline UnmanagedType MarshalAs = UnmanagedType::ObjectRef;
};

template<typename T>
struct QDotNetOutbound<T, std::enable_if_t<std::is_base_of_v<QDotNetRef, T>>>
{
    using SourceType = const T&;
    using OutboundType = const void*;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<T>::TypeName, QDotNetTypeOf<T>::MarshalAs);
    static OutboundType convert(SourceType dotNetObj)
    {
        return dotNetObj.gcHandle();
    }
};

template<typename T>
struct QDotNetInbound<T, std::enable_if_t<std::is_base_of_v<QDotNetRef, T>>>
{
    using InboundType = const void*;
    using TargetType = T;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<T>::TypeName, QDotNetTypeOf<T>::MarshalAs);
    static TargetType convert(InboundType gcHandle)
    {
        return T(gcHandle);
    }
};

template<typename T>
struct QDotNetNull<T, std::enable_if_t<std::is_base_of_v<QDotNetRef, T>>>
{
    static T value() { return T(nullptr); }
    static bool isNull(const T &obj) { return !obj.isValid(); }
};

namespace QtDotNet
{
    using Null = QDotNetNull<QDotNetRef>;

    template<typename T>
    bool isNull(const T &obj) { return QDotNetNull<T>::isNull(obj); }

    template<typename T>
    T null() { return QDotNetNull<T>::value(); }
}
