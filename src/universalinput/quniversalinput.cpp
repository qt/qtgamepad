// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "core/input/input.cpp" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#include "quniversalinput.h"
#include "quniversalinput_p.h"
#include "qjoystickinput_p.h"
#include "qjoystickinputfactory_p.h"
#include "qjoydevicemappingparser_p.h"
#include "qmouseinput_p.h"
#include "qmouseinputfactory_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcUniversalInput, "qt.universalinput")

// The input enums live in QUniversalInput; alias them for the free
// helper functions and debug operators in this file.
using HatDirection = QUniversalInput::HatDirection;
using HatFlag = QUniversalInput::HatFlag;
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

/*!
    \class QUniversalInput
    \inmodule QtUniversalInput
    \since 6.12
    \brief The QUniversalInput class provides access to connected joysticks and
    gamepads.

    QUniversalInput is a process-wide singleton, accessed through instance(),
    that reports the live state of the joysticks and gamepads connected to the
    system. Devices are identified by an integer device index. Where a device
    is recognized by the bundled game controller database it is exposed as a
    gamepad with a common button and axis layout; isGamepad() reports whether a
    given device has such a mapping.

    Input is delivered through the joyButtonEvent(), joyAxisEvent() and
    joyConnectionChanged() signals. For higher level, named actions use
    \l QActionStore, and for a simple single-gamepad convenience API use
    \l QGamepad.

    \sa QActionStore, QGamepad
*/

/*!
    \enum QUniversalInput::JoyButton

    This enum represents the buttons on a gamepad using a layout modelled on a
    common controller.

    \value Invalid An invalid or unknown button.
    \value A The bottom face button.
    \value B The right face button.
    \value X The left face button.
    \value Y The top face button.
    \value Back The back, select or share button.
    \value Guide The guide or system button.
    \value Start The start button.
    \value LeftStick The left stick when pressed in.
    \value RightStick The right stick when pressed in.
    \value LeftShoulder The left shoulder button.
    \value RightShoulder The right shoulder button.
    \value DpadUp The up direction of the directional pad.
    \value DpadDown The down direction of the directional pad.
    \value DpadLeft The left direction of the directional pad.
    \value DpadRight The right direction of the directional pad.
    \value Misc1 A miscellaneous button.
    \value Paddle1 The first paddle button.
    \value Paddle2 The second paddle button.
    \value Paddle3 The third paddle button.
    \value Paddle4 The fourth paddle button.
    \value Touchpad The touchpad when pressed.
*/

/*!
    \enum QUniversalInput::JoyAxis

    This enum represents the analog axes of a gamepad.

    \value Invalid An invalid or unknown axis.
    \value LeftX The horizontal axis of the left stick.
    \value LeftY The vertical axis of the left stick.
    \value RightX The horizontal axis of the right stick.
    \value RightY The vertical axis of the right stick.
    \value TriggerLeft The left analog trigger.
    \value TriggerRight The right analog trigger.
*/

/*!
    \enum QUniversalInput::HatDirection

    This enum represents the individual directions of a hat (directional pad).

    \value Up The up direction.
    \value Right The right direction.
    \value Down The down direction.
    \value Left The left direction.
    \value Max The number of directions.
*/

/*!
    \enum QUniversalInput::HatFlag

    This enum holds the individual flags that make up a \c HatMask. Because a
    hat can be pressed diagonally, the flags can be combined, for example
    \c{HatFlag::Up | HatFlag::Right}.

    \value Center No direction is pressed.
    \value Up The up direction is pressed.
    \value Right The right direction is pressed.
    \value Down The down direction is pressed.
    \value Left The left direction is pressed.
*/

/*!
    \typedef QUniversalInput::HatMask

    A \l QFlags combination of \l HatFlag values describing the current state of
    a hat (directional pad).
*/

/*!
    \enum QUniversalInput::JoyType

    This enum describes the kind of input a mapping entry refers to.

    \value TypeButton A button.
    \value TypeAxis An axis.
    \value TypeHat A hat (directional pad).
    \value TypeMax The number of input types.
*/

