// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "platform/linuxbsd/joypad_linux.cpp" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#include "linuxjoystickinput.h"

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

#include <libudev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcUniversalInput, "qt.universalinput")

// Bring the QUniversalInput input enums into scope for this file.
using HatDirection = QUniversalInput::HatDirection;
using HatFlag = QUniversalInput::HatFlag;
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

// GODOT begin
#define LONG_BITS (sizeof(long) * 8)
#define NBITS(x) ((((x)-1) / LONG_BITS) + 1)
#define test_bit(nr, addr) (((1UL << ((nr) % LONG_BITS)) & ((addr)[(nr) / LONG_BITS])) != 0)

static QString ignore_str = u"js"_s;
// GODOT end

LinuxJoystickInput::LinuxJoystickInput()
    : m_udev(nullptr)
{
    m_udev = udev_new();
    if (!m_udev) {
        qCWarning(lcUniversalInput) << "Could not initialize udev";
        m_udev = nullptr; // ensure udev is nullptr
    }

    // Pick up devices that are already connected, then watch for hotplug
    // events instead of re-enumerating on a timer.
    probeJoypads();
    setupMonitor();

    // Rumble is requested from the main thread via QUniversalInput::addForce();
    // start/stop the effect on demand rather than polling for it.
    connect(QUniversalInput::instance(), &QUniversalInput::joyVibrationRequested,
            this, &LinuxJoystickInput::onVibrationRequested);
}

LinuxJoystickInput::~LinuxJoystickInput()
{
    if (m_udevMonitor)
        udev_monitor_unref(m_udevMonitor);
    m_udevMonitor = nullptr;

    if (m_udev)
        udev_unref(m_udev);

    m_udev = nullptr;
}

void LinuxJoystickInput::probeJoypads()
{
    if (!m_udev) {
        qCWarning(lcUniversalInput) << "Could not probe joypads, udev is not initialized";
        return;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");

    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry = nullptr;
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        udev_device *dev = udev_device_new_from_syspath(m_udev, path);
        // QString action = udev_device_get_action(dev);
        const char *devnode = udev_device_get_devnode(dev);

        if (devnode) {
            QString devnode_str = devnode;

            // check if exists
            if (std::find(m_attached_devices.begin(), m_attached_devices.end(), devnode_str) != m_attached_devices.end()) {
                udev_device_unref(dev);
                continue;
            }

            if (!devnode_str.contains(ignore_str))
                setupJoypadObject(devnode_str);
        }

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
}

static inline uint16_t BSWAP16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static inline QString _hex_str(QChar c)
{
    return QString::number(c.unicode(), 16);
}

void LinuxJoystickInput::setupJoypadObject(const QString &device)
{
    auto input = QUniversalInput::instance();
    int id = input->unusedJoyId();
    if (id == -1) {
        qCWarning(lcUniversalInput) << "Could not find unused joypad";
        return;
    }

    // GODOT begin ; dont know what is godot and what is ours now
    // tries to open the device to check if it's a joystick
    int fd = open(device.toUtf8().constData(), O_RDWR | O_NONBLOCK);
    if (fd == -1) {
        // race condition? the only one that can be opened is xbox360 controller
        return;
    }

    unsigned long evbit[NBITS(EV_MAX)] = { 0 };
    unsigned long keybit[NBITS(MAX_KEY)] = { 0 };
    unsigned long absbit[NBITS(MAX_ABS)] = { 0 };

    // add to attached devices so we don't try to open it again
    m_attached_devices.push_back(device);

    if ((ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) ||
        (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) < 0)) {
        close(fd);
        return;
    }

    // Check if the device supports basic gamepad events
    bool has_abs_left = (test_bit(ABS_X, absbit) && test_bit(ABS_Y, absbit));
    bool has_abs_right = (test_bit(ABS_RX, absbit) && test_bit(ABS_RY, absbit));
    if (!(test_bit(EV_KEY, evbit) && test_bit(EV_ABS, evbit) && (has_abs_left || has_abs_right))) {
        close(fd);
        return;
    }

    char namebuf[128];
    QString name = "";
    if (ioctl(fd, EVIOCGNAME(sizeof(namebuf)), namebuf) >= 0)
        name = namebuf;


    input_id inpid;
    if (ioctl(fd, EVIOCGID, &inpid) < 0) {
        close(fd);
        return;
    }

    // reset gamepad
    m_joypads[id] = gamepad();
    auto& joy = m_joypads[id];
    joy.fd = fd;
    joy.devpath = QString(device);
    joy.attached = true;
    joy.id = id;
    joy.vibrating = false;

    setupJoypadProperties(&joy);

    // Wake up and read events only when the device has data ready, rather than
    // polling. This keeps CPU usage at zero while idle and delivers events with
    // the lowest possible latency.
    joy.notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(joy.notifier, &QSocketNotifier::activated, this, [this, id] { processJoypad(id); });

    char uid[64];
    sprintf(uid, "%04x%04x", BSWAP16(inpid.bustype), 0);
    if (inpid.vendor && inpid.product && inpid.version) {
        uint16_t vendor = BSWAP16(inpid.vendor);
        uint16_t product = BSWAP16(inpid.product);
        uint16_t version = BSWAP16(inpid.version);

        sprintf(uid + QString(uid).length(), "%04x%04x%04x%04x%04x%04x", vendor, 0, product, 0, version, 0);
        input->updateJoyConnection(id, true, "Udev Joypad", uid);
    } else {
        QString uidname = uid;
        int uidlen = std::min((int)name.length(), 11);
        for (int i = 0; i < uidlen; i++) {
            uidname = uidname + _hex_str(name[i]);
        }
        uidname += "00";
        input->updateJoyConnection(id, true, "Udev Joypad", uidname);
    }

    // GODOT end
}

