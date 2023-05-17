/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetref.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QException>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

class QDotNetException : public QException, public QDotNetRef
{
public:
    QDotNetException()
        : QDotNetException({ "System.Exception" })
    {}

    QDotNetException(const QString &type)
    {
        const QDotNetFunction<QDotNetException> ctor = adapter().resolveConstructor({ type });
        *this = ctor();
    }

    QDotNetException(const QString &type, const QString &message)
    {
        const QDotNetFunction<QDotNetException, QString> ctor = adapter().resolveConstructor(
            { type, QDotNetTypeOf<QString>::MarshalAs });
        *this = ctor(message);
    }

    QDotNetException(const void *objRef = nullptr)
        : QDotNetRef(objRef)
    {}

    QDotNetException(const QDotNetException &cpySrc)
        : QDotNetRef(adapter().addObjectRef(&cpySrc))
    {}

    QDotNetException &operator =(const QDotNetException &cpySrc)
    {
        QDotNetRef::operator=(cpySrc);
        return *this;
    }

    QDotNetException(QDotNetException &&movSrc) noexcept
        : QDotNetRef(std::move(movSrc))
    {}

    void raise() const override { throw *this; }
    QDotNetException *clone() const override { return new QDotNetException(*this); }

    QDotNetException &operator=(QDotNetException &&movSrc) noexcept
    {
        QDotNetRef::operator=(std::move(movSrc));
        return *this;
    }

    template<typename TResult, typename ...TArg>
    QDotNetFunction<TResult, TArg...> method(const QString &methodName,
        QDotNetFunction<TResult, TArg...> &func) const
    {
        if (!func.isValid()) {
            const QList<QDotNetParameter> parameters
            {
                QDotNetInbound<TResult>::Parameter,
                QDotNetOutbound<TArg>::Parameter...
            };
            func = adapter().resolveInstanceMethod(*this, methodName, parameters);
        }
        return func;
    }

    qint32 hResult() const
    {
        return method("get_HResult", fnHResult).invoke(*this);
    }

    QDotNetException innerException() const
    {
        return method("get_InnerException", fnInnerException).invoke(*this)
            .cast<QDotNetException>();
    }

    QString message() const
    {
        return method("get_Message", fnMessage).invoke(*this);
    }

    QString source() const
    {
        return method("get_Source", fnSource).invoke(*this);
    }

    QString stackTrace() const
    {
        return method("get_StackTrace", fnStackTrace).invoke(*this);
    }

    QDotNetRef type() const
    {
        return method("GetType", fnGetType).invoke(*this);
    }

    QString toString() const
    {
        return method("ToString", fnToString).invoke(*this);
    }

    bool equals(const QDotNetRef &obj) const
    {
        return method("Equals", fnEquals).invoke(*this, obj);
    }

private:
    mutable QDotNetFunction<qint32> fnHResult = nullptr;
    mutable QDotNetFunction<QDotNetRef> fnInnerException = nullptr;
    mutable QDotNetFunction<QString> fnMessage = nullptr;
    mutable QDotNetFunction<QString> fnSource = nullptr;
    mutable QDotNetFunction<QString> fnStackTrace = nullptr;
    mutable QDotNetFunction<QDotNetRef> fnGetType = nullptr;
    mutable QDotNetFunction<QString> fnToString = nullptr;
    mutable QDotNetFunction<bool, QDotNetRef> fnEquals = nullptr;
};
