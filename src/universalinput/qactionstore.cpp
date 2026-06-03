// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qactionstore.h"

#include <private/qobject_p.h>

#include <QKeyEvent>

QT_BEGIN_NAMESPACE

// Bring the QUniversalInput input enums into scope for this file.
using HatDirection = QUniversalInput::HatDirection;
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

class QActionStorePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QActionStore)

public:
    QActionStorePrivate()
    {
    }

    QHash<QString, QActionStore::Action> actions;

    void _q_handleJoyAxisEvent(int device, JoyAxis axis, float value);
    void _q_handleJoyButtonEvent(int device, JoyButton button, bool isPressed);
};

/*!
    \class QActionStore
    \inmodule QtUniversalInput
    \since 6.12
    \brief The QActionStore class maps raw input onto named, game-level actions.

    QActionStore lets an application describe input in terms of abstract
    actions, such as "Jump" or "MoveLeft", instead of specific buttons, axes,
    keys or mouse buttons. Actions are described with an \l ActionBuilder and
    registered with registerAction(). When matching input arrives, the store
    emits actionEvent() with the action's name, along with a more specific
    signal such as actionJoyButtonEvent() for the originating input.

    \sa QUniversalInput, ActionBuilder
*/

/*!
    \enum QActionStore::Controller

    Identifies which device an action listens to.

    \value All Matches any device.
    \value Device0 The device at index 0.
    \value Device1 The device at index 1.
    \value Device2 The device at index 2.
    \value Device3 The device at index 3.
    \value Device4 The device at index 4.
    \value Device5 The device at index 5.
    \value Device6 The device at index 6.
    \value Device7 The device at index 7.
    \value Device8 The device at index 8.
    \value Device9 The device at index 9.
    \value Device10 The device at index 10.
    \value Device11 The device at index 11.
    \value Device12 The device at index 12.
    \value Device13 The device at index 13.
    \value Device14 The device at index 14.
    \value Device15 The device at index 15.
    \value Device16 The device at index 16.
    \value DeviceMAX The number of supported devices.
*/

/*!
    \enum QActionStore::AxisDirection

    Selects the direction of an axis that an action responds to.

    \value All Matches any direction.
    \value Up The negative vertical direction.
    \value Right The positive horizontal direction.
    \value Down The positive vertical direction.
    \value Left The negative horizontal direction.
    \value Max The number of directions.
*/

/*!
    \class QActionStore::ActionBuilder
    \inmodule QtUniversalInput
    \since 6.12
    \brief A builder for assembling a QActionStore action from input triggers.

    Construct an ActionBuilder with the action name, add one or more triggers
    with addButton(), addAxis(), addKey() or addMouseButton(), and pass the
    result of build() to QActionStore::registerAction().
*/

/*!
    \fn QActionStore::ActionBuilder::ActionBuilder(const QString &name)

    Constructs an action builder for the action called \a name.
*/

/*!
    Constructs a QActionStore with the given \a parent.
*/
QActionStore::QActionStore(QObject *parent)
    : QObject(*new QActionStorePrivate, parent)
{
    Q_D(QActionStore);
    d->q_ptr = this;

    auto input = QUniversalInput::instance();
    QObjectPrivate::connect(input, &QUniversalInput::joyAxisEvent,
                            d, &QActionStorePrivate::_q_handleJoyAxisEvent);
    QObjectPrivate::connect(input, &QUniversalInput::joyButtonEvent,
                            d, &QActionStorePrivate::_q_handleJoyButtonEvent);

    if (parent)
        parent->installEventFilter(this);
}

QActionStore::~QActionStore()
{
    if (parent())
        parent()->removeEventFilter(this);
}

/*!
    Registers \a action with the store. Once registered, matching input emits
    the corresponding action signals. Build an action with \l ActionBuilder.

    \sa clearActions()
*/
void QActionStore::registerAction(const Action &action)
{
    Q_D(QActionStore);
    d->actions.insert(action.name, action);
}

/*!
    Removes all registered actions.

    \sa registerAction()
*/
void QActionStore::clearActions()
{
    Q_D(QActionStore);
    d->actions.clear();
}

void QActionStorePrivate::_q_handleJoyAxisEvent(int device, JoyAxis axis, float value)
{
    for (int j = 0; j < actions.size(); j++) {
        auto key = actions.keys()[j];
        auto action = actions[key];
        for (auto axisAction : action.axes) {
            const auto absValue = qAbs(value);
            if (axisAction.axis == axis && (static_cast<int>(axisAction.device) == device || axisAction.device == QActionStore::Controller::All) && absValue >= axisAction.deadzone) {
                switch (axisAction.direction) {
                case QActionStore::AxisDirection::Left:
                case QActionStore::AxisDirection::Up:
                    if (value < 0) {
                        Q_EMIT q_func()->actionEvent(action.name);
                        Q_EMIT q_func()->actionJoyAxisEvent(action.name, device, axis, absValue);
                    }
                    break;
                case QActionStore::AxisDirection::Right:
                case QActionStore::AxisDirection::Down:
                    if (value > 0) {
                        Q_EMIT q_func()->actionEvent(action.name);
                        Q_EMIT q_func()->actionJoyAxisEvent(action.name, device, axis, absValue);
                    }
                    break;
                case QActionStore::AxisDirection::All:
                    Q_EMIT q_func()->actionEvent(action.name);
                    Q_EMIT q_func()->actionJoyAxisEvent(action.name, device, axis, absValue);
                    break;
                default:
                    break;
                };
            }

            if (actions.size() == 0)
                break;
        }
        if (actions.size() == 0)
            break;
    }
}

