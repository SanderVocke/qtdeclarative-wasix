/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QV4FUNCTION_H
#define QV4FUNCTION_H

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

#include "qv4global_p.h"
#include <private/qqmlglobal_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qv4context_p.h>

QT_BEGIN_NAMESPACE

namespace QV4 {

struct Q_QML_EXPORT Function {
    const CompiledData::Function *compiledFunction;
    CompiledData::CompilationUnit *compilationUnit;

    ReturnedValue execute(Heap::ExecutionContext *c, CallData *callData, const FunctionObject *f = 0) {
        return code(f, callData, c, this);
    }
    ReturnedValue call(Heap::ExecutionContext *c, CallData *callData, const FunctionObject *f = 0) {
        return call(f, callData, c, this);
    }


    typedef ReturnedValue (*Code)(const FunctionObject *, CallData *, Heap::ExecutionContext *c, Function *);
    Code code;
    const uchar *codeData;

    // first nArguments names in internalClass are the actual arguments
    InternalClass *internalClass;
    uint nFormals;
    bool hasQmlDependencies;
    bool canUseSimpleCall;

    Function(ExecutionEngine *engine, CompiledData::CompilationUnit *unit, const CompiledData::Function *function, Code codePtr);
    ~Function();

    // used when dynamically assigning signal handlers (QQmlConnection)
    void updateInternalClass(ExecutionEngine *engine, const QList<QByteArray> &parameters);

    inline Heap::String *name() {
        return compilationUnit->runtimeStrings[compiledFunction->nameIndex];
    }
    inline QString sourceFile() const { return compilationUnit->fileName(); }

    inline bool usesArgumentsObject() const { return compiledFunction->flags & CompiledData::Function::UsesArgumentsObject; }
    inline bool isStrict() const { return compiledFunction->flags & CompiledData::Function::IsStrict; }
    inline bool isNamedExpression() const { return compiledFunction->flags & CompiledData::Function::IsNamedExpression; }

    inline bool canUseSimpleFunction() const { return canUseSimpleCall; }

    QQmlSourceLocation sourceLocation() const
    {
        return QQmlSourceLocation(sourceFile(), compiledFunction->location.line, compiledFunction->location.column);
    }

private:
    static ReturnedValue call(const FunctionObject *f, CallData *callData, Heap::ExecutionContext *context, Function *function)
    {
        if (!function->canUseSimpleCall)
            context = ExecutionContext::newCallContext(context, function, callData, f);

        return function->execute(context, callData, f);
    }
};


inline unsigned int Heap::CallContext::formalParameterCount() const
{
    Q_ASSERT(v4Function);
    return v4Function->nFormals;
}


}

QT_END_NAMESPACE

#endif
