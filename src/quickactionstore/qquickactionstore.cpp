// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickactionstore_p.h"

QT_BEGIN_NAMESPACE

// Events

QQuickInputActionEvent::QQuickInputActionEvent(QObject *parent)
    : QObject(parent)
{
}

QQuickInputJoyButtonEvent::QQuickInputJoyButtonEvent(QObject *parent)
    : QQuickInputActionEvent(parent)
{
}

int QQuickInputJoyButtonEvent::device() const
{
    return m_device;
}

void QQuickInputJoyButtonEvent::setDevice(int device)
{
    m_device = device;
    emit deviceChanged(device);
}

int QQuickInputJoyButtonEvent::button() const
{
    return m_button;
}

void QQuickInputJoyButtonEvent::setButton(int button)
{
    m_button = button;
    emit buttonChanged(button);
}

bool QQuickInputJoyButtonEvent::isPressed() const
{
    return m_pressed;
}

void QQuickInputJoyButtonEvent::setPressed(bool pressed)
{
    m_pressed = pressed;
    emit isPressedChanged(pressed);
}

QQuickInputKeyEvent::QQuickInputKeyEvent(QObject *parent)
    : QQuickInputActionEvent(parent)
{
}

Qt::Key QQuickInputKeyEvent::key() const
{
    return m_key;
}

void QQuickInputKeyEvent::setKey(Qt::Key key)
{
    m_key = key;
    emit keyChanged(key);
}

bool QQuickInputKeyEvent::isPressed() const
{
    return m_isPressed;
}

void QQuickInputKeyEvent::setPressed(bool pressed)
{
    m_isPressed = pressed;
    emit isPressedChanged(pressed);
}

QQuickInputMouseButtonEvent::QQuickInputMouseButtonEvent(QObject *parent)
    : QQuickInputActionEvent(parent)
{
}

int QQuickInputMouseButtonEvent::button() const
{
    return m_button;
}

void QQuickInputMouseButtonEvent::setButton(int button)
{
    m_button = button;
    emit buttonChanged(button);
}

bool QQuickInputMouseButtonEvent::isPressed() const
{
    return m_isPressed;
}

void QQuickInputMouseButtonEvent::setPressed(bool pressed)
{
    m_isPressed = pressed;
    emit isPressedChanged(pressed);
}

QQuickInputJoyAxisEvent::QQuickInputJoyAxisEvent(QObject *parent)
    : QQuickInputActionEvent(parent)
{
}

int QQuickInputJoyAxisEvent::device() const
{
    return m_device;
}
void QQuickInputJoyAxisEvent::setDevice(int device)
{
    m_device = device;
    emit deviceChanged();
}

int QQuickInputJoyAxisEvent::axis() const
{
    return m_axis;
}

void QQuickInputJoyAxisEvent::setAxis(int axis)
{
    m_axis = axis;
    emit axisChanged();
}

int QQuickInputJoyAxisEvent::direction() const
{
    return m_direction;
}

void QQuickInputJoyAxisEvent::setDirection(int direction)
{
    m_direction = direction;
    emit directionChanged();
}

float QQuickInputJoyAxisEvent::deadzone() const {
    return m_deadzone;
}

void QQuickInputJoyAxisEvent::setDeadzone(float deadzone)
{
    m_deadzone = deadzone;
    emit deadzoneChanged();
}

// Actions

QQuickInputAction::QQuickInputAction(QObject *parent) : QObject(parent)
{
}

QString QQuickInputAction::title() const
{
    return m_title;
}

void QQuickInputAction::setTitle(const QString &title)
{
    m_title = title;
    emit titleChanged(title);
}

QQmlListProperty<QQuickInputActionEvent> QQuickInputAction::events()
{
    return QQmlListProperty<QQuickInputActionEvent>(this, nullptr, &QQuickInputAction::appendEvent, &QQuickInputAction::countEvent, &QQuickInputAction::atEvent, &QQuickInputAction::clearEvent);
}