/*!
    \enum QUniversalInput::JoyAxisRange

    This enum describes which part of an axis a mapping entry uses.

    \value NegativeHalfAxis Only the negative half of the axis.
    \value FullAxis The full range of the axis.
    \value PositiveHalfAxis Only the positive half of the axis.
*/

/*!
    \fn QUniversalInput *QUniversalInput::instance()

    Returns the process-wide QUniversalInput singleton, creating it on first
    use.
*/

/*!
    \fn QString QUniversalInput::joyName(int device) const

    Returns the human-readable name of the joystick at index \a device, or an
    empty string if no such device is connected.
*/

/*!
    \fn bool QUniversalInput::isJoyConnected(int device) const

    Returns \c true if a joystick is connected at index \a device.
*/

/*!
    \fn bool QUniversalInput::isGamepad(int device) const

    Returns \c true if the device at index \a device is recognized as a gamepad,
    that is, it has a mapping in the game controller database.
*/

/*!
    \fn void QUniversalInput::setMouseDisabled(bool disabled)

    Sets whether relative mouse mode is enabled to \a disabled. When enabled,
    pointer motion is reported through mouseMovedWithDeltas() rather than as
    absolute cursor positions.

    \sa isMouseDisabled(), mouseMovedWithDeltas()
*/

/*!
    \fn bool QUniversalInput::isMouseDisabled() const

    Returns \c true if relative mouse mode is enabled.

    \sa setMouseDisabled()
*/

/*!
    \fn void QUniversalInput::joyConnectionChanged(int index, bool isConnected)

    This signal is emitted when the joystick at \a index is connected or
    disconnected, as given by \a isConnected.
*/

/*!
    \fn void QUniversalInput::joyButtonEvent(int device, QUniversalInput::JoyButton button, bool isPressed)

    This signal is emitted when \a button on the device at index \a device
    changes state, as given by \a isPressed.
*/

/*!
    \fn void QUniversalInput::joyAxisEvent(int device, QUniversalInput::JoyAxis axis, float value)

    This signal is emitted when \a axis on the device at index \a device changes
    to \a value, in the range -1.0 to 1.0.
*/

/*!
    \fn void QUniversalInput::mouseDisabledChanged()

    This signal is emitted when the relative mouse mode changes.

    \sa setMouseDisabled()
*/

/*!
    \fn void QUniversalInput::mouseMovedWithDeltas(const QVector2D &deltas)

    This signal is emitted in relative mouse mode when the pointer moves by
    \a deltas.

    \sa setMouseDisabled()
*/

static JoyAxis combineDevice(JoyAxis value, int device)
{
    return JoyAxis(static_cast<int>(value) | (device << 20));
}

static JoyButton combineDevice(JoyButton value, int device)
{
    return JoyButton(static_cast<int>(value) | (device << 20));
}

static QByteArray hexStr(quint8 byte)
{
    static const char *dict = "0123456789abcdef";
    char ret[3];
    ret[2] = 0;

    ret[0] = dict[byte >> 4];
    ret[1] = dict[byte & 0xf];

    return QByteArray(ret);
}

QUniversalInputPrivate::QUniversalInputPrivate()
{

}

QUniversalInputPrivate::~QUniversalInputPrivate()
{
    delete joystickInput;
}

void QUniversalInputPrivate::_q_init()
{
    loadMappingDatabase();

    QStringList keys = QJoystickInputFactory::keys();
    if (!keys.isEmpty())
        joystickInput = QJoystickInputFactory::create(keys.first(), QStringList());

    // If we fail to load a plugin, create a dummy joystick input
    if (!joystickInput)
        joystickInput = new QJoystickInput();

    keys = QMouseInputFactory::keys();
    if (!keys.isEmpty())
        mouseInput = QMouseInputFactory::create(keys.first(), QStringList());

    // If we fail to load a plugin, create a dummy mouse input
    if (!mouseInput)
        mouseInput = new QMouseInput();
}