void QActionStorePrivate::_q_handleJoyButtonEvent(int device, JoyButton button, bool isPressed)
{
    for (int j = 0; j < actions.size(); j++) {
        auto key = actions.keys()[j];
        auto action = actions[key];

        for (auto buttonAction : action.buttons) {
            if (buttonAction.button == button && buttonAction.isPressed == isPressed && (static_cast<int>(buttonAction.device) == device || buttonAction.device == QActionStore::Controller::All)) {
                Q_EMIT q_func()->actionEvent(action.name);
                Q_EMIT q_func()->actionJoyButtonEvent(action.name, device, button, isPressed);
            }
            if (actions.size() == 0)
                break;
        }
        if (actions.size() == 0)
            break;
    }
}


/*!
    Delivers a keyboard event for \a key with the given \a isPressed state to
    the store, emitting actionEvent() and actionKeyEvent() for any matching
    action. This is normally called by the event filter installed on the
    parent object, but may also be called directly.
*/
void QActionStore::sendKeyEvent(Qt::Key key, bool isPressed)
{
    Q_D(QActionStore);
    for (int j = 0; j < d->actions.size(); j++) {
        auto aKey = d->actions.keys()[j];
        auto action = d->actions[aKey];
        for (auto keyAction : action.keys) {
            if (keyAction.key == key && keyAction.isPressed == isPressed)
            {
                Q_EMIT actionEvent(action.name);
                Q_EMIT actionKeyEvent(action.name, key, isPressed);
            }
            if (d->actions.size() == 0)
                break;
        }
        if (d->actions.size() == 0)
            break;
    }
}

/*!
    Delivers a mouse button event for \a button with the given \a isPressed
    state to the store, emitting actionEvent() and actionMouseButtonEvent() for
    any matching action.
*/
void QActionStore::sendMouseButtonEvent(Qt::MouseButton button, bool isPressed)
{
    Q_D(QActionStore);
    for (int j = 0; j < d->actions.size(); j++) {
        auto aKey = d->actions.keys()[j];
        auto action = d->actions[aKey];
        for (auto mouseButtonAction : action.mouseButtons) {
            if (mouseButtonAction.button == button && mouseButtonAction.isPressed == isPressed) {
                Q_EMIT actionEvent(action.name);
                Q_EMIT actionMouseButtonEvent(action.name, button, isPressed);
            }
            if (d->actions.size() == 0)
                break;
        }
        if (d->actions.size() == 0)
            break;
    }
}

// ActionBuilder
QActionStore::ActionBuilder::ActionBuilder(const QString &name)
    : m_action({name, {}, {}, {}, {}})
{
}

/*!
    Adds an axis trigger for \a axis in \a direction on \a device, activating
    once the axis passes \a deadzone. Returns a reference to this builder.
*/
QActionStore::ActionBuilder &QActionStore::ActionBuilder::addAxis(Controller device, JoyAxis axis, AxisDirection direction, float deadzone)
{
    m_action.axes.push_back({device, axis, direction, deadzone});
    return *this;
}

/*!
    Adds a button trigger for \a button on \a device with the given
    \a isPressed state. Returns a reference to this builder.
*/
QActionStore::ActionBuilder &QActionStore::ActionBuilder::addButton(Controller device, JoyButton button, bool isPressed)
{
    m_action.buttons.push_back({device, button, isPressed});
    return *this;
}

/*!
    Adds a keyboard trigger for \a key with the given \a isPressed state.
    Returns a reference to this builder.
*/
QActionStore::ActionBuilder &QActionStore::ActionBuilder::addKey(Qt::Key key, bool isPressed)
{
    m_action.keys.push_back({key, isPressed});
    return *this;
}

/*!
    Adds a mouse trigger for \a button with the given \a isPressed state.
    Returns a reference to this builder.
*/
QActionStore::ActionBuilder &QActionStore::ActionBuilder::addMouseButton(Qt::MouseButton button, bool isPressed)
{
    m_action.mouseButtons.push_back({button, isPressed});
    return *this;
}

/*!
    Returns the assembled action, ready to be passed to
    QActionStore::registerAction().
*/
QActionStore::Action QActionStore::ActionBuilder::build() const
{
    return m_action;
}

/*!
    \fn void QActionStore::actionEvent(const QString &action)

    This signal is emitted whenever the registered action named \a action is
    triggered by any of its inputs.
*/

/*!
    \fn void QActionStore::actionKeyEvent(const QString &action, Qt::Key key, bool isPressed)

    This signal is emitted when \a action is triggered by the keyboard \a key,
    with the given \a isPressed state.
*/

/*!
    \fn void QActionStore::actionMouseButtonEvent(const QString &action, Qt::MouseButton button, bool isPressed)

    This signal is emitted when \a action is triggered by the mouse \a button,
    with the given \a isPressed state.
*/

/*!
    \fn void QActionStore::actionJoyButtonEvent(const QString &action, int device, QUniversalInput::JoyButton button, bool isPressed)

    This signal is emitted when \a action is triggered by \a button on the
    device at index \a device, with the given \a isPressed state.
*/

/*!
    \fn void QActionStore::actionJoyAxisEvent(const QString &action, int device, QUniversalInput::JoyAxis axis, float value)

    This signal is emitted when \a action is triggered by \a axis on the device
    at index \a device, reaching \a value.
*/

QT_END_NAMESPACE

#include "moc_qactionstore.cpp"
