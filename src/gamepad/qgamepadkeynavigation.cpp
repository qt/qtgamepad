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

#include "qgamepadkeynavigation.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGamepad/QGamepad>

#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QGamepadKeyNavigationPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGamepadKeyNavigation)
public:
    QGamepadKeyNavigationPrivate()
        : active(true)
        , gamepad(nullptr)
        , buttonL2Pressed(false)
        , buttonR2Pressed(false)
    {
    }

    void sendGeneratedKeyEvent(QKeyEvent *event);

    bool active;
    QGamepad *gamepad;
    QGamepadManager *gamepadManger;
    bool buttonL2Pressed;
    bool buttonR2Pressed;
    QMap<QGamepadManager::GamepadButton, Qt::Key> keyMapping;

    void _q_processGamepadButtonPressEvent(int index, QGamepadManager::GamepadButton button, double value);
    void _q_processGamepadButtonReleaseEvent(int index, QGamepadManager::GamepadButton button);
};

void QGamepadKeyNavigationPrivate::sendGeneratedKeyEvent(QKeyEvent *event)
{
    if (!active) {
        delete event;
        return;
    }
    const QGuiApplication *app = qApp;
    QWindow *focusWindow = app ? app->focusWindow() : nullptr;
    if (focusWindow)
        QGuiApplication::sendEvent(focusWindow, event);
}

void QGamepadKeyNavigationPrivate::_q_processGamepadButtonPressEvent(int index, QGamepadManager::GamepadButton button, double value)
{
    Q_UNUSED(value)
    //If a gamepad has been set then, only use the events of that gamepad
    if (gamepad && gamepad->deviceId() != index)
        return;

    //Trigger buttons are a special case as they get multiple press events as the value changes
    if (button == QGamepadManager::ButtonL2 && buttonL2Pressed)
        return;
    else
        buttonL2Pressed = true;
    if (button == QGamepadManager::ButtonR2 && buttonR2Pressed)
        return;
    else
        buttonR2Pressed = true;

    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, keyMapping[button], Qt::NoModifier);
    sendGeneratedKeyEvent(event);
}

void QGamepadKeyNavigationPrivate::_q_processGamepadButtonReleaseEvent(int index, QGamepadManager::GamepadButton button)
{
    //If a gamepad has been set then, only use the events of that gamepad
    if (gamepad && gamepad->deviceId() != index)
        return;

    //Free the trigger buttons if necessary
    if (button == QGamepadManager::ButtonL2)
        buttonL2Pressed = false;
    if (button == QGamepadManager::ButtonR2)
        buttonR2Pressed = false;

    QKeyEvent *event = new QKeyEvent(QEvent::KeyRelease, keyMapping[button], Qt::NoModifier);
    sendGeneratedKeyEvent(event);
}

QGamepadKeyNavigation::QGamepadKeyNavigation(QObject *parent)
    : QObject(*new QGamepadKeyNavigationPrivate(), parent)
{
    Q_D(QGamepadKeyNavigation);
    d->gamepadManger = QGamepadManager::instance();

    //Default keymap
    d->keyMapping.insert(QGamepadManager::ButtonUp, Qt::Key_Up);
    d->keyMapping.insert(QGamepadManager::ButtonDown, Qt::Key_Down);
    d->keyMapping.insert(QGamepadManager::ButtonLeft, Qt::Key_Left);
    d->keyMapping.insert(QGamepadManager::ButtonRight, Qt::Key_Right);
    d->keyMapping.insert(QGamepadManager::ButtonA, Qt::Key_Return);
    d->keyMapping.insert(QGamepadManager::ButtonB, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonX, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonY, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonSelect, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonStart, Qt::Key_Return);
    d->keyMapping.insert(QGamepadManager::ButtonGuide, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonL1, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonR1, Qt::Key_Forward);
    d->keyMapping.insert(QGamepadManager::ButtonL2, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonR2, Qt::Key_Forward);
    d->keyMapping.insert(QGamepadManager::ButtonL3, Qt::Key_Back);
    d->keyMapping.insert(QGamepadManager::ButtonR3, Qt::Key_Forward);

    connect(d->gamepadManger, SIGNAL(gamepadButtonPressEvent(int,QGamepadManager::GamepadButton,double)),
            this, SLOT(_q_processGamepadButtonPressEvent(int,QGamepadManager::GamepadButton,double)));
    connect(d->gamepadManger, SIGNAL(gamepadButtonReleaseEvent(int,QGamepadManager::GamepadButton)),
            this, SLOT(_q_processGamepadButtonReleaseEvent(int,QGamepadManager::GamepadButton)));
}

bool QGamepadKeyNavigation::active() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->active;
}