void QUniversalInputPrivate::loadMappingDatabase()
{
    QJoyDeviceMappingParser parser(QString::fromUtf8(":/qt-project.org/qtuniversalinput/gamecontrollerdb.txt"));
    auto mapping = parser.next();
    for (; mapping.has_value(); mapping = parser.next())
        mappingDatabase.push_back(mapping.value());
}

void QUniversalInput::VelocityTrack::update(const QVector2D &valueDelta)
{
    float delta_t = frameTimer.restart() / 1000.0f;

    if (delta_t > maxRefFrame) {
        // First movement in a long time, reset and start again.
        velocity = QVector2D();
        accum = valueDelta;
        accumTime = 0.0f;
        return;
    }

    accum += valueDelta;
    accumTime += delta_t;

    if (accumTime < minRefFrame) {
        // Not enough time has passed to calculate speed precisely.
        return;
    }

    velocity = accum / accumTime;
    accum = QVector2D();
    accumTime = 0.0f;
}

void QUniversalInput::VelocityTrack::reset()
{
    frameTimer.restart();
    velocity = QVector2D();
    accum = QVector2D();
    accumTime = 0.0f;
}

QUniversalInput::VelocityTrack::VelocityTrack()
{
    minRefFrame = 0.1f;
    maxRefFrame = 3.0f;
    frameTimer.start();
    reset();
}

void QUniversalInput::loadPlugins()
{
    Q_D(QUniversalInput);
    d->_q_init();
}

QUniversalInput::QUniversalInput()
    : QObject(*new QUniversalInputPrivate(), nullptr)
{
    // We need to delay the plugin loading until the event loop is running
    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);
}

QUniversalInput::~QUniversalInput()
{
}

QUniversalInput *QUniversalInput::instance()
{
    static QUniversalInput instance;
    return &instance;
}

QString QUniversalInput::joyName(int device) const
{
    Q_D(const QUniversalInput);
    // If the device does not exist, an empty Joypad
    // Struct should be returned, so name will be empty
    const auto joypad = d->joypadNames[device];
    return joypad.name;
}

bool QUniversalInput::isJoyConnected(int device) const
{
    Q_D(const QUniversalInput);
    // If the device does not exist, an empty Joypad
    // Struct should be returned
    const auto joypad = d->joypadNames[device];
    return joypad.isConnected;
}

bool QUniversalInput::isGamepad(int device) const
{
    Q_D(const QUniversalInput);
    // If the device does not exist, an empty Joypad
    // Struct should be returned
    const auto joypad = d->joypadNames[device];
    // If a device has a mapping, it is a gamepad
    return joypad.mapping != -1;
}

/*! \internal */
int QUniversalInput::unusedJoyId()
{
    Q_D(QUniversalInput);
    for (int i = 0; i < JoypadsMax; i++)
        if (!d->joypadNames.contains(i) || !d->joypadNames[i].isConnected)
            return i;
    return -1;
}

/*! \internal */
void QUniversalInput::updateJoyConnection(int index, bool isConnected, const QString &name, const QString &guid)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    Joypad js;
    js.name = isConnected ? name : QString();
    js.uid = isConnected ? guid : QString();

    if (isConnected) {
        QByteArray uidname = guid.toLocal8Bit();
        if (guid.isEmpty()) {
            int uidlen = int(qMin(name.length(), 16LL));
            QByteArray localName = name.toLocal8Bit();
            for (int i = 0; i < uidlen; i++)
                uidname = uidname + hexStr(localName[i]);
        }
        js.uid = QString::fromLocal8Bit(uidname);
        js.isConnected = true;
        int mapping = d->fallbackMapping;
        for (int i = 0; i < d->mappingDatabase.size(); i++) {
            if (js.uid == d->mappingDatabase[i].uid) {
                mapping = i;
                js.name = d->mappingDatabase[i].name;
            }
        }
        js.mapping = mapping;
    } else {
        js.isConnected = false;
        for (int i = 0; i < JoyButtonsMax; i++) {
            JoyButton c = combineDevice(static_cast<JoyButton>(i), index);
            d->joystickButtonsPressed.remove(c);
        }
        for (int i = 0; i < JoyAxesMax; i++)
            setJoyAxis(index, static_cast<JoyAxis>(i), 0.0f);

    }
    d->joypadNames[index] = js;

    Q_EMIT joyConnectionChanged(index, isConnected);
}

