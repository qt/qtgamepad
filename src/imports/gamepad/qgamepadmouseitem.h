/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Gamepad module
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QGAMEPADMOUSEITEM_H
#define QGAMEPADMOUSEITEM_H

#include <QtQuick/QQuickItem>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>

QT_BEGIN_NAMESPACE

class QGamepad;
class QGamepadManager;
class QGamepadMouseItem : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(GamepadJoystick)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QGamepad* gamepad READ gamepad WRITE setGamepad NOTIFY gamepadChanged)
    Q_PROPERTY(GamepadJoystick joystick READ joystick WRITE setJoystick NOTIFY joystickChanged)
    Q_PROPERTY(double deadZoneSize READ deadZoneSize WRITE setDeadZoneSize NOTIFY deadZoneSizeChanged)
    Q_PROPERTY(QPointF mousePosition READ mousePosition NOTIFY mousePositionChanged)

public:
    enum GamepadJoystick {
        LeftStick,
        RightStick,
        Both
    };

    explicit QGamepadMouseItem(QQuickItem *parent = nullptr);

    bool active() const;
    QGamepad* gamepad() const;
    GamepadJoystick joystick() const;
    double deadZoneSize() const;
    QPointF mousePosition() const;

signals:

    void activeChanged(bool isActive);
    void gamepadChanged(QGamepad* gamepad);
    void joystickChanged(GamepadJoystick joystick);
    void deadZoneSizeChanged(double size);
    void mousePositionChanged(QPointF position);

public slots:

    void setActive(bool arg);
    void setGamepad(QGamepad* gamepad);
    void setJoystick(GamepadJoystick joystick);
    void setDeadZoneSize(double size);

    void mouseButtonPressed(int button);
    void mouseButtonReleased(int button);

private slots:
    void updateMousePostion();
    void processMouseMoveEvent(QPointF position);
    void processMouseButtonEvent(bool isPressed, Qt::MouseButton button = Qt::LeftButton);

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private:
    void sendGeneratedMouseEvent(QMouseEvent *event);

    QPointF m_mousePosition;
    QTimer m_updateTimer;
    QElapsedTimer m_deltaTimer;
    Qt::MouseButtons m_mouseButtons;

    bool m_active;
    QGamepad* m_gamepad;
    GamepadJoystick m_joystick;
    double m_deadZoneSize;


};

QT_END_NAMESPACE

#endif // QGAMEPADMOUSEITEM_H
