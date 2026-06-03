// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgamepad.h"

#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

// Bring the QUniversalInput input enums into scope for this file.
using HatDirection = QUniversalInput::HatDirection;
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

class QGamepadPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGamepad)

public:
    QGamepadPrivate(int deviceId)
        : deviceId(deviceId)
    {
    }

    int deviceId = -1;
    bool connected = false;
    QString name;
    float axisLeftX = 0.0f;
    float axisLeftY = 0.0f;
    float axisRightX = 0.0f;
    float axisRightY = 0.0f;
    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    bool buttonL1 = false;
    bool buttonR1 = false;
    float buttonL2 = 0.0f;
    float buttonR2 = 0.0f;
    bool buttonSelect = false;
    bool buttonStart = false;
    bool buttonL3 = false;
    bool buttonR3 = false;
    bool buttonUp = false;
    bool buttonDown = false;
    bool buttonLeft = false;
    bool buttonRight = false;
    bool buttonGuide = false;

    void setConnected(bool isConnected);
    void setName(const QString &name);

    void _q_handleGamepadConnectionChangedEvent(int index, bool isConnected);
    void _q_handleGamepadAxisEvent(int device, JoyAxis axis, float value);
    void _q_handleGamepadButtonEvent(int device, JoyButton button, bool isPressed);
};

void QGamepadPrivate::setConnected(bool isConnected)
{
    Q_Q(QGamepad);
    if (connected != isConnected) {
        connected = isConnected;
        emit q->connectedChanged();
    }
}

void QGamepadPrivate::setName(const QString &theName)
{
    Q_Q(QGamepad);
    if (name != theName) {
        name = theName;
        emit q->nameChanged();
    }
}

/*!
 * \internal
 */
void QGamepadPrivate::_q_handleGamepadConnectionChangedEvent(int index, bool isConnected)
{
    if (index == deviceId)
        setConnected(isConnected);
}

/*!
 * \internal
 */
void QGamepadPrivate::_q_handleGamepadAxisEvent(int device, JoyAxis axis, float value)
{
    Q_Q(QGamepad);
    if (device != deviceId)
        return;

    switch (axis) {
    case JoyAxis::LeftX:
        axisLeftX = value;
        emit q->axisLeftXChanged();
        break;
    case JoyAxis::LeftY:
        axisLeftY = value;
        emit q->axisLeftYChanged();
        break;
    case JoyAxis::RightX:
        axisRightX = value;
        emit q->axisRightXChanged();
        break;
    case JoyAxis::RightY:
        axisRightY = value;
        emit q->axisRightYChanged();
        break;
    case JoyAxis::TriggerLeft:
        buttonL2 = value;
        emit q->buttonL2Changed();
        break;
    case JoyAxis::TriggerRight:
        buttonR2 = value;
        emit q->buttonR2Changed();
        break;
    default:
        break;
    }
}

/*!
 * \internal
 */
void QGamepadPrivate::_q_handleGamepadButtonEvent(int device, JoyButton button, bool isPressed)
{
    Q_Q(QGamepad);
    if (device != deviceId)
        return;

    switch (button) {
    case JoyButton::A:
        buttonA = isPressed;
        emit q->buttonAChanged();
        break;
    case JoyButton::B:
        buttonB = isPressed;
        emit q->buttonBChanged();
        break;
    case JoyButton::X:
        buttonX = isPressed;
        emit q->buttonXChanged();
        break;
    case JoyButton::Y:
        buttonY = isPressed;
        emit q->buttonYChanged();
        break;
    case JoyButton::LeftShoulder:
        buttonL1 = isPressed;
        emit q->buttonL1Changed();
        break;
    case JoyButton::RightShoulder:
        buttonR1 = isPressed;
        emit q->buttonR1Changed();
        break;
    case JoyButton::LeftStick:
        buttonL3 = isPressed;
        emit q->buttonL3Changed();
        break;
    case JoyButton::RightStick:
        buttonR3 = isPressed;
        emit q->buttonR3Changed();
        break;
    case JoyButton::Back:
        buttonSelect = isPressed;
        emit q->buttonSelectChanged();
        break;
    case JoyButton::Start:
        buttonStart = isPressed;
        emit q->buttonStartChanged();
        break;
    case JoyButton::DpadUp:
        buttonUp = isPressed;
        emit q->buttonUpChanged();
        break;
    case JoyButton::DpadDown:
        buttonDown = isPressed;
        emit q->buttonDownChanged();
        break;
    case JoyButton::DpadLeft:
        buttonLeft = isPressed;
        emit q->buttonLeftChanged();
        break;
    case JoyButton::DpadRight:
        buttonRight = isPressed;
        emit q->buttonRightChanged();
        break;
    case JoyButton::Guide:
        buttonGuide = isPressed;
        emit q->buttonGuideChanged();
        break;
    default:
        break;
    }
}