/*! \internal */
void QUniversalInput::joyButton(int device, JoyButton button, bool isPressed)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    if (int(button) < 0 || int(button) >= JoyButtonsMax) {
        qCWarning(lcUniversalInput) << "Ignoring out-of-range joypad button" << int(button);
        return;
    }

    Joypad &joy = d->joypadNames[device];

    if (joy.lastButtons[size_t(button)] == isPressed)
        return;

    joy.lastButtons[size_t(button)] = isPressed;
    if (joy.mapping == -1) {
        sendButtonEvent(device, button, isPressed);
        return;
    }

    JoyEvent map = mappedButtonEvent(d->mappingDatabase[joy.mapping], button);

    if (map.type == TypeButton) {
        sendButtonEvent(device, JoyButton(map.index), isPressed);
        return;
    }

    if (map.type == TypeAxis)
        sendAxisEvent(device, JoyAxis(map.index), isPressed ? map.value : 0.0f);
}

/*! \internal */
void QUniversalInput::joyAxis(int device, JoyAxis axis, float value)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    if (int(axis) < 0 || int(axis) >= JoyAxesMax) {
        qCWarning(lcUniversalInput) << "Ignoring out-of-range joypad axis" << int(axis);
        return;
    }

    Joypad &joy = d->joypadNames[device];

    if (joy.lastAxis[size_t(axis)] == value)
        return;

    joy.lastAxis[size_t(axis)] = value;

    if (joy.mapping == -1) {
        sendAxisEvent(device, axis, value);
        return;
    }

    JoyAxisRange range = FullAxis;
    JoyEvent map = mappedAxisEvent(d->mappingDatabase[joy.mapping], axis, value, &range);

    if (map.type == TypeButton) {
        bool pressed = map.value > 0.5;
        if (pressed != d->joystickButtonsPressed.contains(combineDevice(JoyButton(map.index), device)))
            sendButtonEvent(device, JoyButton(map.index), pressed);

        // Ensure opposite D-Pad button is also released.
        switch (JoyButton(map.index)) {
        case JoyButton::DpadUp:
            if (d->joystickButtonsPressed.contains(combineDevice(JoyButton::DpadDown, device)))
                sendButtonEvent(device, JoyButton::DpadDown, false);
            break;
        case JoyButton::DpadDown:
            if (d->joystickButtonsPressed.contains(combineDevice(JoyButton::DpadUp, device)))
                sendButtonEvent(device, JoyButton::DpadUp, false);
            break;
        case JoyButton::DpadLeft:
            if (d->joystickButtonsPressed.contains(combineDevice(JoyButton::DpadRight, device)))
                sendButtonEvent(device, JoyButton::DpadRight, false);
            break;
        case JoyButton::DpadRight:
            if (d->joystickButtonsPressed.contains(combineDevice(JoyButton::DpadLeft, device)))
                sendButtonEvent(device, JoyButton::DpadLeft, false);
            break;
        default:
            // Nothing to do.
            break;
        }
        return;
    }

    if (map.type == TypeAxis) {
        JoyAxis axis = JoyAxis(map.index);
        float value = map.value;
#ifndef Q_OS_ANDROID
        // Only a full-axis trigger reports [-1, 1] and needs converting; half
        // axes, and Android triggers, already report [0, 1].
        if (range == FullAxis && (axis == JoyAxis::TriggerLeft || axis == JoyAxis::TriggerRight))
            value = 0.5f + value / 2.0f;
#endif
        sendAxisEvent(device, axis, value);
        return;
    }
}

