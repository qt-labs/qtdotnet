/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "foo.h"

#include <qdotnetevent.h>

struct FooPrivate final : QDotNetObject::IEventHandler
{
    Foo *q;
    FooPrivate(Foo *q) : q(q) {}

    QDotNetFunction<Foo, IBarTransformation> ctor = nullptr;

    QDotNetFunction<QString> bar;
    QDotNetFunction<void, QString> setBar;

    void handleEvent(const QString &eventName, QDotNetObject &sender, QDotNetObject &args) override
    {
        if (eventName != "PropertyChanged")
            return;

        if (args.type().fullName() != QDotNetPropertyEvent::FullyQualifiedTypeName)
            return;

        const auto propertyChangedEvent = args.cast<QDotNetPropertyEvent>();
        if (propertyChangedEvent.propertyName() == "Bar")
            emit q->barChanged();
    }
};

Q_DOTNET_OBJECT_IMPL(Foo, Q_DOTNET_OBJECT_INIT(d(new FooPrivate(this))));

Foo::Foo() : d(new FooPrivate(this))
{
    const auto ctor = constructor<Foo, Null<IBarTransformation>>();
    *this = ctor(nullptr);
    subscribeEvent("PropertyChanged", d);
}

Foo::Foo(const IBarTransformation &transformation) : d(new FooPrivate(this))
{
    *this = constructor(d->ctor).invoke(*this, transformation);
    subscribeEvent("PropertyChanged", d);
}

Foo::~Foo()
{
    delete d;
}

QString Foo::bar() const
{
    return method("get_Bar", d->bar).invoke(*this);
}

void Foo::setBar(const QString &value)
{
    method("set_Bar", d->setBar).invoke(*this, value);
}

IBarTransformation::IBarTransformation() : QDotNetInterface(FullyQualifiedTypeName)
{
    setCallback<QString, QString>("Transform", { QDotNetParameter::String, UnmanagedType::LPWStr },
        [this](const QString &bar) {
            return transform(bar);
        });
}
