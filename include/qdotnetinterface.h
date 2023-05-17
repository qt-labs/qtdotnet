/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetref.h"
#include "qdotnetcallback.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QList>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

class QDotNetInterface : public QDotNetRef
{
public:
    QDotNetInterface(const QString &interfaceName)
        : QDotNetRef(adapter().addInterfaceProxy(interfaceName))
    {}

    template<typename TResult, typename... TArg>
    void setCallback(const QString &methodName, const QList<QDotNetParameter> &params,
        typename QDotNetCallback<TResult, TArg...>::FunctionType function)
    {
        auto *callback = new QDotNetCallback<TResult, TArg...>(function);
        callbacks.append(callback);

        QList<QDotNetParameter> modifiedParams
        {
            params[0],
            UnmanagedType::SysInt,
            UnmanagedType::U8
        };
        for (qsizetype i = 1; i < params.size(); ++i)
            modifiedParams.append(params[i]);

        adapter().setInterfaceMethod(*this, methodName, modifiedParams,
            reinterpret_cast<void *>(callback->delegate()),
            reinterpret_cast<void *>(callback->cleanUp()), callback);
    }

    template<typename TResult, typename... TArg>
    void setCallback(const QString &methodName,
        typename QDotNetCallback<TResult, TArg...>::FunctionType function)
    {
        auto *callback = new QDotNetCallback<TResult, TArg...>(function);
        callbacks.append(callback);

        const QList<QDotNetParameter> parameters
        {
            QDotNetInbound<TResult>::Parameter,
            UnmanagedType::SysInt,
            UnmanagedType::U8,
            QDotNetInbound<TArg>::Parameter...
        };

        adapter().setInterfaceMethod(
            *this, methodName, parameters,
            callback->delegate(), callback->cleanUp(), callback);
    }

    ~QDotNetInterface() override
    {
        for (const QDotNetCallbackBase *callback : callbacks)
            delete callback;
        callbacks.clear();
    }

private:
    QList<QDotNetCallbackBase *> callbacks;
};

template<typename T>
struct QDotNetTypeOf<T, std::enable_if_t<std::is_base_of_v<QDotNetInterface, T>>>
{
    static inline const QString TypeName = T::FullyQualifiedTypeName;
    static inline UnmanagedType MarshalAs = UnmanagedType::ObjectRef;
};