QGamepad *QGamepadKeyNavigation::gamepad() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->gamepad;
}

Qt::Key QGamepadKeyNavigation::upKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonUp];
}

Qt::Key QGamepadKeyNavigation::downKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonDown];
}

Qt::Key QGamepadKeyNavigation::leftKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonLeft];
}

Qt::Key QGamepadKeyNavigation::rightKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonRight];
}

Qt::Key QGamepadKeyNavigation::buttonAKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonA];
}

Qt::Key QGamepadKeyNavigation::buttonBKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonB];
}

Qt::Key QGamepadKeyNavigation::buttonXKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonX];
}

Qt::Key QGamepadKeyNavigation::buttonYKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonY];
}

Qt::Key QGamepadKeyNavigation::buttonSelectKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonSelect];
}

Qt::Key QGamepadKeyNavigation::buttonStartKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonStart];
}

Qt::Key QGamepadKeyNavigation::buttonGuideKey() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonGuide];
}

Qt::Key QGamepadKeyNavigation::buttonL1Key() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonL1];
}

Qt::Key QGamepadKeyNavigation::buttonR1Key() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonL2];
}

Qt::Key QGamepadKeyNavigation::buttonL2Key() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonL2];
}

Qt::Key QGamepadKeyNavigation::buttonR2Key() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonL2];
}

Qt::Key QGamepadKeyNavigation::buttonL3Key() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonL3];
}

Qt::Key QGamepadKeyNavigation::buttonR3Key() const
{
    Q_D(const QGamepadKeyNavigation);
    return d->keyMapping[QGamepadManager::ButtonL3];
}

void QGamepadKeyNavigation::setActive(bool isActive)
{
    Q_D(QGamepadKeyNavigation);
    if (d->active != isActive) {
        d->active = isActive;
        emit activeChanged(isActive);
    }
}

void QGamepadKeyNavigation::setGamepad(QGamepad *gamepad)
{
    Q_D(QGamepadKeyNavigation);
    if (d->gamepad != gamepad) {
        d->gamepad = gamepad;
        emit gamepadChanged(gamepad);
    }
}

void QGamepadKeyNavigation::setUpKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonUp] != key) {
        d->keyMapping[QGamepadManager::ButtonUp] = key;
        emit upKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setDownKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonDown] != key) {
        d->keyMapping[QGamepadManager::ButtonDown] = key;
        emit downKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setLeftKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonLeft] != key) {
        d->keyMapping[QGamepadManager::ButtonLeft] = key;
        emit leftKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setRightKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonRight] != key) {
        d->keyMapping[QGamepadManager::ButtonRight] = key;
        emit rightKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonAKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonA] != key) {
        d->keyMapping[QGamepadManager::ButtonA] = key;
        emit buttonAKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonBKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonB] != key) {
        d->keyMapping[QGamepadManager::ButtonB] = key;
        emit buttonBKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonXKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonX] != key) {
        d->keyMapping[QGamepadManager::ButtonX] = key;
        emit buttonXKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonYKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonY] != key) {
        d->keyMapping[QGamepadManager::ButtonY] = key;
        emit buttonYKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonSelectKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonSelect] != key) {
        d->keyMapping[QGamepadManager::ButtonSelect] = key;
        emit buttonSelectKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonStartKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonStart] != key) {
        d->keyMapping[QGamepadManager::ButtonStart] = key;
        emit buttonStartKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonGuideKey(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonGuide] != key) {
        d->keyMapping[QGamepadManager::ButtonGuide] = key;
        emit buttonGuideKeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonL1Key(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonL1] != key) {
        d->keyMapping[QGamepadManager::ButtonL1] = key;
        emit buttonL1KeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonR1Key(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonR1] != key) {
        d->keyMapping[QGamepadManager::ButtonR1] = key;
        emit buttonR1KeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonL2Key(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonL2] != key) {
        d->keyMapping[QGamepadManager::ButtonL2] = key;
        emit buttonL1KeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonR2Key(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonR2] != key) {
        d->keyMapping[QGamepadManager::ButtonR2] = key;
        emit buttonR1KeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonL3Key(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonL3] != key) {
        d->keyMapping[QGamepadManager::ButtonL3] = key;
        emit buttonL1KeyChanged(key);
    }
}

void QGamepadKeyNavigation::setButtonR3Key(Qt::Key key)
{
    Q_D(QGamepadKeyNavigation);
    if (d->keyMapping[QGamepadManager::ButtonR3] != key) {
        d->keyMapping[QGamepadManager::ButtonR3] = key;
        emit buttonR1KeyChanged(key);
    }
}



QT_END_NAMESPACE

#include "moc_qgamepadkeynavigation.cpp"
