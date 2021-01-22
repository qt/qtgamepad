/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Gamepad module
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qgamepadbackendfactory_p.h"
#include "qgamepadbackendplugin_p.h"
#include "qgamepadbackend_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QtGamepadBackendFactoryInterface_iid, QLatin1String("/gamepads"), Qt::CaseInsensitive))
#if QT_CONFIG(library)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader, (QtGamepadBackendFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))
#endif

QStringList QGamepadBackendFactory::keys(const QString &pluginPath)
{
    QStringList list;
    if (!pluginPath.isEmpty()) {
#if QT_CONFIG(library)
        QCoreApplication::addLibraryPath(pluginPath);
        list = directLoader()->keyMap().values();
        if (!list.isEmpty()) {
            const QString postFix = QStringLiteral(" (from ")
                    + QDir::toNativeSeparators(pluginPath)
                    + QLatin1Char(')');
            const QStringList::iterator end = list.end();
            for (QStringList::iterator it = list.begin(); it != end; ++it)
                (*it).append(postFix);
        }
#else
        qWarning("Cannot query QGamepadBackend plugins at %s: Library loading is disabled.",
                 pluginPath.toLocal8Bit().constData());
#endif
    }
    list.append(loader()->keyMap().values());
    return list;
}

QGamepadBackend *QGamepadBackendFactory::create(const QString &name, const QStringList &args, const QString &pluginPath)
{
    if (!pluginPath.isEmpty()) {
#if QT_CONFIG(library)
        QCoreApplication::addLibraryPath(pluginPath);
        if (QGamepadBackend *ret = qLoadPlugin<QGamepadBackend, QGamepadBackendPlugin>(directLoader(), name, args))
            return ret;
#else
        qWarning("Cannot load QGamepadBackend plugin from %s. Library loading is disabled.",
                 pluginPath.toLocal8Bit().constData());
#endif
    }
    if (QGamepadBackend *ret = qLoadPlugin<QGamepadBackend, QGamepadBackendPlugin>(loader(), name, args))
        return ret;
    return 0;
}

QT_END_NAMESPACE
