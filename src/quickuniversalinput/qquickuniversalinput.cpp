// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickuniversalinput_p.h"

#include <QUniversalInput>

#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

// Bring the QUniversalInput input enums into scope for this file.
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

class QQuickUniversalInputPrivate : public QObjectPrivate {
    Q_DECLARE_PUBLIC(QQuickUniversalInput)
public:
    QQuickUniversalInputPrivate();
};

QQuickUniversalInputPrivate::QQuickUniversalInputPrivate()
{

}

QQuickUniversalInput::QQuickUniversalInput(QObject *parent)
    : QObject(*new QQuickUniversalInputPrivate, parent)
{
    connect(QUniversalInput::instance(), &QUniversalInput::joyConnectionChanged, this, &QQuickUniversalInput::joyConnectionChanged);
    connect(QUniversalInput::instance(), &QUniversalInput::joyButtonEvent, this, &QQuickUniversalInput::joyButtonEvent);
    connect(QUniversalInput::instance(), &QUniversalInput::joyAxisEvent, this, &QQuickUniversalInput::joyAxisEvent);
    connect(QUniversalInput::instance(), &QUniversalInput::mouseMovedWithDeltas, this, &QQuickUniversalInput::mouseDeltaChanged);
}

bool QQuickUniversalInput::isMouseDisabled() const
{
    auto input = QUniversalInput::instance();
    return input->isMouseDisabled();
}

void QQuickUniversalInput::setMouseDisabled(bool disabled)
{
    auto input = QUniversalInput::instance();
    if (input->isMouseDisabled() == disabled)
        return;
    input->setMouseDisabled(disabled);
    emit mouseDisabledChanged();
}

void QQuickUniversalInput::addForce(int device, const QVector2D &force, float duration)
{
    QUniversalInput::instance()->addForce(device, force, duration);
}

void QQuickUniversalInput::joyButton(int device, JoyButton button, bool isPressed)
{
    auto input = QUniversalInput::instance();
    input->joyButton(device,button, isPressed);
}

void QQuickUniversalInput::joyAxis(int device, JoyAxis axis, float value)
{
    auto input = QUniversalInput::instance();
    input->joyAxis(device, axis, value);
}

void QQuickUniversalInput::joyHat(int device, HatMask value)
{
    auto input = QUniversalInput::instance();
    input->joyHat(device, value);
}

QT_END_NAMESPACE
