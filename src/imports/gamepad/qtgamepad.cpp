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

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtGamepad/QGamepad>
#include <QtGamepad/QGamepadKeyNavigation>

#include "qgamepadmouseitem.h"

QT_BEGIN_NAMESPACE

static QObject *gamepadmanager_singletontype_provider(QQmlEngine * /* engine */, QJSEngine * /* scriptEngine */)
{
    QGamepadManager *manager = QGamepadManager::instance();
    QQmlEngine::setObjectOwnership(manager, QQmlEngine::CppOwnership);
    return manager;
}

class QGamepadModule : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    QGamepadModule(QObject *parent = 0) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtGamepad"));

        //@uri QtGamepad

        qmlRegisterSingletonType<QGamepadManager>(uri, 1, 0, "GamepadManager", &gamepadmanager_singletontype_provider);
        qmlRegisterType<QGamepad>(uri, 1, 0, "Gamepad");
        qmlRegisterType<QGamepadKeyNavigation>(uri, 1, 0, "GamepadKeyNavigation");
        qmlRegisterType<QGamepadMouseItem>(uri, 1, 0, "GamepadMouseItem");

        // Register the latest Qt version as QML type version
        qmlRegisterModule(uri, 1, QT_VERSION_MINOR);
    }

    void initializeEngine(QQmlEngine *engine, const char *uri)
    {
        Q_UNUSED(uri);
        Q_UNUSED(engine);
    }
};

QT_END_NAMESPACE

#include "qtgamepad.moc"
