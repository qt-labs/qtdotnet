/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetfunction.h"

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QMap>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

#include <functional>

class QDotNetCallbackBase
{
protected:
    QDotNetCallbackBase() = default;
public:
    virtual ~QDotNetCallbackBase() = default;
};

template<typename TResult, typename... TArg>
class QDotNetCallback : public QDotNetCallbackBase
{
public:
    using ReturnType = typename QDotNetInbound<TResult>::TargetType;
    using FunctionType = std::function<ReturnType(
        typename QDotNetInbound<TArg>::TargetType... arg)>;

    using OutboundType = typename QDotNetOutbound<TResult>::OutboundType;
    using Delegate = OutboundType(QDOTNETFUNCTION_CALLTYPE *)(
        QDotNetCallback *callback, quint64 key, typename QDotNetInbound<TArg>::InboundType...);

    using CleanUp = void(QDOTNETFUNCTION_CALLTYPE *)(QDotNetCallback *callback, quint64 key);

    QDotNetCallback(FunctionType function)
        : function(function)
    {}

    ~QDotNetCallback() override = default;

    static Delegate delegate()
    {
        return callbackDelegate;
    }

    static CleanUp cleanUp()
    {
        return callbackCleanUp;
    }

private:
    struct Box
    {
        ReturnType returnValue;
    };
    QMap<quint64, Box *> boxes;

    static OutboundType QDOTNETFUNCTION_CALLTYPE callbackDelegate(
        QDotNetCallback *callback, quint64 key, typename QDotNetInbound<TArg>::InboundType... arg)
    {
        Box *box = callback->boxes[key] = new Box
        {
            callback->function(QDotNetInbound<TArg>::convert(arg)...)
        };
        const auto result = QDotNetOutbound<TResult>::convert(box->returnValue);
        return result;
    }

    static void QDOTNETFUNCTION_CALLTYPE callbackCleanUp(QDotNetCallback *callback, quint64 key)
    {
        if (const Box *box = callback->boxes.take(key))
            delete box;
    }

    FunctionType function = nullptr;
};

template<typename... TArg>
class QDotNetCallback<void, TArg...> : public QDotNetCallbackBase
{
public:
    using FunctionType = std::function<void(typename QDotNetOutbound<TArg>::SourceType... arg)>;

    QDotNetCallback(FunctionType function)
        : function(function)
    {}

    ~QDotNetCallback() override = default;

    using Delegate = void(QDOTNETFUNCTION_CALLTYPE *)(
        QDotNetCallback *callback, quint64 key, typename QDotNetInbound<TArg>::InboundType...);
    static Delegate delegate()
    {
        return callbackDelegate;
    }

    using CleanUp = void(QDOTNETFUNCTION_CALLTYPE *)(QDotNetCallback *callback, quint64 key);
    static CleanUp cleanUp()
    {
        return callbackCleanUp;
    }

private:
    static void QDOTNETFUNCTION_CALLTYPE callbackDelegate(QDotNetCallback *callback, quint64 key,
        typename QDotNetInbound<TArg>::InboundType... arg)
    {
        callback->function(QDotNetInbound<TArg>::convert(arg)...);
    }

    static void QDOTNETFUNCTION_CALLTYPE callbackCleanUp(QDotNetCallback *callback, quint64 key)
    {}

    FunctionType function = nullptr;
};
