// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "core/input/input.h" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#ifndef QUNIVERSALINPUT_H
#define QUNIVERSALINPUT_H

#include <QtCore/QObject>
#include <QtCore/QElapsedTimer>
#include <QtCore/QString>
#include <QtGui/QVector2D>
#include <QtUniversalInput/qtuniversalinputglobal.h>
#include <QtGui/QMouseEvent>

#ifdef QT_BUILD_INTERNAL
// In the global namespace, so the friend declaration below still refers to the
// autotest when Qt is built into a namespace.
class tst_QMappingTransform;
#endif

QT_BEGIN_NAMESPACE

class QUniversalInputPrivate;
class Q_UNIVERSALINPUT_EXPORT QUniversalInput : public QObject
{
    Q_OBJECT
    // The enums share value names (e.g. HatDirection::Up and HatMask::Up), so
    // expose them to QML only through their scope, e.g. JoyButton.A.
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
public:
    enum class HatDirection {
        Up = 0,
        Right = 1,
        Down = 2,
        Left = 3,
        Max = 4,
    };
    Q_ENUM(HatDirection)

    enum class HatFlag {
        Center = 0x0,
        Up = 0x1,
        Right = 0x2,
        Down = 0x4,
        Left = 0x8,
    };
    Q_DECLARE_FLAGS(HatMask, HatFlag)
    Q_FLAG(HatMask)

    enum class JoyAxis {
        Invalid = -1,
        LeftX = 0,
        LeftY = 1,
        RightX = 2,
        RightY = 3,
        TriggerLeft = 4,
        TriggerRight = 5,
    };
    Q_ENUM(JoyAxis)

    enum class JoyButton {
        Invalid = -1,
        A = 0,
        B = 1,
        X = 2,
        Y = 3,
        Back = 4,
        Guide = 5,
        Start = 6,
        LeftStick = 7,
        RightStick = 8,
        LeftShoulder = 9,
        RightShoulder = 10,
        DpadUp = 11,
        DpadDown = 12,
        DpadLeft = 13,
        DpadRight = 14,
        Misc1 = 15,
        Paddle1 = 16,
        Paddle2 = 17,
        Paddle3 = 18,
        Paddle4 = 19,
        Touchpad = 20,
    };
    Q_ENUM(JoyButton)

    /*! \internal */
    enum {
        JoypadsMax = 16,
        JoyAxesMax = 10,
        JoyButtonsMax = 128,
    };

    /*! \internal */
    struct Action {
        quint64 frame;
        bool isPressed;
        bool isExact;
        float strength;
        float rawStrength;
    };
    /*! \internal */
    struct VibrationInfo {
        float weakMagnitude;
        float strongMagnitude;
        float duration;
        quint64 timestamp;
    };
    /*! \internal */
    struct VelocityTrack {
        QElapsedTimer frameTimer;
        QVector2D velocity;
        QVector2D accum;
        float accumTime = 0.0;
        float minRefFrame;
        float maxRefFrame;

        void update(const QVector2D &delta);
        void reset();
        VelocityTrack();
    };

    /*! \internal */
    struct Joypad {
        QString name;
        QString uid;
        bool isConnected = false;
        bool lastButtons[JoyButtonsMax] = { false };
        float lastAxis[JoyAxesMax] = { 0.0f };
        HatMask lastHat = HatFlag::Center;
        int mapping = -1;
        int hatCurrent = 0;
    };

    enum JoyType {
        TypeButton,
        TypeAxis,
        TypeHat,
        TypeMax,
    };

    enum JoyAxisRange {
        NegativeHalfAxis = -1,
        FullAxis = 0,
        PositiveHalfAxis = 1
    };

    /*! \internal */
    struct JoyEvent {
        int type = TypeMax;
        int index = -1;
        float value = 0.0f;
    };

    /*! \internal */
    struct JoyBinding {
        JoyType inputType;
        union {
            JoyButton button;

            struct {
                JoyAxis axis;
                JoyAxisRange range;
                bool invert;
            } axis;

