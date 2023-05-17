/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include <qdotnetinterface.h>
#include <qdotnetobject.h>

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <QObject>
#include <QString>
#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif

struct FooPrivate;

class IBarTransformation : public QDotNetInterface
{
public:
    static inline const QString &FullyQualifiedTypeName =
        QStringLiteral("FooLib.IBarTransformation, FooLib");

    virtual QString transform(const QString &) = 0;

protected:
    IBarTransformation();
    ~IBarTransformation() override = default;
};

class Foo final : public QObject, public QDotNetObject
{
    Q_OBJECT
    Q_PROPERTY(QString bar READ bar WRITE setBar NOTIFY barChanged)

public:
    Q_DOTNET_OBJECT(Foo, "FooLib.Foo, FooLib");

    Foo();
    Foo(const IBarTransformation &transformation);
    ~Foo() override;

    [[nodiscard]] QString bar() const;
    void setBar(const QString &value);

signals:
    void barChanged();

private:
    FooPrivate *d;
};