void QQuickInputAction::appendEvent(QQmlListProperty<QQuickInputActionEvent> *list, QQuickInputActionEvent *event)
{
    if (event == nullptr)
        return;
    auto *self = static_cast<QQuickInputAction *>(list->object);
    self->m_events.push_back(event);
    emit self->eventsChanged();

    if (auto *joyButtonEvent = qobject_cast<QQuickInputJoyButtonEvent *>(event)) {
        connect(joyButtonEvent, &QQuickInputJoyButtonEvent::deviceChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(joyButtonEvent, &QQuickInputJoyButtonEvent::buttonChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(joyButtonEvent, &QQuickInputJoyButtonEvent::isPressedChanged, self, [self]()
                { emit self->eventsChanged(); });
    }

    if (auto *keyEvent = qobject_cast<QQuickInputKeyEvent *>(event)) {
        connect(keyEvent, &QQuickInputKeyEvent::keyChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(keyEvent, &QQuickInputKeyEvent::isPressedChanged, self, [self]()
                { emit self->eventsChanged(); });
    }

    if (auto *joyAxisEvent = qobject_cast<QQuickInputJoyAxisEvent *>(event)) {
        connect(joyAxisEvent, &QQuickInputJoyAxisEvent::deviceChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(joyAxisEvent, &QQuickInputJoyAxisEvent::axisChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(joyAxisEvent, &QQuickInputJoyAxisEvent::directionChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(joyAxisEvent, &QQuickInputJoyAxisEvent::deadzoneChanged, self, [self]()
                { emit self->eventsChanged(); });
    }

    if (auto *mouseButtonEvent = qobject_cast<QQuickInputMouseButtonEvent *>(event)) {
        connect(mouseButtonEvent, &QQuickInputMouseButtonEvent::buttonChanged, self, [self]()
                { emit self->eventsChanged(); });
        connect(mouseButtonEvent, &QQuickInputMouseButtonEvent::isPressedChanged, self, [self]()
                { emit self->eventsChanged(); });
    }
}
qsizetype QQuickInputAction::countEvent(QQmlListProperty<QQuickInputActionEvent> *list)
{
    auto *self = static_cast<QQuickInputAction *>(list->object);
    return self->m_events.size();
}
QQuickInputActionEvent *QQuickInputAction::atEvent(QQmlListProperty<QQuickInputActionEvent> *list, qsizetype index)
{
    auto *self = static_cast<QQuickInputAction *>(list->object);
    return self->m_events.at(index);
}
void QQuickInputAction::clearEvent(QQmlListProperty<QQuickInputActionEvent> *list)
{
    auto *self = static_cast<QQuickInputAction *>(list->object);
    self->m_events.clear();
    emit self->eventsChanged();
}

QActionStore::Action QQuickInputAction::action() const
{
    QActionStore::ActionBuilder builder(m_title);

    for (auto *event : m_events) {
        if (auto *joyButtonEvent = qobject_cast<QQuickInputJoyButtonEvent *>(event))
            builder.addButton(QActionStore::Controller(joyButtonEvent->device()), QUniversalInput::JoyButton(joyButtonEvent->button()), joyButtonEvent->isPressed());
        if (auto *keyEvent = qobject_cast<QQuickInputKeyEvent *>(event))
            builder.addKey(keyEvent->key(), keyEvent->isPressed());
        if (auto *joyAxisEvent = qobject_cast<QQuickInputJoyAxisEvent *>(event))
            builder.addAxis(QActionStore::Controller(joyAxisEvent->device()), QUniversalInput::JoyAxis(joyAxisEvent->axis()), QActionStore::AxisDirection(joyAxisEvent->direction()), joyAxisEvent->deadzone());
        if (auto *mouseButtonEvent = qobject_cast<QQuickInputMouseButtonEvent *>(event))
            builder.addMouseButton(Qt::MouseButton(mouseButtonEvent->button()), mouseButtonEvent->isPressed());
    }

    return builder.build();
}

QQuickActionStore::QQuickActionStore(QObject *parent)
    : QActionStore(parent)
{
}

QQmlListProperty<QQuickInputAction> QQuickActionStore::actions()
{
    return QQmlListProperty<QQuickInputAction>(this,
                                               nullptr,
                                               QQuickActionStore::appendAction,
                                               QQuickActionStore::countAction,
                                               QQuickActionStore::atAction,
                                               QQuickActionStore::clearAction);
}

void QQuickActionStore::appendAction(QQmlListProperty<QQuickInputAction> *list, QQuickInputAction *action)
{
    if (action == nullptr)
        return;

    auto *self = static_cast<QQuickActionStore *>(list->object);
    self->m_actions.push_back(action);

    auto actionItem = action->action();
    self->registerAction(actionItem);

    connect(action, &QQuickInputAction::titleChanged, self, [self, actionItem](){
        self->clearActions();
        for (auto *action : self->m_actions){
            self->registerAction(action->action());
        }
    });
    connect(action, &QQuickInputAction::eventsChanged, self, [self, actionItem](){
        self->clearActions();
        for (auto *action : self->m_actions){
            self->registerAction(action->action());
        }
    });

    emit self->actionsChanged();
}

qsizetype QQuickActionStore::countAction(QQmlListProperty<QQuickInputAction> *list)
{
    auto *self = static_cast<QQuickActionStore *>(list->object);
    return self->m_actions.size();
}

QQuickInputAction *QQuickActionStore::atAction(QQmlListProperty<QQuickInputAction> *list, qsizetype index)
{
    auto *self = static_cast<QQuickActionStore *>(list->object);
    return self->m_actions.at(index);
}

void QQuickActionStore::clearAction(QQmlListProperty<QQuickInputAction> *list)
{
    auto *self = static_cast<QQuickActionStore *>(list->object);
    self->clearActions();
    self->m_actions.clear();

    emit self->actionsChanged();
}

QT_END_NAMESPACE
