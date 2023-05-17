/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetexception.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QList>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

template<typename T, typename... TArg>
class QDotNetSafeMethod
{
    using FuncType = QDotNetFunction<T, TArg...>;
    using SafeFuncType = QDotNetFunction<QDotNetRef, QDotNetRef, TArg...>;

public:
    QDotNetSafeMethod() = default;
    QDotNetSafeMethod(FuncType func)
        : func(func)
    {
        const QList<QDotNetParameter> parameters
        {
            QDotNetInbound<QDotNetRef>::Parameter,
            QDotNetOutbound<QDotNetRef>::Parameter,
            QDotNetOutbound<TArg>::Parameter...
        };
        safeFunc = QDotNetAdapter::instance().resolveSafeMethod(func.ptr(), parameters);
    }

    bool isValid() const { return func.isValid(); }

    class SafeReturn : public QDotNetRef
    {
    public:
        SafeReturn(const void *objectRef = nullptr)
            : QDotNetRef(objectRef)
        {}

        SafeReturn(const SafeReturn &cpySrc)
            : QDotNetRef(cpySrc)
        {}

        SafeReturn(SafeReturn &&movSrc) noexcept
            : QDotNetRef(std::move(movSrc))
        {}

        T value() const
        {
            if (!fnValue.isValid()) {
                fnValue = adapter().resolveInstanceMethod(
                    *this, "get_Value", { QDotNetInbound<T>::Parameter });
            }
            return fnValue();
        }
        QDotNetException exception() const
        {
            if (!fnException.isValid()) {
                fnException = adapter().resolveInstanceMethod(
                    *this, "get_Exception", { QDotNetInbound<QDotNetException>::Parameter });
            }
            return fnException();
        }
    private:
        mutable QDotNetFunction<T> fnValue = nullptr;
        mutable QDotNetFunction<QDotNetException> fnException = nullptr;
    };

    typename QDotNetInbound<T>::TargetType invoke(QDotNetOutbound<QDotNetRef>::SourceType obj,
        typename QDotNetOutbound<TArg>::SourceType... arg) const
    {
        SafeReturn safeReturn = safeFunc(obj, arg...).template cast<SafeReturn>();
        QDotNetException exception = safeReturn.exception();
        if (!exception.isValid())
            return safeReturn.value();
        throw exception;
    }

    typename QDotNetInbound<T>::TargetType invoke(nullptr_t nullObj,
        typename QDotNetOutbound<TArg>::SourceType... arg) const
    {
        SafeReturn safeReturn = safeFunc(nullObj, arg...).template cast<SafeReturn>();
        QDotNetException exception = safeReturn.exception();
        if (!exception.isValid())
            return safeReturn.value();
        throw exception;
    }

private:
    FuncType func = nullptr;
    SafeFuncType safeFunc = nullptr;
};

template<typename... TArg>
class QDotNetSafeMethod<void, TArg...>
{
    using FuncType = QDotNetFunction<void, TArg...>;
    using SafeFuncType = QDotNetFunction<QDotNetRef, QDotNetRef, TArg...>;

public:
    QDotNetSafeMethod() = default;
    QDotNetSafeMethod(FuncType func)
        : func(func)
    {
        const QList<QDotNetParameter> parameters
        {
            QDotNetInbound<QDotNetRef>::Parameter,
            QDotNetOutbound<QDotNetRef>::Parameter,
            QDotNetOutbound<TArg>::Parameter...
        };
        safeFunc = QDotNetAdapter::instance().resolveSafeMethod(func.ptr(), parameters);
    }

    bool isValid() const { return func.isValid(); }

    class SafeReturn : public QDotNetRef
    {
    public:
        SafeReturn(const void *objectRef = nullptr)
            : QDotNetRef(objectRef)
        {}

        SafeReturn(const SafeReturn &cpySrc)
            : QDotNetRef(cpySrc)
        {}

        SafeReturn(SafeReturn &&movSrc) noexcept
            : QDotNetRef(std::move(movSrc))
        {}

        QDotNetException exception() const
        {
            if (!fnException.isValid()) {
                fnException = QDotNetAdapter::instance().resolveInstanceMethod(
                    *this, "get_Exception", { QDotNetInbound<QDotNetException>::Parameter });
            }
            return fnException();
        }
    private:
        mutable QDotNetFunction<QDotNetException> fnException = nullptr;
    };

    void invoke(QDotNetOutbound<QDotNetRef>::SourceType obj,
        typename QDotNetOutbound<TArg>::SourceType... arg) const
    {
        SafeReturn safeReturn = safeFunc(obj, arg...).template cast<SafeReturn>();
        QDotNetException exception = safeReturn.exception();
        if (exception.isValid())
            throw exception;
    }

    void invoke(nullptr_t nullObj, typename QDotNetOutbound<TArg>::SourceType... arg) const
    {
        SafeReturn safeReturn = safeFunc(nullObj, arg...).template cast<SafeReturn>();
        QDotNetException exception = safeReturn.exception();
        if (exception.isValid())
            throw exception;
    }

private:
    FuncType func = nullptr;
    SafeFuncType safeFunc = nullptr;
};