void LinuxJoystickInput::setupJoypadProperties(gamepad* joy)
{
    // GODOT begin
    unsigned long keybit[NBITS(KEY_MAX)] = { 0 };
    unsigned long absbit[NBITS(ABS_MAX)] = { 0 };

    int num_buttons = 0;
    int num_axes = 0;

    if ((ioctl(joy->fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) < 0) ||
        (ioctl(joy->fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) < 0)) {
        return;
    }
    for (int i = BTN_JOYSTICK; i < KEY_MAX; ++i)
        if (test_bit(i, keybit))
            joy->key_map[i] = num_buttons++;

    for (int i = BTN_MISC; i < BTN_JOYSTICK; ++i)
        if (test_bit(i, keybit))
            joy->key_map[i] = num_buttons++;

    for (int i = 0; i < ABS_MISC; ++i) {
        /* Skip hats */
        if (i == ABS_HAT0X) {
            i = ABS_HAT3Y;
            continue;
        }
        if (test_bit(i, absbit)) {
            joy->joy_axis[i] = num_axes++;
            joy->abs_info[i] = new input_absinfo;
            if (ioctl(joy->fd, EVIOCGABS(i), joy->abs_info[i]) < 0) {
                delete joy->abs_info[i];
                joy->abs_info[i] = nullptr;
            }
        }
    }

    joy->force_feedback = false;
    unsigned long ffbit[NBITS(FF_CNT)];
    if (ioctl(joy->fd, EVIOCGBIT(EV_FF, sizeof(ffbit)), ffbit) != -1)
        if (test_bit(FF_RUMBLE, ffbit))
            joy->force_feedback = true;


    // GODOT end
}

void LinuxJoystickInput::closeJoypads()
{
    for (int i = 0; i < JOYPADS_MAX; i++) {
        gamepad &joypad = m_joypads[i];
        closeJoypad(joypad, i);
    }
}

void LinuxJoystickInput::closeJoypad(const char *p_devpath)
{
    for (int i = 0; i < JOYPADS_MAX; i++) {
        gamepad &joypad = m_joypads[i];
        if (m_joypads[i].devpath == p_devpath)
            closeJoypad(joypad, i);
    }
}

