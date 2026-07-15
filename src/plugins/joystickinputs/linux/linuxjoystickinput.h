// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "platform/linuxbsd/joypad_linux.h" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#ifndef LINUXJOYSTICKINPUT_H
#define LINUXJOYSTICKINPUT_H

#include <QtUniversalInput/private/qjoystickinput_p.h>

#include <QtCore/QElapsedTimer>

#include <vector>

// for JoypadEvent
#include <QtUniversalInput/private/qjoystickinput_p.h>
#include <quniversalinput.h>

struct udev;
struct input_absinfo;

QT_BEGIN_NAMESPACE

class QSocketNotifier;

class LinuxJoystickInput : public QJoystickInput
{
    Q_OBJECT
public:
    LinuxJoystickInput();
    ~LinuxJoystickInput();

    void probeJoypads();
    void processJoypads();
    void processJoypad(int id);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    enum {
        JOYPADS_MAX = 16,
        JOY_AXIS_COUNT = 6,
        MIN_JOY_AXIS = 10,
        MAX_JOY_AXIS = 32768,
        MAX_JOY_BUTTONS = 128,
        KEY_EVENT_BUFFER_SIZE = 512,
        MAX_TRIGGER = 1023, // was 255, but xbox one controller max is 1023

        // from godot linux_joystick.h
        MAX_ABS = 63,
        MAX_KEY = 767, // Hack because <linux/input.h> can't be included here
    };

    struct gamepad {
        int id = -1;
        bool attached = false;
        bool confirmed = false;
        int key_map[MAX_KEY];
        int joy_axis[JOY_AXIS_COUNT];

        QUniversalInput::HatMask dpad;

        int fd = -1;
        QSocketNotifier *notifier = nullptr;
        QString devpath;

        input_absinfo *abs_info[MAX_ABS] = {};

        bool force_feedback = false;
        int ff_effect_id = -1;
        bool vibrating = false;
    };

    void setupJoypadObject(const QString& name);
    void setupJoypadProperties(gamepad* joy);
    void closeJoypads();
    void closeJoypad(const char *p_devpath);
    void closeJoypad(gamepad &p_joypad, int p_id);

    void joypadVibrationStart(gamepad &p_joypad, float p_weak_magnitude, float p_strong_magnitude, float p_duration, uint64_t p_timestamp);
    void joypadVibrationStop(gamepad &p_joypad, uint64_t p_timestamp);

    struct udev *m_udev = nullptr;
    gamepad m_joypads[JOYPADS_MAX]; // joypad joystick gamestick tomatoe potatoe
    std::vector<QString> m_attached_devices;
    QElapsedTimer m_elapsedTimer;
};

QT_END_NAMESPACE

#endif // LINUXJOYSTICKINPUT_H
