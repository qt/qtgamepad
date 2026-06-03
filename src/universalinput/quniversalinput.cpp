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

QString QUniversalInput::getJoyName(int device) const
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

int QUniversalInput::getUnusedJoyId()
{
    Q_D(QUniversalInput);
    for (int i = 0; i < JoypadsMax; i++)
        if (!d->joypadNames.contains(i) || !d->joypadNames[i].isConnected)
            return i;
    return -1;
}

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
        for (int i = 0; i < static_cast<int>(JoyButton::MAX); i++) {
            JoyButton c = combineDevice(static_cast<JoyButton>(i), index);
            d->joystickButtonsPressed.remove(c);
        }
        for (int i = 0; i < static_cast<int>(JoyAxis::MAX); i++)
            setJoyAxis(index, static_cast<JoyAxis>(i), 0.0f);

    }
    d->joypadNames[index] = js;

    Q_EMIT joyConnectionChanged(index, isConnected);
}

void QUniversalInput::joyButton(int device, JoyButton button, bool isPressed)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    Joypad &joy = d->joypadNames[device];
    Q_ASSERT(int(button) < int(JoyButton::MAX));

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

void QUniversalInput::joyAxis(int device, JoyAxis axis, float value)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);

    Q_ASSERT(int(axis) < int(JoyAxis::MAX));

    Joypad &joy = d->joypadNames[device];

    if (joy.lastAxis[size_t(axis)] == value)
        return;

    joy.lastAxis[size_t(axis)] = value;

    if (joy.mapping == -1) {
        sendAxisEvent(device, axis, value);
        return;
    }

    JoyEvent map = mappedAxisEvent(d->mappingDatabase[joy.mapping], axis, value);

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
        if (axis == JoyAxis::TriggerLeft || axis == JoyAxis::TriggerRight)
            value = 0.5f + value / 2.0f; // Convert to a value between 0.0f and 1.0f.
        sendAxisEvent(device, axis, value);
        return;
    }
}

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

QVector2D QUniversalInput::getJoyVibrationStrength(int device)
{
    Q_D(QUniversalInput);
    if (d->joystickVibrations.contains(device))
        return QVector2D(d->joystickVibrations[device].weakMagnitude, d->joystickVibrations[device].strongMagnitude);
    else
        return QVector2D(0.0f, 0.0f);
}

float QUniversalInput::getJoyVibrationDuration(int device)
{
    Q_D(QUniversalInput);
    if (d->joystickVibrations.contains(device))
        return d->joystickVibrations[device].duration;
    else
        return 0.0f;
}

quint64 QUniversalInput::getJoyVibrationTimestamp(int device)
{
    Q_D(QUniversalInput);
    if (d->joystickVibrations.contains(device))
        return d->joystickVibrations[device].timestamp;
    else
        return 0;
}

void QUniversalInput::addForce(int device, QVector2D strength, float duration)
{
    Q_D(QUniversalInput);
    QMutexLocker locker(&d->mutex);
    d->joystickVibrations[device].weakMagnitude = strength.x();
    d->joystickVibrations[device].strongMagnitude = strength.y();
    d->joystickVibrations[device].duration = duration; // sec
    d->joystickVibrations[device].timestamp = QDateTime::currentMSecsSinceEpoch();
}

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

QUniversalInput::JoyEvent QUniversalInput::mappedAxisEvent(const JoyDeviceMapping &mapping, JoyAxis axis, float inValue)
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
            switch (binding.input.hat.hat_mask) {
            case HatMask::Up:
                hat_direction = HatDirection::Up;
                break;
            case HatMask::Right:
                hat_direction = HatDirection::Right;
                break;
            case HatMask::Down:
                hat_direction = HatDirection::Down;
                break;
            case HatMask::Left:
                hat_direction = HatDirection::Left;
                break;
            default:
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

QDebug operator<<(QDebug debug, const HatMask &hatMask)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "HatMask::";

    switch (hatMask) {
    case HatMask::Center: debug.nospace() << "Center"; break;
    case HatMask::Up: debug.nospace() << "Up"; break;
    case HatMask::Right: debug.nospace() << "Right"; break;
    case HatMask::Down: debug.nospace() << "Down"; break;
    case HatMask::Left: debug.nospace() << "Left"; break;
    default:
        debug.nospace() << "(" << static_cast<int>(hatMask) << ")";
        break;
    }
    return debug;
}

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