void LinuxJoystickInput::closeJoypad(gamepad &p_joypad, int p_id)
{
    auto input = QUniversalInput::instance();

    if (p_joypad.fd != -1) {
        // closeJoypad() may be reached from within the notifier's own callback
        // (a read error while processing events), so disable it right away and
        // defer deletion until control returns to the event loop.
        if (p_joypad.notifier) {
            p_joypad.notifier->setEnabled(false);
            p_joypad.notifier->deleteLater();
            p_joypad.notifier = nullptr;
        }
        if (p_joypad.vibrationStopTimer) {
            p_joypad.vibrationStopTimer->stop();
            p_joypad.vibrationStopTimer->deleteLater();
            p_joypad.vibrationStopTimer = nullptr;
        }
        close(p_joypad.fd);
        p_joypad.fd = -1;
        m_attached_devices.erase(std::find(m_attached_devices.begin(), m_attached_devices.end(), p_joypad.devpath));
        input->updateJoyConnection(p_id, false, "");
    }
}

static inline float axisCorrect(int value, int min, int max)
{
    return 2.0f * (value - min) / (max - min) - 1.0f;
}

// Reads and dispatches all pending events for a single joypad. Driven by the
// device's QSocketNotifier, so this only runs when there is data to read.
void LinuxJoystickInput::processJoypad(int id)
{
    auto input = QUniversalInput::instance();

    gamepad& joy = m_joypads[id];
    if (!joy.attached)
        return;

    // get joypad events
    input_event event;
    std::vector<input_event> events;
    errno = 0;
    while (read(joy.fd, &event, sizeof(event)) > 0) {
        events.push_back(event);
    }

    if (errno != EAGAIN) {
        closeJoypad(joy, id);
        return;
    }

    // GODOT begin

    for (const auto& event : events) {
        // event may be tainted and out of MAX_KEY range, which will cause
        // joy.key_map[event.code] to crash
        if (event.code >= MAX_KEY) {
            continue;
        }

        switch (event.type) {
        case EV_KEY:
            input->joyButton(joy.id, (JoyButton)joy.key_map[event.code], event.value);
            break;

        case EV_ABS:
            switch (event.code) {
            case ABS_HAT0X:
                if (event.value != 0) {
                    if (event.value < 0) {
                        joy.dpad = HatFlag::Left;
                    } else {
                        joy.dpad = HatFlag::Right;
                    }
                } else {
                    joy.dpad = HatFlag::Center;
                }
                input->joyHat(joy.id, joy.dpad);
                break;

            case ABS_HAT0Y:
                if (event.value != 0) {
                    if (event.value < 0) {
                        joy.dpad = HatFlag::Up;
                    } else {
                        joy.dpad = HatFlag::Down;
                    }
                } else {
                    joy.dpad = HatFlag::Center;
                }
                input->joyHat(joy.id, joy.dpad);
                break;

            default:
                if (event.code >= MAX_ABS) {
                    continue;
                }
                if (joy.abs_info[event.code]) {
                    // using the min/max values from the device
                    auto min = joy.abs_info[event.code]->minimum;
                    auto max = joy.abs_info[event.code]->maximum;

                    float value = event.value;
                    value = axisCorrect(value, min, max);

                    // The controller database references physical axes by their
                    // SDL-style sequential index (a0, a1, ...), not by evdev ABS
                    // code, so report that index and let the mapping layer decide
                    // which logical axis it drives. Hardcoding ABS_Z/ABS_RZ to the
                    // triggers, for example, dropped the right stick on pads that
                    // place it there while exposing the triggers as buttons.
                    JoyAxis axis = JoyAxis(joy.joy_axis[event.code]);

                    input->joyAxis(joy.id, axis, value);
                }
                break;
            }
            break;
        }
    }

    // GODOT end
}