/*! \internal */
void QUniversalInput::joyHat(int device, HatMask value)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    const Joypad &joy = d->joypadNames[device];

    JoyEvent map[size_t(HatDirection::Max)];
    map[size_t(HatDirection::Up)].type = TypeButton;
    map[size_t(HatDirection::Up)].index = int(JoyButton::DpadUp);
    map[size_t(HatDirection::Up)].value = 0;

    map[size_t(HatDirection::Right)].type = TypeButton;
    map[size_t(HatDirection::Right)].index = int(JoyButton::DpadRight);
    map[size_t(HatDirection::Right)].value = 0;

    map[size_t(HatDirection::Down)].type = TypeButton;
    map[size_t(HatDirection::Down)].index = int(JoyButton::DpadDown);
    map[size_t(HatDirection::Down)].value = 0;

    map[size_t(HatDirection::Left)].type = TypeButton;
    map[size_t(HatDirection::Left)].index = int(JoyButton::DpadLeft);
    map[size_t(HatDirection::Left)].value = 0;

    // This is a bit weird... as it overwrites index [0] DirectionUp
    if (joy.mapping != -1)
        mappedHatEvents(d->mappingDatabase[joy.mapping], HatDirection(0), map);

    int cur_val = d->joypadNames[device].hatCurrent;

    for (int hat_direction = 0, hat_mask = 1; hat_direction < static_cast<int>(HatDirection::Max); hat_direction++, hat_mask <<= 1) {
        if ((int(value) & hat_mask) != (cur_val & hat_mask)) {
            if (map[hat_direction].type == TypeButton)
                sendButtonEvent(device, JoyButton(map[hat_direction].index), int(value) & hat_mask);
            if (map[hat_direction].type == TypeAxis)
                sendAxisEvent(device, JoyAxis(map[hat_direction].index), (int(value) & hat_mask) ? map[hat_direction].value : 0.0f);
        }
    }

    d->joypadNames[device].hatCurrent = int(value);
}

/*! \internal */
QVector2D QUniversalInput::joyVibrationStrength(int device)
{
    Q_D(QUniversalInput);
    if (d->joystickVibrations.contains(device))
        return QVector2D(d->joystickVibrations[device].weakMagnitude, d->joystickVibrations[device].strongMagnitude);
    else
        return QVector2D(0.0f, 0.0f);
}

/*! \internal */
float QUniversalInput::joyVibrationDuration(int device)
{
    Q_D(QUniversalInput);
    if (d->joystickVibrations.contains(device))
        return d->joystickVibrations[device].duration;
    else
        return 0.0f;
}

/*! \internal */
quint64 QUniversalInput::joyVibrationTimestamp(int device)
{
    Q_D(QUniversalInput);
    if (d->joystickVibrations.contains(device))
        return d->joystickVibrations[device].timestamp;
    else
        return 0;
}

/*! \internal */
void QUniversalInput::addForce(int device, QVector2D strength, float duration)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);
    d->joystickVibrations[device].weakMagnitude = strength.x();
    d->joystickVibrations[device].strongMagnitude = strength.y();
    d->joystickVibrations[device].duration = duration; // sec
    d->joystickVibrations[device].timestamp = QDateTime::currentMSecsSinceEpoch();
}

/*! \internal */
void QUniversalInput::setJoyAxis(int device, JoyAxis axis, float value)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    JoyAxis c = combineDevice(axis, device);
    d->joystickAxes[c] = value;
}

void QUniversalInput::sendButtonEvent(int device, JoyButton index, bool pressed)
{
    Q_UNUSED(device);
    Q_UNUSED(index);
    Q_UNUSED(pressed);
    Q_EMIT joyButtonEvent(device, index, pressed);
}

