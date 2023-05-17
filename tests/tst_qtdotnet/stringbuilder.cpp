/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include <qdotnetsafemethod.h>

#include "stringbuilder.h"

struct StringBuilderPrivate final
{
    StringBuilderPrivate() = default;
    StringBuilderPrivate(StringBuilder *q) : q(q)
    {}
    StringBuilder *q = nullptr;
    QDotNetSafeMethod<StringBuilder, QString> append;
};

Q_DOTNET_OBJECT_IMPL(StringBuilder, Q_DOTNET_OBJECT_INIT(d(new StringBuilderPrivate(this))));

StringBuilder::StringBuilder() :
    d(new StringBuilderPrivate(this))
{
    const QDotNetFunction<StringBuilder> ctor = constructor<StringBuilder>();
    *this = ctor();
}

StringBuilder::StringBuilder(qint32 capacity, qint32 maxCapacity) :
    d(new StringBuilderPrivate(this))
{
    const auto ctor = constructor<StringBuilder, qint32, qint32>();
    *this = ctor(capacity, maxCapacity);
}

StringBuilder::~StringBuilder()
{
    delete d;
}

StringBuilder StringBuilder::append(const QString &str)
{
    return method("Append", d->append).invoke(*this, str);
}
