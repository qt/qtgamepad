/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Gamepad module
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGamepad/qtgamepadglobal.h>

QT_BEGIN_NAMESPACE

class QGamepadBackend;
class QGamepad;

class Q_GAMEPAD_EXPORT QGamepadManager : public QObject
{
    Q_OBJECT
    Q_FLAGS(GamepadButton GamepadButtons)
    Q_FLAGS(GamepadAxis GamepadAxes)

public:
    enum GamepadButton {
        ButtonInvalid = -1,
        ButtonA = 0,
        ButtonB,
        ButtonX,
        ButtonY,
        ButtonL1,
        ButtonR1,
        ButtonL2,
        ButtonR2,
        ButtonSelect,
        ButtonStart,
        ButtonL3,
        ButtonR3,
        ButtonUp,
        ButtonDown,
        ButtonRight,
        ButtonLeft,
        ButtonGuide
    };
    Q_DECLARE_FLAGS(GamepadButtons, GamepadButton)

    enum GamepadAxis {
        AxisInvalid = -1,
        AxisLeftX = 0,
        AxisLeftY,
        AxisRightX,
        AxisRightY
    };
    Q_DECLARE_FLAGS(GamepadAxes, GamepadAxis)

    static QGamepadManager* instance();

    bool isGamepadConnected(int index);

Q_SIGNALS:
    void gamepadConnected(int index);
    void gamepadDisconnected(int index);
    void gamepadAxisEvent(int index, QGamepadManager::GamepadAxis axis, double value);
    void gamepadButtonPressEvent(int index, QGamepadManager::GamepadButton button, double value);
    void gamepadButtonReleaseEvent(int index, QGamepadManager::GamepadButton button);

private Q_SLOTS:
    void forwardGamepadConnected(int index);
    void forwardGamepadDisconnected(int index);
    void forwardGamepadAxisEvent(int index, QGamepadManager::GamepadAxis axis, double value);
    void forwardGamepadButtonPressEvent(int index, QGamepadManager::GamepadButton button, double value);
    void forwardGamepadButtonReleaseEvent(int index, QGamepadManager::GamepadButton button);

private:
    QGamepadManager();
    ~QGamepadManager();
    QGamepadManager(QGamepadManager const&);
    void operator=(QGamepadManager const&);

    void loadBackend();

    QGamepadBackend *m_gamepadBackend;
    QList<int> m_connectedGamepads;

};

Q_DECLARE_METATYPE(QGamepadManager::GamepadButton)
Q_DECLARE_METATYPE(QGamepadManager::GamepadAxis)

QT_END_NAMESPACE

#endif // JOYSTICKMANAGER_H