void QUniversalInput::sendAxisEvent(int device, JoyAxis axis, float value)
{
    Q_UNUSED(device);
    Q_UNUSED(axis);
    Q_UNUSED(value);
    Q_EMIT joyAxisEvent(device, axis, value);
}

// mouse disable
void QUniversalInput::setMouseDisabled(bool disabled)
{
    Q_D(QUniversalInput);
    if (d->mouseDisabled == disabled)
        return;

    d->mouseDisabled = disabled;
    Q_EMIT mouseDisabledChanged();
}

bool QUniversalInput::isMouseDisabled() const
{
    Q_D(const QUniversalInput);
    return d->mouseDisabled;
}

/*! \internal */
void QUniversalInput::mouseMove(const QVector2D &deltas)
{
    Q_EMIT mouseMovedWithDeltas(deltas);
}

// mouse disable

QUniversalInput::JoyEvent QUniversalInput::mappedButtonEvent(const JoyDeviceMapping &mapping, JoyButton button)
{
    JoyEvent event;

    for (int i = 0; i < mapping.bindings.size(); i++) {
        const JoyBinding binding = mapping.bindings[i];
        if (binding.inputType == TypeButton && binding.input.button == button) {
            event.type = binding.outputType;
            switch (binding.outputType) {
            case TypeButton:
                event.index = static_cast<int>(binding.output.button);
                return event;
            case TypeAxis:
                event.index = static_cast<int>(binding.output.axis.axis);
                switch (binding.output.axis.range) {
                case PositiveHalfAxis:
                    event.value = 1;
                    break;
                case NegativeHalfAxis:
                    event.value = -1;
                    break;
                case FullAxis:
                    // It doesn't make sense for a button to map to a full axis,
                    // but keeping as a default for a trigger with a positive half-axis.
                    event.value = 1;
                    break;
                }
                return event;
            default:
                qCWarning(lcUniversalInput, "Joypad button mapping error.");
            }
        }
    }
    return event;
}

QUniversalInput::JoyEvent QUniversalInput::mappedAxisEvent(const JoyDeviceMapping &mapping, JoyAxis axis, float inValue, JoyAxisRange *outRange)
{
    JoyEvent event;

    for (int i = 0; i < mapping.bindings.size(); i++) {
        const JoyBinding binding = mapping.bindings[i];
        if (binding.inputType == TypeAxis && binding.input.axis.axis == axis) {
            float value = inValue;
            if (binding.input.axis.invert)
                value = -value;
            if (binding.input.axis.range == FullAxis ||
                    (binding.input.axis.range == PositiveHalfAxis && value >= 0) ||
                    (binding.input.axis.range == NegativeHalfAxis && value < 0)) {
                event.type = binding.outputType;
                if (outRange)
                    *outRange = binding.input.axis.range;
                float shifted_positive_value = 0;
                switch (binding.input.axis.range) {
                case PositiveHalfAxis:
                    shifted_positive_value = value;
                    break;
                case NegativeHalfAxis:
                    shifted_positive_value = value + 1;
                    break;
                case FullAxis:
                    shifted_positive_value = (value + 1) / 2;
                    break;
                }
                switch (binding.outputType) {
                case TypeButton:
                    event.index = static_cast<int>(binding.output.button);
                    switch (binding.input.axis.range) {
                    case PositiveHalfAxis:
                        event.value = shifted_positive_value;
                        break;
                    case NegativeHalfAxis:
                        event.value = 1 - shifted_positive_value;
                        break;
                    case FullAxis:
                        // It doesn't make sense for a full axis to map to a button,
                        // but keeping as a default for a trigger with a positive half-axis.
                        event.value = (shifted_positive_value * 2) - 1;
                        break;
                    }
                    return event;
                case TypeAxis:
                    event.index = static_cast<int>(binding.output.axis.axis);
                    event.value = value;
                    if (binding.output.axis.range != binding.input.axis.range) {
                        switch (binding.output.axis.range) {
                        case PositiveHalfAxis:
                            event.value = shifted_positive_value;
                            break;
                        case NegativeHalfAxis:
                            event.value = shifted_positive_value - 1;
                            break;
                        case FullAxis:
                            event.value = (shifted_positive_value * 2) - 1;
                            break;
                        }
                    }
                    return event;
                default:
                    qCWarning(lcUniversalInput, "Joypad axis mapping error.");
                }
            }
        }
    }
    return event;
}