/*!
   \class QGamepad
   \inmodule QtGamepad
   \brief A gamepad device connected to a system.

   QGamepad is used to access the current state of gamepad hardware connected
   to a system.
 */

/*!
 *  \qmltype Gamepad
 *  \inqmlmodule QtGamepad
 *  \brief A gamepad device connected to a system.
 *  \instantiates QGamepad
 *
 *  Gamepad QML type is used to access the current state of gamepad
 *  hardware connected to a system.
 */

/*!
 * Constructs a QGamepad with the given \a deviceId and \a parent.
 */
QGamepad::QGamepad(int deviceId, QObject *parent)
    : QObject(*new QGamepadPrivate(deviceId), parent)
{
    auto *input = QUniversalInput::instance();

    Q_D(QGamepad);
    QObjectPrivate::connect(input, &QUniversalInput::joyConnectionChanged,
                            d, &QGamepadPrivate::_q_handleGamepadConnectionChangedEvent);
    QObjectPrivate::connect(input, &QUniversalInput::joyAxisEvent,
                            d, &QGamepadPrivate::_q_handleGamepadAxisEvent);
    QObjectPrivate::connect(input, &QUniversalInput::joyButtonEvent,
                            d, &QGamepadPrivate::_q_handleGamepadButtonEvent);

    d->setConnected(input->isJoyConnected(deviceId));
    d->setName(input->joyName(deviceId));
}

QGamepad::~QGamepad()
{
}

/*!
 * \property QGamepad::deviceId
 *
 * This property holds the deviceId of the gamepad device. Multiple gamepad devices can be
 * connected at any given time, so setting this property defines which gamepad to use.
 *
 * \sa QGamepadManager::connectedGamepads()
 */
/*!
 * \qmlproperty int Gamepad::deviceId
 *
 * This property holds the deviceId of the gamepad device. Multiple gamepad devices can be
 * connected at any given time, so setting this property defines which gamepad to use.
 *
 * \sa {GamepadManager::connectedGamepads}{GamepadManager.connectedGamepads}
 */
int QGamepad::deviceId() const
{
    Q_D(const QGamepad);
    return d->deviceId;
}

/*!
 * \property QGamepad::connected
 *
 * The connectivity state of the gamepad device.
 * If a gamepad is connected, this property will be \c true, otherwise \c false.
 */
/*!
 * \qmlproperty bool Gamepad::connected
 * \readonly
 *
 * The connectivity state of the gamepad device.
 * If a gamepad is connected, this property will be \c true, otherwise \c false.
 */
bool QGamepad::isConnected() const
{
    Q_D(const QGamepad);
    return d->connected;
}

/*!
 * \property QGamepad::name
 *
 * The reported name of the gamepad if one is available.
 */
/*!
 * \qmlproperty string Gamepad::name
 * \readonly
 *
 * The reported name of the gamepad if one is available.
 */
QString QGamepad::name() const
{
    Q_D(const QGamepad);
    return d->name;
}

/*!
 * \property QGamepad::axisLeftX
 *
 * The value of the left thumbstick's X axis.
 * The axis values range from -1.0 to 1.0.
 */
