/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include "qdotnetmarshal.h"

#ifdef Q_OS_WINDOWS
#   define QDOTNETFUNCTION_CALLTYPE __stdcall
#else
#   define QDOTNETFUNCTION_CALLTYPE
#endif

template<typename T, typename... TArg>
class QDotNetFunction
{
public:
    QDotNetFunction(void *funcPtr = nullptr)
        : funcPtr(reinterpret_cast<Delegate>(funcPtr))
    {}

    QDotNetFunction(const QDotNetFunction &cpySrc)
        : funcPtr(cpySrc.funcPtr)
    {}

    QDotNetFunction &operator=(const QDotNetFunction &cpySrc)
    {
        this->funcPtr = cpySrc.funcPtr;
        return *this;
    }

    void *ptr() const { return reinterpret_cast<void *>(funcPtr); }
    bool isValid() const { return funcPtr != nullptr; }

    typename QDotNetInbound<T>::TargetType operator()(
        typename QDotNetOutbound<TArg>::SourceType... arg) const
    {
        if (!isValid())
            return QDotNetNull<T>::value();
        return QDotNetInbound<T>::convert(funcPtr(QDotNetOutbound<TArg>::convert(arg)...));
    }

private:
    using Delegate = typename QDotNetInbound<T>::InboundType(QDOTNETFUNCTION_CALLTYPE *)(
        typename QDotNetOutbound<TArg>::OutboundType...);
    Delegate funcPtr = nullptr;
};

template<typename... TArg>
class QDotNetFunction<void, TArg...>
{
public:
    QDotNetFunction(void *funcPtr = nullptr)
        : funcPtr(reinterpret_cast<Delegate>(funcPtr))
    {}

    void *ptr() const { return reinterpret_cast<void *>(funcPtr); }
    bool isValid() const { return funcPtr != nullptr; }

    void operator()(typename QDotNetOutbound<TArg>::SourceType... arg) const
    {
        if (isValid())
            funcPtr(QDotNetOutbound<TArg>::convert(arg)...);
    }

private:
    using Delegate = void(QDOTNETFUNCTION_CALLTYPE *)(
        typename QDotNetOutbound<TArg>::OutboundType...);
    Delegate funcPtr = nullptr;
};