void QUniversalInput::mappedHatEvents(const JoyDeviceMapping &mapping, HatDirection hat, JoyEvent events[size_t(HatDirection::Max)])
{
    for (int i = 0; i < mapping.bindings.size(); i++) {
        const JoyBinding binding = mapping.bindings[i];
        if (binding.inputType == TypeHat && binding.input.hat.hat == hat) {
            HatDirection hat_direction;
            const HatMask hatMask = binding.input.hat.hat_mask;
            if (hatMask == HatFlag::Up)
                hat_direction = HatDirection::Up;
            else if (hatMask == HatFlag::Right)
                hat_direction = HatDirection::Right;
            else if (hatMask == HatFlag::Down)
                hat_direction = HatDirection::Down;
            else if (hatMask == HatFlag::Left)
                hat_direction = HatDirection::Left;
            else {
                qCWarning(lcUniversalInput, "Joypad button mapping error.");
                continue;
            }

            events[size_t(hat_direction)].type = binding.outputType;
            switch (binding.outputType) {
            case TypeButton:
                events[size_t(hat_direction)].index = int(binding.output.button);
                break;
            case TypeAxis:
                events[size_t(hat_direction)].index = int(binding.output.axis.axis);
                switch (binding.output.axis.range) {
                case PositiveHalfAxis:
                    events[size_t(hat_direction)].value = 1;
                    break;
                case NegativeHalfAxis:
                    events[size_t(hat_direction)].value = -1;
                    break;
                case FullAxis:
                    // It doesn't make sense for a hat direction to map to a full axis,
                    // but keeping as a default for a trigger with a positive half-axis.
                    events[size_t(hat_direction)].value = 1;
                    break;
                }
                break;
            default:
                qCWarning(lcUniversalInput, "Joypad button mapping error.");
            }
        }
    }
}

/*! \internal */
QDebug operator<<(QDebug debug, const JoyButton &joyButton)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "JoyButton::";

    switch (joyButton) {
    case JoyButton::Invalid: debug.nospace() << "Invalid"; break;
    case JoyButton::A: debug.nospace() << "A"; break;
    case JoyButton::B: debug.nospace() << "B"; break;
    case JoyButton::X: debug.nospace() << "X"; break;
    case JoyButton::Y: debug.nospace() << "Y"; break;
    case JoyButton::Back: debug.nospace() << "Back"; break;
    case JoyButton::Guide: debug.nospace() << "Guide"; break;
    case JoyButton::Start: debug.nospace() << "Start"; break;
    case JoyButton::LeftStick: debug.nospace() << "LeftStick"; break;
    case JoyButton::RightStick: debug.nospace() << "RightStick"; break;
    case JoyButton::LeftShoulder: debug.nospace() << "LeftShoulder"; break;
    case JoyButton::RightShoulder: debug.nospace() << "RightShoulder"; break;
    case JoyButton::DpadUp: debug.nospace() << "DpadUp"; break;
    case JoyButton::DpadDown: debug.nospace() << "DpadDown"; break;
    case JoyButton::DpadLeft: debug.nospace() << "DpadLeft"; break;
    case JoyButton::DpadRight: debug.nospace() << "DpadRight"; break;
    case JoyButton::Misc1: debug.nospace() << "Misc1"; break;
    case JoyButton::Paddle1: debug.nospace() << "Paddle1"; break;
    case JoyButton::Paddle2: debug.nospace() << "Paddle2"; break;
    case JoyButton::Paddle3: debug.nospace() << "Paddle3"; break;
    case JoyButton::Paddle4: debug.nospace() << "Paddle4"; break;
    case JoyButton::Touchpad: debug.nospace() << "Touchpad"; break;
    default:
        debug.nospace() << "Button(" << static_cast<int>(joyButton) << ")";
        break;
    }
    return debug;
}

