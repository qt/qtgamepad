// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QACTIONSTORE_H
#define QACTIONSTORE_H

#include <QtUniversalInput/quniversalinput.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

class QActionStorePrivate;

class Q_UNIVERSALINPUT_EXPORT QActionStore : public QObject
{
    Q_OBJECT
public:
    enum class Controller
    {
        All = -1,
        Device0 = 0,
        Device1 = 1,
        Device2 = 2,
        Device3 = 3,
        Device4 = 4,
        Device5 = 5,
        Device6 = 6,
        Device7 = 7,
        Device8 = 8,
        Device9 = 9,
        Device10 = 10,
        Device11 = 11,
        Device12 = 12,
        Device13 = 13,
        Device14 = 14,
        Device15 = 15,
        Device16 = 16,
        DeviceMAX = 17,
    };
    Q_ENUM(Controller)

    enum class AxisDirection
    {
        All = -1,
        Up = 0,
        Right = 1,
        Down = 2,
        Left = 3,
        Max = 4,
    };
    Q_ENUM(AxisDirection)

    struct JoyButtonAction
    {
        Controller device = Controller::All;
        QUniversalInput::JoyButton button = QUniversalInput::JoyButton::Invalid;
        bool isPressed = false;
    };

    struct JoyAxisAction
    {
        Controller device = Controller::All;
        QUniversalInput::JoyAxis axis = QUniversalInput::JoyAxis::Invalid;
        AxisDirection direction = AxisDirection::Max;
        float deadzone = 0.5f;
    };

    struct KeyEventAction
    {
        Qt::Key key = Qt::Key_unknown;
        bool isPressed = false;
    };

    struct MouseButtonAction
    {
        Qt::MouseButton button = Qt::NoButton;
        bool isPressed = false;
    };

    struct Action
    {
        QString name = u""_s;
        QList<JoyButtonAction> buttons;
        QList<JoyAxisAction> axes;
        QList<KeyEventAction> keys;
        QList<MouseButtonAction> mouseButtons;
    };

    struct Q_UNIVERSALINPUT_EXPORT ActionBuilder
    {
        ActionBuilder(const QString &name);
        ActionBuilder &addAxis(Controller device, QUniversalInput::JoyAxis axis, AxisDirection direction, float deadzone);
        ActionBuilder &addButton(Controller device, QUniversalInput::JoyButton button, bool isPressed = true);
        ActionBuilder &addKey(Qt::Key key, bool isPressed = true);
        ActionBuilder &addMouseButton(Qt::MouseButton button, bool isPressed = true);

        Action build() const;

    private:
        Action m_action;
    };

    explicit QActionStore(QObject *parent = nullptr);
    ~QActionStore();

    void registerAction(const Action &action);
    void clearActions();

Q_SIGNALS:
    void actionEvent(const QString &action);
    void actionKeyEvent(const QString &action, Qt::Key key, bool isPressed);
    void actionMouseButtonEvent(const QString &action, Qt::MouseButton button, bool isPressed);
    void actionJoyButtonEvent(const QString &action, int device, QUniversalInput::JoyButton button, bool isPressed);
    void actionJoyAxisEvent(const QString &action, int device, QUniversalInput::JoyAxis axis, float value);

public Q_SLOTS:
    void sendKeyEvent(Qt::Key key, bool isPressed = true);
    void sendMouseButtonEvent(Qt::MouseButton button, bool isPressed = true);

private:
    Q_DECLARE_PRIVATE(QActionStore)
    Q_DISABLE_COPY_MOVE(QActionStore)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QActionStore *)
Q_DECLARE_METATYPE(QActionStore::Controller)
Q_DECLARE_METATYPE(QActionStore::AxisDirection)
Q_DECLARE_METATYPE(QActionStore::JoyButtonAction)
Q_DECLARE_METATYPE(QActionStore::JoyAxisAction)
Q_DECLARE_METATYPE(QActionStore::KeyEventAction)
Q_DECLARE_METATYPE(QActionStore::MouseButtonAction)
Q_DECLARE_METATYPE(QActionStore::Action)

#endif // QACTIONSTORE_H
