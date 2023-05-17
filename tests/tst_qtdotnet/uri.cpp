/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#include "uri.h"

using namespace QDotNetTypes::System;

template<>
struct QDotNetOutbound<UriHostNameType>
{
    using SourceType = UriHostNameType;
    using OutboundType = qint32;
    static inline const QDotNetParameter Parameter = QDotNetParameter(
        QDotNetTypeOf<UriHostNameType>::TypeName,
        QDotNetTypeOf<UriHostNameType>::MarshalAs);
    static OutboundType convert(SourceType srvValue)
    {
        return static_cast<qint32>(srvValue);
    }
};

template<>
struct QDotNetInbound<UriHostNameType>
{
    using InboundType = qint32;
    using TargetType = UriHostNameType;
    static inline const QDotNetParameter Parameter = QDotNetParameter(
        QDotNetTypeOf<UriHostNameType>::TypeName,
        QDotNetTypeOf<UriHostNameType>::MarshalAs);
    static TargetType convert(InboundType inboundValue)
    {
        return static_cast<UriHostNameType>(inboundValue);
    }
};

template<>
struct QDotNetNull<UriHostNameType>
{
    static UriHostNameType value() { return UriHostNameType::Unknown; }
    static bool isNull(const UriHostNameType &) { return false; }
};

template<>
struct QDotNetOutbound<UriKind>
{
    using SourceType = UriKind;
    using OutboundType = qint32;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<UriKind>::TypeName, QDotNetTypeOf<UriKind>::MarshalAs);
    static OutboundType convert(SourceType srvValue)
    {
        return static_cast<qint32>(srvValue);
    }
};

template<>
struct QDotNetInbound<UriKind>
{
    using InboundType = qint32;
    using TargetType = UriKind;
    static inline const QDotNetParameter Parameter =
        QDotNetParameter(QDotNetTypeOf<UriKind>::TypeName, QDotNetTypeOf<UriKind>::MarshalAs);
    static TargetType convert(InboundType inboundValue)
    {
        return static_cast<UriKind>(inboundValue);
    }
};

template<>
struct QDotNetNull<UriKind>
{
    static UriKind value() { return UriKind::RelativeOrAbsolute; }
    static bool isNull(const UriKind &) { return false; }
};

struct UriPrivate
{
    QDotNetSafeMethod<QString> absolutePath;
    QDotNetSafeMethod<QString> absoluteUri;
    QDotNetSafeMethod<QString> authority;
    QDotNetSafeMethod<QString> dnsSafeHost;
    QDotNetSafeMethod<QString> fragment;
    QDotNetSafeMethod<QString> host;
    QDotNetSafeMethod<UriHostNameType> hostNameType;
    QDotNetSafeMethod<QString> idnHost;
    QDotNetSafeMethod<bool> isAbsoluteUri;
    QDotNetSafeMethod<bool> isDefaultPort;
    QDotNetSafeMethod<bool> isFile;
    QDotNetSafeMethod<bool> isLoopback;
    QDotNetSafeMethod<bool> isUnc;
    QDotNetSafeMethod<QString> localPath;
    QDotNetSafeMethod<QString> originalString;
    QDotNetSafeMethod<QString> pathAndQuery;
    QDotNetSafeMethod<qint32> port;
    QDotNetSafeMethod<QString> query;
    QDotNetSafeMethod<QString> scheme;
    QDotNetSafeMethod<QDotNetArray<QString>> segments;
    QDotNetSafeMethod<bool> userEscaped;
    QDotNetSafeMethod<QString> userInfo;
};

Q_DOTNET_OBJECT_IMPL(Uri,
    Q_DOTNET_OBJECT_INIT(d(new UriPrivate)));

Uri::Uri() : d(new UriPrivate)
{}

Uri::Uri(const QString &uriString, UriKind uriKind)
    : d(new UriPrivate)
{
    const auto ctor = constructor<Uri, QString, UriKind>();
    *this = ctor(uriString, uriKind);
}

Uri::Uri(const Uri &baseUri, const QString &relativeUri)
    : d(new UriPrivate)
{
    const auto ctor = constructor<Uri, Uri, QString>();
    *this = ctor(&baseUri, relativeUri);
}

Uri::Uri(const Uri &baseUri, const Uri &relativeUri)
    : d(new UriPrivate)
{
    const auto ctor = constructor<Uri, Uri, Uri>();
    *this = ctor(&baseUri, &relativeUri);
}

Uri::~Uri()
{
    delete d;
}

QString Uri::absolutePath() const
{
    return method("get_AbsolutePath", d->absolutePath).invoke(*this);
}

QString Uri::absoluteUri() const
{
    return method("get_AbsoluteUri", d->absoluteUri).invoke(*this);
}

QString Uri::authority() const
{
    return method("get_Authority", d->authority).invoke(*this);
}

QString Uri::dnsSafeHost() const
{
    return method("get_DnsSafeHost", d->dnsSafeHost).invoke(*this);
}

QString Uri::fragment() const
{
    return method("get_Fragment", d->fragment).invoke(*this);
}

QString Uri::host() const
{
    return method("get_Host", d->host).invoke(*this);
}

UriHostNameType Uri::hostNameType() const
{
    return method("get_HostNameType", d->hostNameType).invoke(*this);
}

QString Uri::idnHost() const
{
    return method("get_IdnHost", d->idnHost).invoke(*this);
}

bool Uri::isAbsoluteUri() const
{
    return method("get_IsAbsoluteUri", d->isAbsoluteUri).invoke(*this);
}

bool Uri::isDefaultPort() const
{
    return method("get_IsDefaultPort", d->isDefaultPort).invoke(*this);
}

bool Uri::isFile() const
{
    return method("get_IsFile", d->isFile).invoke(*this);
}

bool Uri::isLoopback() const
{
    return method("get_IsLoopback", d->isLoopback).invoke(*this);
}

bool Uri::isUnc() const
{
    return method("get_IsUnc", d->isUnc).invoke(*this);
}

QString Uri::localPath() const
{
    return method("get_LocalPath", d->localPath).invoke(*this);
}

QString Uri::originalString() const
{
    return method("get_OriginalString", d->originalString).invoke(*this);
}

QString Uri::pathAndQuery() const
{
    return method("get_PathAndQuery", d->pathAndQuery).invoke(*this);
}

qint32 Uri::port() const
{
    return method("get_Port", d->port).invoke(*this);
}

QString Uri::query() const
{
    return method("get_Query", d->query).invoke(*this);
}

QString Uri::scheme() const
{
    return method("get_Scheme", d->scheme).invoke(*this);
}

QDotNetArray<QString> Uri::segments() const
{
    return method("get_Segments", d->segments).invoke(*this);
}

bool Uri::userEscaped() const
{
    return method("get_UserEscaped", d->userEscaped).invoke(*this);
}

QString Uri::userInfo() const
{
    return method("get_UserInfo", d->userInfo).invoke(*this);
}