/*! \internal */
QDebug operator<<(QDebug debug, const JoyAxis &axis)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "JoyAxis::";

    switch (axis) {
    case JoyAxis::Invalid: debug.nospace() << "Invalid"; break;
    case JoyAxis::LeftX: debug.nospace() << "LeftX"; break;
    case JoyAxis::LeftY: debug.nospace() << "LeftY"; break;
    case JoyAxis::RightX: debug.nospace() << "RightX"; break;
    case JoyAxis::RightY: debug.nospace() << "RightY"; break;
    case JoyAxis::TriggerLeft: debug.nospace() << "TriggerLeft"; break;
    case JoyAxis::TriggerRight: debug.nospace() << "TriggerRight"; break;
    default:
        debug.nospace() << "(" << static_cast<int>(axis) << ")";
        break;
    }
    return debug;
}

/*! \internal */
QDebug operator<<(QDebug debug, const QUniversalInput::JoyAxisRange &range)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "JoyAxisRange::";
    switch (range) {
    case QUniversalInput::NegativeHalfAxis: debug.nospace() << "NegativeHalfAxis"; break;
    case QUniversalInput::FullAxis: debug.nospace() << "FullAxis"; break;
    case QUniversalInput::PositiveHalfAxis: debug.nospace() << "PositiveHalfAxis"; break;
    default:
        debug.nospace() << "(" << static_cast<int>(range) << ")";
        break;
    }

    return debug;
}

/*! \internal */
QDebug operator<<(QDebug debug, const HatDirection &hatDirection)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "HatDirection::";
    switch (hatDirection) {
    case HatDirection::Up: debug.nospace() << "Up"; break;
    case HatDirection::Right: debug.nospace() << "Right"; break;
    case HatDirection::Down: debug.nospace() << "Down"; break;
    case HatDirection::Left: debug.nospace() << "Left"; break;
    default:
        debug.nospace() << "(" << static_cast<int>(hatDirection) << ")";
        break;
    }
    return debug;
}

/*! \internal */
QDebug operator<<(QDebug debug, const QUniversalInput::JoyBinding &binding)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "JoyBinding( inputType: ";
    switch (binding.inputType) {

    case QUniversalInput::TypeButton:
        debug.nospace() << "TypeButton, input: " << binding.input.button;
        break;
    case QUniversalInput::TypeAxis:
        debug.nospace() << "TypeAxis, input: " << binding.input.axis.axis << ", range: " << binding.input.axis.range;
        break;
    case QUniversalInput::TypeHat:
        debug.nospace() << "TypeHat, input: " << binding.input.hat.hat << ", mask: " << binding.input.hat.hat_mask;
        break;
    case QUniversalInput::TypeMax:
        break;
    }

    debug.nospace() << ", outputType: ";
    switch (binding.outputType) {
    case QUniversalInput::TypeButton:
        debug.nospace() << "TypeButton, output: " << binding.output.button;
        break;
    case QUniversalInput::TypeAxis:
        debug.nospace() << "TypeAxis, output: " << binding.output.axis.axis << ", range: " << binding.output.axis.range;
        break;
    case QUniversalInput::TypeHat:
    case QUniversalInput::TypeMax:
        break;
    }

    debug.nospace() << ")";

    return debug;
}

/*! \internal */
QDebug operator<<(QDebug debug, const QUniversalInput::JoyDeviceMapping &mapping)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "JoyDeviceMapping(" << mapping.uid << ", " << mapping.name << ", bindings: [";
    for (const QUniversalInput::JoyBinding &binding : mapping.bindings) {
        debug.nospace() << binding << ", ";
    }
    debug.nospace() << "])";
    return debug;
}

QT_END_NAMESPACE
