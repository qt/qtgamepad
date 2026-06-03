// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTQUICKUNIVERSALINPUT_H
#define QTQUICKUNIVERSALINPUT_H

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

#include <QtQml/QQmlEngine>

#include <QtUniversalInput/QUniversalInput>
#include <QtQuick/QQuickItem>
#include <QVector2D>

QT_BEGIN_NAMESPACE

class QQuickUniversalInputPrivate;

class QQuickUniversalInput : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(UniversalInput)
    Q_PROPERTY(bool mouseDisabled READ isMouseDisabled WRITE setMouseDisabled NOTIFY mouseDisabledChanged)

public:
    QQuickUniversalInput(QObject *parent = nullptr);
    ~QQuickUniversalInput() override = default;

    bool isMouseDisabled() const;
    void setMouseDisabled(bool disabled);

Q_SIGNALS:
    void joyConnectionChanged(int index, bool isConnected);
    void joyButtonEvent(int device, QUniversalInput::JoyButton button, bool isPressed);
    void joyAxisEvent(int device, QUniversalInput::JoyAxis axis, float value);

    void mouseDisabledChanged();
    void mouseDeltaChanged(const QVector2D& delta);

public Q_SLOTS:
    void addForce(int device, const QVector2D& force, float duration);

    void joyButton(int device, QUniversalInput::JoyButton button, bool isPressed);
    void joyAxis(int device, QUniversalInput::JoyAxis axis, float value);
    void joyHat(int device, QUniversalInput::HatMask value);


private:
    Q_DISABLE_COPY(QQuickUniversalInput)
    Q_DECLARE_PRIVATE(QQuickUniversalInput)
};

QT_END_NAMESPACE

#endif // QTQUICKUNIVERSALINPUT_H
