/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetobject.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QString>
#include <QRegularExpression>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

template <typename T, std::enable_if_t<
    std::is_fundamental_v<T>
    || std::is_same_v<T, QString>
    || std::is_base_of_v<QDotNetRef, T>, bool> = true>
class QDotNetArray : public QDotNetObject
{
    class Element;
    static QString arrayOf(const QString& typeName)
    {
        const auto idx = typeName.indexOf(QRegularExpression(",|$"));
        return QString("%1[]%2").arg(typeName.left(idx)).arg(typeName.mid(idx));
    }

public:
    Q_DOTNET_OBJECT_INLINE(QDotNetArray, arrayOf(QDotNetTypeOf<T>::TypeName));

    QDotNetArray(qint32 length)
    {
        const QString elementTypeName = QDotNetTypeOf<T>::TypeName;
        const QDotNetType elementType = QDotNetType::find(elementTypeName);

        QDotNetType arrayType = QDotNetType::find(QDotNetArray::FullyQualifiedTypeName);
        auto ctor = constructor<QDotNetArray, qint32>();
        *this = ctor(length);
    }

    qint32 length() const
    {
        return method("get_Length", fnLength).invoke(*this);
    }

    T get(qint32 idx) const
    {
        if constexpr (std::is_fundamental_v<T>)
            return method("Get", fnGetValue).invoke(*this, idx);
        if constexpr (std::is_same_v<T, QString>)
            return method("Get", fnGetObject).invoke(*this, idx).toString();
        if constexpr (std::is_base_of_v<QDotNetRef, T>)
            return method("Get", fnGetObject).invoke(*this, idx).template cast<T>();
        throw std::invalid_argument("T");
    }

    void set(qint32 idx, const T &value)
    {
        if constexpr (std::is_same_v<T, QString>) {
            if (!fnSetString.isValid()) {
                QDotNetFunction<void, qint32, QDotNetObject> const func = adapter()
                    .resolveInstanceMethod(*this, "Set",
                        { UnmanagedType::Void, UnmanagedType::I4, QDotNetParameter::String });
                fnSetString = func;
            }
            return method("Set", fnSet).invoke(*this, idx, value);
        }
        return method("Set", fnSet).invoke(*this, idx, value);
    }

    Element operator[](qint32 idx)
    {
        return Element(this, idx);
    }

    Element begin()
    {
        return Element(this, 0);
    }

    Element end()
    {
        return Element(this, length());
    }

private:
    class Element
    {
        friend class QDotNetArray;

    public:
        operator T()
        {
            return value = a->get(idx);
        }
        T *operator->()
        {
            value = a->get(idx);
            return &value;
        }
        T &operator*()
        {
            value = a->get(idx);
            return value;
        }
        Element &operator=(const T &value)
        {
            a->set(idx, value);
            return *this;
        }
        bool isEnd() const
        {
            return idx >= a->length();
        }
        Element &operator++()
        {
            ++idx;
            return *this;
        }
        bool operator !=(const Element &that) const
        {
            if (isEnd() && that.isEnd())
                return false;
            return a != that.a || idx != that.idx;
        }

    private:
        Element(QDotNetArray *a, qint32 idx)
            : a(a)
            , idx(idx)
        {}
        QDotNetArray* a = nullptr;
        qint32 idx;
        T value;
    };

    mutable QDotNetSafeMethod<qint32> fnLength;
    mutable QDotNetSafeMethod<T, qint32> fnGetValue;
    mutable QDotNetSafeMethod<QDotNetObject, qint32> fnGetObject;
    QDotNetSafeMethod<void, qint32, T> fnSet;
    QDotNetSafeMethod<void, qint32, QDotNetObject> fnSetString;
};
