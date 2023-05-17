/***************************************************************************************************
 Copyright (C) 2023 The Qt Company Ltd.
 SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
***************************************************************************************************/

#pragma once

#include <qdotnetarray.h>
#include <qdotnetmarshal.h>
#include <qdotnetobject.h>

struct UriPrivate;

namespace QDotNetTypes
{
    namespace System
    {
        enum class UriHostNameType
        {
            Unknown = 0,
            Basic = 1,
            Dns = 2,
            IPv4 = 3,
            IPv6 = 4
        };

        enum class UriKind
        {
            RelativeOrAbsolute = 0,
            Absolute = 1,
            Relative = 2
        };
    }
}

template<>
struct QDotNetTypeOf<QDotNetTypes::System::UriHostNameType>
{
    static inline const QString TypeName = QString("System.UriHostNameType, System");
    static inline UnmanagedType MarshalAs = UnmanagedType::I4;
};

template<>
struct QDotNetTypeOf<QDotNetTypes::System::UriKind>
{
    static inline const QString TypeName = QString("System.UriKind, System");
    static inline UnmanagedType MarshalAs = UnmanagedType::I4;
};

class Uri : public QDotNetObject
{
public:
    Q_DOTNET_OBJECT(Uri, "System.Uri, System");
    Uri();
    Uri(const char *uriString) : Uri(QString(uriString)) {}
    Uri(
        const QString &uriString,
        QDotNetTypes::System::UriKind uriKind = QDotNetTypes::System::UriKind::Absolute);
    Uri(const Uri &baseUri, const QString &relativeUri);
    Uri(const Uri &baseUri, const Uri &relativeUri);
    ~Uri() override;

    QString absolutePath() const;
    QString absoluteUri() const;
    QString authority() const;
    QString dnsSafeHost() const;
    QString fragment() const;
    QString host() const;
    QDotNetTypes::System::UriHostNameType hostNameType() const;
    QString idnHost() const;
    bool isAbsoluteUri() const;
    bool isDefaultPort() const;
    bool isFile() const;
    bool isLoopback() const;
    bool isUnc() const;
    QString localPath() const;
    QString originalString() const;
    QString pathAndQuery() const;
    qint32 port() const;
    QString query() const;
    QString scheme() const;
    QDotNetArray<QString> segments() const;
    bool userEscaped() const;
    QString userInfo() const;


private:
    UriPrivate *d;
};