            struct {
                HatDirection hat;
                HatMask hat_mask;
            } hat;

        } input;

        JoyType outputType;
        union {
            JoyButton button;

            struct {
                JoyAxis axis;
                JoyAxisRange range;
            } axis;

        } output;
    };

    /*! \internal */
    struct JoyDeviceMapping {
        QString uid;
        QString name;
        QVector<JoyBinding> bindings;
    };

    static QUniversalInput *instance();

    QString joyName(int device) const;
    bool isJoyConnected(int device) const;
    bool isGamepad(int device) const;

    // API used by platform specific plugins
    // Joypad/Joystick/Gamepads
    int unusedJoyId();
    void updateJoyConnection(int index, bool isConnected, const QString &name, const QString &guid = QString());

    void joyButton(int device, JoyButton button, bool isPressed);
    void joyAxis(int device, JoyAxis axis, float value);
    void joyHat(int device, HatMask value);

    // Force Feedback
    QVector2D joyVibrationStrength(int device);
    float joyVibrationDuration(int device);
    quint64 joyVibrationTimestamp(int device);
    void addForce(int device, QVector2D strength, float duration);

    void setJoyAxis(int device, JoyAxis axis, float value);

    void setMouseDisabled(bool disabled);
    bool isMouseDisabled() const;
    void mouseMove(const QVector2D &deltas);

Q_SIGNALS:
    void joyConnectionChanged(int index, bool isConnected);
    void joyButtonEvent(int device, JoyButton button, bool isPressed);
    void joyAxisEvent(int device, JoyAxis axis, float value);
    void joyVibrationRequested(int device);

    void mouseDisabledChanged();
    void mouseMovedWithDeltas(const QVector2D &deltas);

private Q_SLOTS:
    void loadPlugins();

private:
    QUniversalInput();
    ~QUniversalInput();

    void sendButtonEvent(int device, JoyButton index, bool pressed);
    void sendAxisEvent(int device, JoyAxis axis, float value);
    JoyEvent mappedButtonEvent(const JoyDeviceMapping &mapping, JoyButton button);
    JoyEvent mappedAxisEvent(const JoyDeviceMapping &mapping, JoyAxis axis, float inValue, JoyAxisRange *outRange = nullptr);
    void mappedHatEvents(const JoyDeviceMapping &mapping, HatDirection hat, JoyEvent events[size_t(HatDirection::Max)]);

#ifdef QT_BUILD_INTERNAL
    friend class ::tst_QMappingTransform;
#endif

    Q_DECLARE_PRIVATE(QUniversalInput)
    Q_DISABLE_COPY_MOVE(QUniversalInput)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QUniversalInput::HatMask)

Q_UNIVERSALINPUT_EXPORT QDebug operator<<(QDebug debug, const QUniversalInput::JoyButton &joyButton);
Q_UNIVERSALINPUT_EXPORT QDebug operator<<(QDebug debug, const QUniversalInput::JoyAxis &axis);
Q_UNIVERSALINPUT_EXPORT QDebug operator<<(QDebug debug, const QUniversalInput::JoyAxisRange &range);
Q_UNIVERSALINPUT_EXPORT QDebug operator<<(QDebug debug, const QUniversalInput::HatDirection &hatDirection);
Q_UNIVERSALINPUT_EXPORT QDebug operator<<(QDebug debug, const QUniversalInput::JoyBinding &binding);
Q_UNIVERSALINPUT_EXPORT QDebug operator<<(QDebug debug, const QUniversalInput::JoyDeviceMapping &mapping);



QT_END_NAMESPACE

Q_DECLARE_METATYPE(QUniversalInput::JoyButton)
Q_DECLARE_METATYPE(QUniversalInput::JoyAxis)
Q_DECLARE_METATYPE(QUniversalInput::HatDirection)
Q_DECLARE_METATYPE(QUniversalInput::HatMask)

#endif // QUNIVERSALINPUT_H