/*!
 * \qmlproperty float Gamepad::axisLeftX
 * \readonly
 *
 * The value of the left thumbstick's X axis.
 * The axis values range from -1.0 to 1.0.
 */
float QGamepad::axisLeftX() const
{
    Q_D(const QGamepad);
    return d->axisLeftX;
}

/*!
 * \property QGamepad::axisLeftY
 *
 * The value of the left thumbstick's Y axis.
 * The axis values range from -1.0 to 1.0.
 */
/*!
 * \qmlproperty float Gamepad::axisLeftY
 * \readonly
 *
 * The value of the left thumbstick's Y axis.
 * The axis values range from -1.0 to 1.0.
 */
float QGamepad::axisLeftY() const
{
    Q_D(const QGamepad);
    return d->axisLeftY;
}

/*!
 * \property QGamepad::axisRightX
 *
 * This value of the right thumbstick's X axis.
 * The axis values range from -1.0 to 1.0.
 */
/*!
 * \qmlproperty float Gamepad::axisRightX
 * \readonly
 *
 * This value of the right thumbstick's X axis.
 * The axis values range from -1.0 to 1.0.
 */
float QGamepad::axisRightX() const
{
    Q_D(const QGamepad);
    return d->axisRightX;
}

/*!
 * \property QGamepad::axisRightY
 *
 * This value of the right thumbstick's Y axis.
 * The axis values range from -1.0 to 1.0.
 */
/*!
 * \qmlproperty float Gamepad::axisRightY
 * \readonly
 *
 * This value of the right thumbstick's Y axis.
 * The axis values range from -1.0 to 1.0.
 */
float QGamepad::axisRightY() const
{
    Q_D(const QGamepad);
    return d->axisRightY;
}

/*!
 * \property QGamepad::buttonA
 *
 * The state of the A button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonA
 * \readonly
 *
 * The state of the A button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonA() const
{
    Q_D(const QGamepad);
    return d->buttonA;
}

/*!
 * \property QGamepad::buttonB
 *
 * The state of the B button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonB
 * \readonly
 *
 * The state of the B button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonB() const
{
    Q_D(const QGamepad);
    return d->buttonB;
}

/*!
 * \property QGamepad::buttonX
 *
 * The state of the X button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonX
 * \readonly
 *
 * The state of the X button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonX() const
{
    Q_D(const QGamepad);
    return d->buttonX;
}

/*!
 * \property QGamepad::buttonY
 *
 * The state of the Y button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonY
 * \readonly
 *
 * The state of the Y button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonY() const
{
    Q_D(const QGamepad);
    return d->buttonY;
}

/*!
 * \property QGamepad::buttonL1
 *
 * The state of the left shoulder button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonL1
 * \readonly
 *
 * The state of the left shoulder button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonL1() const
{
    Q_D(const QGamepad);
    return d->buttonL1;
}

/*!
 * \property QGamepad::buttonR1
 *
 * The state of the right shoulder button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonR1
 * \readonly
 *
 * The state of the right shoulder button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonR1() const
{
    Q_D(const QGamepad);
    return d->buttonR1;
}

/*!
 * \property QGamepad::buttonL2
 *
 * The value of the left trigger button.
 * This trigger value ranges from 0.0 when not pressed to 1.0
 * when pressed completely.
 */
/*!
 * \qmlproperty float Gamepad::buttonL2
 * \readonly
 *
 * The value of the left trigger button.
 * This trigger value ranges from 0.0 when not pressed to 1.0
 * when pressed completely.
 */
float QGamepad::buttonL2() const
{
    Q_D(const QGamepad);
    return d->buttonL2;
}

/*!
 * \property QGamepad::buttonR2
 *
 * The value of the right trigger button.
 * This trigger value ranges from 0.0 when not pressed to 1.0
 * when pressed completely.
 */
/*!
 * \qmlproperty float Gamepad::buttonR2
 * \readonly
 *
 * The value of the right trigger button.
 * This trigger value ranges from 0.0 when not pressed to 1.0
 * when pressed completely.
 */
