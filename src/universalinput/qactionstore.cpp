// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qactionstore.h"

#include <private/qobject_p.h>

#include <QKeyEvent>

QT_BEGIN_NAMESPACE

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

QActionStore::QActionStore(QObject *parent)
    : QObject(*new QActionStorePrivate, parent)
{
    Q_D(QActionStore);
    d->q_ptr = this;

    auto input = QUniversalInput::instance();
    connect(input, SIGNAL(joyAxisEvent(int, JoyAxis, float)), this, SLOT(_q_handleJoyAxisEvent(int, JoyAxis, float)));
    connect(input, SIGNAL(joyButtonEvent(int, JoyButton, bool)), this, SLOT(_q_handleJoyButtonEvent(int, JoyButton, bool)));

    if (parent)
        parent->installEventFilter(this);
}

QActionStore::~QActionStore()
{
    if (parent())
        parent()->removeEventFilter(this);
}

void QActionStore::registerAction(const Action &action)
{
    Q_D(QActionStore);
    d->actions.insert(action.name, action);
}

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

QActionStore::ActionBuilder &QActionStore::ActionBuilder::addAxis(Controller device, JoyAxis axis, AxisDirection direction, float deadzone)
{
    m_action.axes.push_back({device, axis, direction, deadzone});
    return *this;
}

QActionStore::ActionBuilder &QActionStore::ActionBuilder::addButton(Controller device, JoyButton button, bool isPressed)
{
    m_action.buttons.push_back({device, button, isPressed});
    return *this;
}

QActionStore::ActionBuilder &QActionStore::ActionBuilder::addKey(Qt::Key key, bool isPressed)
{
    m_action.keys.push_back({key, isPressed});
    return *this;
}

QActionStore::ActionBuilder &QActionStore::ActionBuilder::addMouseButton(Qt::MouseButton button, bool isPressed)
{
    m_action.mouseButtons.push_back({button, isPressed});
    return *this;
}

QActionStore::Action QActionStore::ActionBuilder::build() const
{
    return m_action;
}


QT_END_NAMESPACE

#include "moc_qactionstore.cpp"