void LinuxJoystickInput::joypadVibrationStart(gamepad &p_joypad, float p_weak_magnitude, float p_strong_magnitude, float p_duration, uint64_t p_timestamp)
{
    // GODOT start

    if (!p_joypad.force_feedback || p_joypad.fd == -1 || p_weak_magnitude < 0.f || p_weak_magnitude > 1.f || p_strong_magnitude < 0.f || p_strong_magnitude > 1.f)
        return;

    if (p_joypad.ff_effect_id != -1)
        joypadVibrationStop(p_joypad, p_timestamp);

    struct ff_effect effect;
    effect.type = FF_RUMBLE;
    effect.id = -1;
    effect.u.rumble.weak_magnitude = floor(p_weak_magnitude * (float)0xffff);
    effect.u.rumble.strong_magnitude = floor(p_strong_magnitude * (float)0xffff);
    effect.replay.length = floor(p_duration);
    effect.replay.delay = 0;

    if (ioctl(p_joypad.fd, EVIOCSFF, &effect) < 0)
        return;

    struct input_event play;
    play.type = EV_FF;
    play.code = effect.id;
    play.value = 1;
    if (write(p_joypad.fd, (const void *)&play, sizeof(play)) == -1)
        qCWarning(lcUniversalInput) << "Couldn't write to Joypad device.";

    p_joypad.ff_effect_id = effect.id;
    p_joypad.vibrating = true;

    // GODOT end
}

void LinuxJoystickInput::joypadVibrationStop(gamepad &p_joypad, uint64_t p_timestamp)
{
    Q_UNUSED(p_timestamp);

    // GODOT start
    if (!p_joypad.force_feedback || p_joypad.fd == -1 || p_joypad.ff_effect_id == -1)
        return;

    if (ioctl(p_joypad.fd, EVIOCRMFF, p_joypad.ff_effect_id) < 0) {
        return;
    }

    p_joypad.ff_effect_id = -1;
    p_joypad.vibrating = false;
    // GODOT end
}

void LinuxJoystickInput::setupMonitor()
{
    if (!m_udev)
        return;

    // Watch for input devices being plugged in or removed, so we no longer have
    // to re-enumerate everything on a timer.
    m_udevMonitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_udevMonitor) {
        qCWarning(lcUniversalInput) << "Could not create udev monitor";
        return;
    }
    udev_monitor_filter_add_match_subsystem_devtype(m_udevMonitor, "input", nullptr);
    udev_monitor_enable_receiving(m_udevMonitor);

    m_monitorNotifier = new QSocketNotifier(udev_monitor_get_fd(m_udevMonitor),
                                            QSocketNotifier::Read, this);
    connect(m_monitorNotifier, &QSocketNotifier::activated,
            this, &LinuxJoystickInput::onUdevEvent);
}

void LinuxJoystickInput::onUdevEvent()
{
    udev_device *dev = udev_monitor_receive_device(m_udevMonitor);
    if (!dev)
        return;

    const char *action = udev_device_get_action(dev);
    const char *devnode = udev_device_get_devnode(dev);

    if (action && devnode) {
        const QString devnode_str = QString::fromUtf8(devnode);
        if (!devnode_str.contains(ignore_str)) {
            if (qstrcmp(action, "add") == 0) {
                if (std::find(m_attached_devices.begin(), m_attached_devices.end(), devnode_str)
                    == m_attached_devices.end())
                    setupJoypadObject(devnode_str);
            } else if (qstrcmp(action, "remove") == 0) {
                closeJoypad(devnode_str.toUtf8().constData());
            }
        }
    }

    udev_device_unref(dev);
}

void LinuxJoystickInput::onVibrationRequested(int device)
{
    if (device < 0 || device >= JOYPADS_MAX)
        return;

    auto input = QUniversalInput::instance();
    gamepad &joy = m_joypads[device];
    if (!joy.attached || !joy.force_feedback)
        return;

    const float duration = input->joyVibrationDuration(device) * 1000.f; // ms
    if (duration <= 0.f) {
        joypadVibrationStop(joy, 0);
        return;
    }

    const QVector2D strength = input->joyVibrationStrength(device);
    const uint64_t timestamp = input->joyVibrationTimestamp(device);
    joypadVibrationStart(joy, strength.x(), strength.y(), duration, timestamp);

    // Stop the effect once its duration elapses. A single-shot timer honours the
    // requested duration without any periodic polling.
    if (!joy.vibrationStopTimer) {
        joy.vibrationStopTimer = new QTimer(this);
        joy.vibrationStopTimer->setSingleShot(true);
        connect(joy.vibrationStopTimer, &QTimer::timeout, this, [this, device] {
            joypadVibrationStop(m_joypads[device], 0);
        });
    }
    joy.vibrationStopTimer->start(int(duration));
}


QT_END_NAMESPACE
