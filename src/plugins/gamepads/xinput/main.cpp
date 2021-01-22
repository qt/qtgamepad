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

#include <QtGamepad/private/qgamepadbackendfactory_p.h>
#include <QtGamepad/private/qgamepadbackendplugin_p.h>

#include "qxinputgamepadbackend_p.h"

QT_BEGIN_NAMESPACE

class QXInputGamepadBackendPlugin : public QGamepadBackendPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QtGamepadBackendFactoryInterface_iid FILE "xinput.json")
public:
    QGamepadBackend *create(const QString &key, const QStringList &paramList) override;
};

QGamepadBackend *QXInputGamepadBackendPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(key)
    Q_UNUSED(paramList)

    return new QXInputGamepadBackend;
}

QT_END_NAMESPACE

#include "main.moc"
