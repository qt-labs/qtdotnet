/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnethostfxr.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QChar>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

// Format of parameter info field
// 63......................31...........15............0
// |------ ARRAYLENGTH -----|--- FLAGS --|--- TYPE ---|

constexpr auto TYPE_OFFSET = 0;
constexpr auto TYPE_SIZE = 16;

constexpr auto FLAGS_OFFSET = TYPE_OFFSET + TYPE_SIZE;
constexpr auto FLAGS_SIZE = 16;
constexpr auto FLAGS_IN_BIT = 0;
constexpr auto FLAGS_OUT_BIT = 1;
constexpr auto FLAGS_ARRAY_BIT = 2;
constexpr auto FLAGS_FIXEDLENGTH_BIT = 3;
constexpr auto FLAGS_WEAKREF_BIT = 4;

constexpr auto ARRAYLENGTH_OFFSET = FLAGS_OFFSET + FLAGS_SIZE;
constexpr auto ARRAYLENGTH_SIZE = 32;

template<typename T1, typename T2>
constexpr auto  MASK(T1 value, T2 size)
{
    return static_cast<quint64>(value) & ((static_cast<quint64>(1) << size) - 1);
}
template <typename T1, typename T2>
constexpr auto FLAG(T1 condition, T2 offset)
{
    return (condition ? static_cast<quint64>(1) : static_cast<quint64>(0)) << offset;
}

struct QDotNetParameter
{
    const QChar* typeName = nullptr;
    quint64 paramInfo;

    enum class Template
    {
        ObjectRef =  MASK(UnmanagedType::ObjectRef, TYPE_SIZE) << TYPE_OFFSET
            | FLAG(false, FLAGS_WEAKREF_BIT) << FLAGS_OFFSET,
        ObjectWeakRef =  MASK(UnmanagedType::ObjectRef, TYPE_SIZE) << TYPE_OFFSET
            | FLAG(true, FLAGS_WEAKREF_BIT) << FLAGS_OFFSET,
    };

    QDotNetParameter(const char *typeName, Template paramTemplate)
        : QDotNetParameter(QString(typeName), paramTemplate)
    {}

    QDotNetParameter(const QString &typeName, Template paramTemplate)
        : QDotNetParameter(typeName)
    {
        paramInfo = static_cast<quint64>(paramTemplate);
    }

    QDotNetParameter(UnmanagedType unmanagedType = UnmanagedType::ObjectRef,
            bool isIn = false, bool isOut = false, bool isArray = false,
            bool isFixedLength = false, qint32 arrayLength = 0, bool isWeakRef = false)
        : QDotNetParameter(QString{}, unmanagedType, isIn, isOut, isArray, isFixedLength,
            arrayLength, isWeakRef)
    {}

    QDotNetParameter(const char *typeName, UnmanagedType unmanagedType = UnmanagedType::ObjectRef,
            bool isIn = false, bool isOut = false, bool isArray = false,
            bool isFixedLength = false, qint32 arrayLength = 0, bool isWeakRef = false)
        : QDotNetParameter(QString(typeName),
            unmanagedType, isIn, isOut, isArray, isFixedLength, arrayLength, isWeakRef)
    {}

    QDotNetParameter(const QString &typeName,
        UnmanagedType unmanagedType = UnmanagedType::ObjectRef,bool isIn = false,
        bool isOut = false, bool isArray = false, bool isFixedLength = false,
        qint32 arrayLength = 0, bool isWeakRef = false)
    {
        this->typeName = typeName.constData();
        paramInfo = MASK(unmanagedType, TYPE_SIZE) << TYPE_OFFSET;
        paramInfo |= FLAG(isIn, FLAGS_IN_BIT) << FLAGS_OFFSET;
        paramInfo |= FLAG(isOut, FLAGS_OUT_BIT) << FLAGS_OFFSET;
        paramInfo |= FLAG(isArray, FLAGS_ARRAY_BIT) << FLAGS_OFFSET;
        paramInfo |= FLAG(isFixedLength, FLAGS_FIXEDLENGTH_BIT) << FLAGS_OFFSET;
        paramInfo |= FLAG(isWeakRef, FLAGS_WEAKREF_BIT) << FLAGS_OFFSET;
        paramInfo |= MASK(arrayLength, ARRAYLENGTH_SIZE) << ARRAYLENGTH_OFFSET;
    }

    static const QDotNetParameter &String;
};

inline const QDotNetParameter &QDotNetParameter::String = QDotNetParameter(
    QStringLiteral("Qt.DotNet.StringMarshaler, Qt.DotNet.Adapter, CleanUp=false"),
    UnmanagedType::CustomMarshaler);
