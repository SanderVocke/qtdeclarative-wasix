// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQMLCOMPONENT_P_H
#define QQMLCOMPONENT_P_H

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

#include "qqmlcomponent.h"

#include "qqmlengine_p.h"
#include "qqmlerror.h"
#include <private/qqmlobjectcreator_p.h>
#include <private/qqmltypedata_p.h>
#include <private/qqmlguardedcontextdata_p.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QQmlComponent;
class QQmlEngine;

class QQmlComponentAttached;
class Q_QML_PRIVATE_EXPORT QQmlComponentPrivate : public QObjectPrivate, public QQmlTypeData::TypeDataCallback
{
    Q_DECLARE_PUBLIC(QQmlComponent)

public:
    QQmlComponentPrivate()
        : progress(0.), start(-1), engine(nullptr) {}

    void loadUrl(const QUrl &newUrl, QQmlComponent::CompilationMode mode = QQmlComponent::PreferSynchronous);

    QObject *beginCreate(QQmlRefPointer<QQmlContextData>);
    void completeCreate();
    void initializeObjectWithInitialProperties(QV4::QmlContext *qmlContext, const QV4::Value &valuemap, QObject *toCreate, RequiredProperties *requiredProperties);
    static void setInitialProperties(QV4::ExecutionEngine *engine, QV4::QmlContext *qmlContext, const QV4::Value &o, const QV4::Value &v, RequiredProperties *requiredProperties, QObject *createdComponent);
    static QQmlError unsetRequiredPropertyToQQmlError(const RequiredPropertyInfo &unsetRequiredProperty);

    virtual void incubateObject(
            QQmlIncubator *incubationTask,
            QQmlComponent *component,
            QQmlEngine *engine,
            const QQmlRefPointer<QQmlContextData> &context,
            const QQmlRefPointer<QQmlContextData> &forContext);

    QQmlRefPointer<QQmlTypeData> typeData;
    void typeDataReady(QQmlTypeData *) override;
    void typeDataProgress(QQmlTypeData *, qreal) override;

    void fromTypeData(const QQmlRefPointer<QQmlTypeData> &data);

    QUrl url;
    qreal progress;

    int start;
    bool hadTopLevelRequiredProperties() const;
    // TODO: merge compilation unit and type
    QQmlRefPointer<QV4::ExecutableCompilationUnit> compilationUnit;
    QQmlType loadedType;

    struct AnnotatedQmlError
    {
        AnnotatedQmlError() = default;

        AnnotatedQmlError(QQmlError error)
            : error(std::move(error))
        {
        }


        AnnotatedQmlError(QQmlError error, bool transient)
            : error(std::move(error)), isTransient(transient)
        {
        }
        QQmlError error;
        bool isTransient = false; // tells if the error is temporary (e.g. unset required property)
    };

    struct ConstructionState {
        inline RequiredProperties *requiredProperties();
        inline void addPendingRequiredProperty(const QQmlPropertyData *propData, const RequiredPropertyInfo &info);
        inline bool hasUnsetRequiredProperties() const;
        inline void clearRequiredProperties();

        inline void appendErrors(const QList<QQmlError> &qmlErrors);
        inline void appendCreatorErrors();

        inline QQmlObjectCreator *creator();
        inline const QQmlObjectCreator *creator() const;
        inline void clear();
        inline bool hasCreator() const;
        inline QQmlObjectCreator *initCreator(QQmlRefPointer<QQmlContextData> parentContext,
                         const QQmlRefPointer<QV4::ExecutableCompilationUnit> &compilationUnit,
                         const QQmlRefPointer<QQmlContextData> &creationContext);

        QList<AnnotatedQmlError> errors;
        inline bool isCompletePending() const;
        inline void setCompletePending(bool isPending);

        private:
        bool m_completePending = false;
        RequiredProperties m_requiredProperties; // todo: union with another member
        std::unique_ptr<QQmlObjectCreator> m_creator;
    };
    ConstructionState state;

    using DeferredState = std::vector<ConstructionState>;
    static void beginDeferred(QQmlEnginePrivate *enginePriv, QObject *object, DeferredState* deferredState);
    static void completeDeferred(QQmlEnginePrivate *enginePriv, DeferredState *deferredState);

