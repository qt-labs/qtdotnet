/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetadapter.h"

class QDotNetRef
{
public:
    static inline const QString &FullyQualifiedTypeName = QStringLiteral("System.Object");

    const void *gcHandle() const { return objectRef; }
    bool isValid() const { return gcHandle() != nullptr; }

    template<typename T, std::enable_if_t<std::is_base_of_v<QDotNetRef, T>, bool> = true>
    T cast(bool copy = false)
    {
        T newObj(nullptr);
        if (copy)
            newObj.copyFrom(*this);
        else
            newObj.moveFrom(*this);
        return newObj;
    }

    QDotNetRef(const void *objectRef = nullptr)
        : objectRef(objectRef)
    {}

    QDotNetRef(const QDotNetRef &cpySrc)
    {
        copyFrom(cpySrc);
    }

    QDotNetRef(QDotNetRef &&movSrc) noexcept
    {
        moveFrom(movSrc);
    }

    virtual ~QDotNetRef()
    {
        freeObjectRef();
    }

    QDotNetRef &operator=(const QDotNetRef &cpySrc)
    {
        return copyFrom(cpySrc);
    }

    QDotNetRef &operator=(QDotNetRef &&movSrc) noexcept
    {
        return moveFrom(movSrc);
    }

    template<typename T, std::enable_if_t<std::is_base_of_v<QDotNetRef, T>, bool> = true>
    class Null
    {};

protected:
    static QDotNetAdapter &adapter() { return QDotNetAdapter::instance(); }

    void attach(const void *objectRef)
    {
        this->objectRef = objectRef;
    }

    QDotNetRef &copyFrom(const QDotNetRef &that)
    {
        freeObjectRef();
        if (that.isValid())
            objectRef = adapter().addObjectRef(that);
        return *this;
    }

    QDotNetRef &moveFrom(QDotNetRef &that)
    {
        freeObjectRef();
        objectRef = that.objectRef;
        that.objectRef = nullptr;
        return *this;
    }

private:
    void freeObjectRef()
    {
        if (!isValid())
            return;
        adapter().freeObjectRef(*this);
        objectRef = nullptr;
    }

    const void *objectRef = nullptr;
};

template<typename T>
struct QDotNetOutbound<QDotNetRef::Null<T>, std::enable_if_t<std::is_base_of_v<QDotNetRef, T>>>
{
    using SourceType = nullptr_t;
    using OutboundType = const void*;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<T>::TypeName, QDotNetTypeOf<T>::MarshalAs);
    static OutboundType convert(SourceType dotNetObj)
    {
        return nullptr;
    }
};
