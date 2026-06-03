// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTQUICKACTIONSTORE_H
#define QTQUICKACTIONSTORE_H

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

#include <QtUniversalInput/QActionStore>
#include <QtQml/QQmlEngine>

#include <QtQml/QQmlListProperty>

QT_BEGIN_NAMESPACE

// Expose the QUniversalInput enums (JoyButton, JoyAxis, HatMask, ...) to QML
// so the event types can be configured with named values such as
// UniversalInput.JoyButton.A. The QActionStore enums (Controller,
// AxisDirection) are reachable through the ActionStore element itself.
struct QUniversalInputForeign
{
    Q_GADGET
    QML_FOREIGN(QUniversalInput)
    QML_NAMED_ELEMENT(UniversalInput)
    QML_UNCREATABLE("UniversalInput is only used to access its enums")
};

class QQuickInputActionEvent : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ActionEvent)
public:
    QQuickInputActionEvent(QObject *parent = nullptr);
    virtual ~QQuickInputActionEvent() = default;
};

class QQuickInputJoyButtonEvent : public QQuickInputActionEvent
{
    Q_OBJECT
    QML_NAMED_ELEMENT(JoyButtonEvent)
    Q_PROPERTY(int device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(int button READ button WRITE setButton NOTIFY buttonChanged)
    Q_PROPERTY(bool isPressed READ isPressed WRITE setPressed NOTIFY isPressedChanged)

public:
    QQuickInputJoyButtonEvent(QObject *parent = nullptr);
    ~QQuickInputJoyButtonEvent() override = default;

    int device() const;
    void setDevice(int device);

    int button() const;
    void setButton(int button);

    bool isPressed() const;
    void setPressed(bool pressed);

Q_SIGNALS:
    void deviceChanged(int device);
    void buttonChanged(int button);
    void isPressedChanged(bool isPressed);

private:
    int m_device;
    int m_button;
    bool m_pressed;
};

class QQuickInputKeyEvent : public QQuickInputActionEvent
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KeyboardEvent)
    Q_PROPERTY(Qt::Key key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(bool isPressed READ isPressed WRITE setPressed NOTIFY isPressedChanged)

public:
    QQuickInputKeyEvent(QObject *parent = nullptr);
    ~QQuickInputKeyEvent() override = default;

    Qt::Key key() const;
    void setKey(Qt::Key key);

    bool isPressed() const;
    void setPressed(bool pressed);

Q_SIGNALS:
    void keyChanged(Qt::Key key);
    void isPressedChanged(bool isPressed);

private:
    Qt::Key m_key = Qt::Key_unknown;
    bool m_isPressed = false;
};

class QQuickInputMouseButtonEvent : public QQuickInputActionEvent
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MouseButtonEvent)
    Q_PROPERTY(int button READ button WRITE setButton NOTIFY buttonChanged)
    Q_PROPERTY(bool isPressed READ isPressed WRITE setPressed NOTIFY isPressedChanged)
public:
    QQuickInputMouseButtonEvent(QObject *parent = nullptr);
    ~QQuickInputMouseButtonEvent() override = default;

    int button() const;
    void setButton(int button);

    bool isPressed() const;
    void setPressed(bool pressed);

Q_SIGNALS:
    void buttonChanged(int button);
    void isPressedChanged(bool isPressed);

private:
    int m_button = 0;
    bool m_isPressed = false;
};

class QQuickInputJoyAxisEvent : public QQuickInputActionEvent
{
    Q_OBJECT
    QML_NAMED_ELEMENT(JoyAxisEvent)
    Q_PROPERTY(int device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(int axis READ axis WRITE setAxis NOTIFY axisChanged)
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(float deadzone READ deadzone WRITE setDeadzone NOTIFY deadzoneChanged)

public:
    QQuickInputJoyAxisEvent(QObject *parent = nullptr);
    ~QQuickInputJoyAxisEvent() override = default;

    int device() const;
    void setDevice(int device);

    int axis() const;
    void setAxis(int axis);

    int direction() const;
    void setDirection(int direction);

    float deadzone() const;
    void setDeadzone(float deadzone);

Q_SIGNALS:
    void deviceChanged();
    void axisChanged();
    void directionChanged();
    void deadzoneChanged();

private:
    int m_device = 0;
    int m_axis = 0;
    int m_direction = 0;
    float m_deadzone = 0.5f;
};

class QQuickInputAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QQmlListProperty<QQuickInputActionEvent> events READ events NOTIFY eventsChanged)
    Q_CLASSINFO("DefaultProperty", "events")
    QML_NAMED_ELEMENT(InputAction)
public:
    QQuickInputAction(QObject *parent = nullptr);
    ~QQuickInputAction() override = default;

    QQmlListProperty<QQuickInputActionEvent> events();
    static void appendEvent(QQmlListProperty<QQuickInputActionEvent> *list, QQuickInputActionEvent *event);
    static qsizetype countEvent(QQmlListProperty<QQuickInputActionEvent> *list);
    static QQuickInputActionEvent *atEvent(QQmlListProperty<QQuickInputActionEvent> *list, qsizetype index);
    static void clearEvent(QQmlListProperty<QQuickInputActionEvent> *list);

    QString title() const;
    void setTitle(const QString &title);

    QActionStore::Action action() const;

Q_SIGNALS:
    void titleChanged(const QString &title);
    void eventsChanged();

private:
    QString m_title;
    QList<QQuickInputActionEvent *> m_events;
};

class QQuickActionStore : public QActionStore
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QQuickInputAction> actions READ actions)
    Q_CLASSINFO("DefaultProperty", "actions")
    QML_NAMED_ELEMENT(ActionStore)

public:
    QQuickActionStore(QObject *parent = nullptr);
    ~QQuickActionStore() override = default;

    QQmlListProperty<QQuickInputAction> actions();
    static void appendAction(QQmlListProperty<QQuickInputAction> *list, QQuickInputAction *action);
    static qsizetype countAction(QQmlListProperty<QQuickInputAction> *list);
    static QQuickInputAction *atAction(QQmlListProperty<QQuickInputAction> *list, qsizetype index);
    static void clearAction(QQmlListProperty<QQuickInputAction> *list);

Q_SIGNALS:
    void actionsChanged();

private:
    QList<QQuickInputAction *> m_actions;
};

QT_END_NAMESPACE

#endif // QTQUICKACTIONSTORE_H