    static void complete(QQmlEnginePrivate *enginePriv, ConstructionState *state);
    static QQmlProperty removePropertyFromRequired(QObject *createdComponent, const QString &name, RequiredProperties *requiredProperties,
            QQmlEngine *engine, bool *wasInRequiredProperties = nullptr);

    QQmlEngine *engine;
    QQmlGuardedContextData creationContext;

    void clear();

    static QQmlComponentPrivate *get(QQmlComponent *c) {
        return static_cast<QQmlComponentPrivate *>(QObjectPrivate::get(c));
    }

    QObject *doBeginCreate(QQmlComponent *q, QQmlContext *context);
    bool setInitialProperty(QObject *component, const QString &name, const QVariant& value);

    enum CreateBehavior {
        CreateDefault,
        CreateWarnAboutRequiredProperties,
    };
    QObject *createWithProperties(QObject *parent, const QVariantMap &properties,
                                  QQmlContext *context, CreateBehavior behavior = CreateDefault);

    bool isBound() const {
        return compilationUnit->unitData()->flags & QV4::CompiledData::Unit::ComponentsBound;
    }
};


/*!
   \internal A list of pending required properties that need
   to be set in order for object construction to be successful.
 */
inline RequiredProperties *QQmlComponentPrivate::ConstructionState::requiredProperties() {
    if (hasCreator())
        return m_creator->requiredProperties();
    else
        return &m_requiredProperties;
}

inline void QQmlComponentPrivate::ConstructionState::addPendingRequiredProperty(const QQmlPropertyData *propData, const RequiredPropertyInfo &info)
{
    Q_ASSERT(requiredProperties());
    requiredProperties()->insert(propData, info);
}

inline bool QQmlComponentPrivate::ConstructionState::hasUnsetRequiredProperties() const {
    return !const_cast<ConstructionState *>(this)->requiredProperties()->isEmpty();
}

inline void QQmlComponentPrivate::ConstructionState::clearRequiredProperties()
{
    if (auto reqProps = requiredProperties())
        reqProps->clear();
}

inline void QQmlComponentPrivate::ConstructionState::appendErrors(const QList<QQmlError> &qmlErrors)
{
    for (const QQmlError &e : qmlErrors)
        errors.emplaceBack(e);
}

//! \internal Moves errors from creator into construction state itself
inline void QQmlComponentPrivate::ConstructionState::appendCreatorErrors()
{
    if (!hasCreator())
        return;
    auto creatorErrorCount = creator()->errors.size();
    if (creatorErrorCount == 0)
        return;
    auto existingErrorCount = errors.size();
    errors.resize(existingErrorCount + creatorErrorCount);
    for (qsizetype i = 0; i < creatorErrorCount; ++i)
        errors[existingErrorCount + i] = AnnotatedQmlError { std::move(creator()->errors[i]) };
    creator()->errors.clear();
}

inline QQmlObjectCreator *QQmlComponentPrivate::ConstructionState::creator()
{
    return m_creator.get();
}

inline const QQmlObjectCreator *QQmlComponentPrivate::ConstructionState::creator() const
{
    return m_creator.get();
}

inline bool QQmlComponentPrivate::ConstructionState::hasCreator() const { return m_creator != nullptr; }

inline void QQmlComponentPrivate::ConstructionState::clear() { m_creator.reset(); }

inline QQmlObjectCreator *QQmlComponentPrivate::ConstructionState::initCreator(QQmlRefPointer<QQmlContextData> parentContext, const QQmlRefPointer<QV4::ExecutableCompilationUnit> &compilationUnit, const QQmlRefPointer<QQmlContextData> &creationContext)
{
    m_creator.reset(new QQmlObjectCreator(
                        std::move(parentContext), compilationUnit,
                        creationContext));
    return m_creator.get();
}

inline bool QQmlComponentPrivate::ConstructionState::isCompletePending() const
{
    return m_completePending;
}

inline void QQmlComponentPrivate::ConstructionState::setCompletePending(bool isPending)
{
    m_completePending = isPending;
}

QT_END_NAMESPACE

#endif // QQMLCOMPONENT_P_H
