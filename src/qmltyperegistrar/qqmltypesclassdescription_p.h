// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLTYPESCLASSDESCRIPTION_P_H
#define QMLTYPESCLASSDESCRIPTION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qstring.h>
#include <QtCore/qcbormap.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>
#include <QtCore/qversionnumber.h>

QT_BEGIN_NAMESPACE

struct QmlTypesClassDescription
{
    // All the string views in this class are based on string data in the JSON they are parsed from.
    // You must keep the relevant QCborValues alive while the QmlTypesClassDescription exists.

    const QCborMap *resolvedClass = nullptr;
    QAnyStringView file;
    QAnyStringView className;
    QAnyStringView elementName;
    QAnyStringView defaultProp;
    QAnyStringView parentProp;
    QAnyStringView superClass;
    QAnyStringView attachedType;
    QAnyStringView extensionType;
    QAnyStringView sequenceValueType;
    QAnyStringView accessSemantics;
    QList<QTypeRevision> revisions;
    QTypeRevision addedInRevision;
    QTypeRevision removedInRevision;
    bool isCreatable = true;
    bool isStructured = false;
    bool isSingleton = false;
    bool hasCustomParser = false;
    bool omitFromQmlTypes = false;
    bool extensionIsNamespace = false;
    QList<QAnyStringView> implementsInterfaces;
    QList<QAnyStringView> deferredNames;
    QList<QAnyStringView> immediateNames;

    enum CollectMode {
        TopLevel,
        SuperClass,
        RelatedType
    };

    void collect(const QCborMap *classDef, const QVector<QCborMap> &types,
                 const QVector<QCborMap> &foreign, CollectMode mode,
                 QTypeRevision defaultRevision);
    const QCborMap *collectRelated(
            QAnyStringView related, const QVector<QCborMap> &types,
            const QVector<QCborMap> &foreign, QTypeRevision defaultRevision,
            const QList<QAnyStringView> &namespaces);

    static const QCborMap *findType(
            const QVector<QCborMap> &types, const QVector<QCborMap> &foreign,
            const QAnyStringView &name, const QList<QAnyStringView> &namespaces);

    void collectLocalAnonymous(const QCborMap *classDef,const QVector<QCborMap> &types,
                      const QVector<QCborMap> &foreign, QTypeRevision defaultRevision);


private:
    void collectSuperClasses(
            const QCborMap *classDef, const QVector<QCborMap> &types,
            const QVector<QCborMap> &foreign, CollectMode mode, QTypeRevision defaultRevision);
    void collectInterfaces(const QCborMap *classDef);
};

QT_END_NAMESPACE
#endif // QMLTYPESCLASSDESCRIPTION_P_H