float QGamepad::buttonR2() const
{
    Q_D(const QGamepad);
    return d->buttonR2;
}

/*!
 * \property QGamepad::buttonSelect
 *
 * The state of the Select button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button can sometimes be labeled as the Back button on some gamepads.
 */
/*!
 * \qmlproperty bool Gamepad::buttonSelect
 * \readonly
 *
 * The state of the Select button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button can sometimes be labeled as the Back button on some gamepads.
 */
bool QGamepad::buttonSelect() const
{
    Q_D(const QGamepad);
    return d->buttonSelect;
}

/*!
 * \property QGamepad::buttonStart
 *
 * The state of the Start button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button can sometimes be labeled as the Forward button on some gamepads.
 */
/*!
 * \qmlproperty bool Gamepad::buttonStart
 * \readonly
 *
 * The state of the Start button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button can sometimes be labeled as the Forward button on some gamepads.
 */
bool QGamepad::buttonStart() const
{
    Q_D(const QGamepad);
    return d->buttonStart;
}

/*!
 * \property QGamepad::buttonL3
 *
 * The state of the left stick button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button is usually triggered by pressing the left joystick itself.
 */
/*!
 * \qmlproperty bool Gamepad::buttonL3
 * \readonly
 *
 * The state of the left stick button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button is usually triggered by pressing the left joystick itself.
 */
bool QGamepad::buttonL3() const
{
    Q_D(const QGamepad);
    return d->buttonL3;
}

/*!
 * \property QGamepad::buttonR3
 *
 * The state of the right stick button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button is usually triggered by pressing the right joystick itself.
 */
/*!
 * \qmlproperty bool Gamepad::buttonR3
 * \readonly
 *
 * The state of the right stick button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button is usually triggered by pressing the right joystick itself.
 */
bool QGamepad::buttonR3() const
{
    Q_D(const QGamepad);
    return d->buttonR3;
}

/*!
 * \property QGamepad::buttonUp
 *
 * The state of the direction pad up button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonUp
 * \readonly
 *
 * The state of the direction pad up button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonUp() const
{
    Q_D(const QGamepad);
    return d->buttonUp;
}

/*!
 * \property QGamepad::buttonDown
 *
 * The state of the direction pad down button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonDown
 * \readonly
 *
 * The state of the direction pad down button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonDown() const
{
    Q_D(const QGamepad);
    return d->buttonDown;
}

/*!
 * \property QGamepad::buttonLeft
 *
 * The state of the direction pad left button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonLeft
 * \readonly
 *
 * The state of the direction pad left button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonLeft() const
{
    Q_D(const QGamepad);
    return d->buttonLeft;
}

/*!
 * \property QGamepad::buttonRight
 *
 * The state of the direction pad right button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
/*!
 * \qmlproperty bool Gamepad::buttonRight
 * \readonly
 *
 * The state of the direction pad right button.
 * The value is \c true when pressed, and \c false when not pressed.
 */
bool QGamepad::buttonRight() const
{
    Q_D(const QGamepad);
    return d->buttonRight;
}

/*!
 * \property QGamepad::buttonGuide
 *
 * The state of the guide button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button is typically the one in the center of the gamepad with a logo.
 * Not all gamepads have a guide button.
 */
/*
 * \qmlproperty bool Gamepad::buttonGuide
 * \readonly
 *
 * The state of the guide button.
 * The value is \c true when pressed, and \c false when not pressed.
 * This button is typically the one in the center of the gamepad with a logo.
 * Not all gamepads have a guide button.
 */

bool QGamepad::buttonGuide() const
{
    Q_D(const QGamepad);
    return d->buttonGuide;
}

void QGamepad::setDeviceId(int number)
{
    Q_D(QGamepad);
    if (d->deviceId != number) {
        d->deviceId = number;
        emit deviceIdChanged();
        auto input = QUniversalInput::instance();
        d->setName(input->joyName(d->deviceId));
        d->setConnected(input->isJoyConnected(d->deviceId));
    }
}

QT_END_NAMESPACE

#include "moc_qgamepad.cpp"
