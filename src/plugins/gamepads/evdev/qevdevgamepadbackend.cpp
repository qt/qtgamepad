/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Gamepad module
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qevdevgamepadbackend_p.h"
#include <QtCore/QSocketNotifier>
#include <QtCore/QLoggingCategory>
#include <QtPlatformSupport/private/qdevicediscovery_p.h>
#include <QtCore/private/qcore_unix_p.h>
#include <linux/input.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcEGB, "qt.gamepad")

QEvdevGamepadBackend::QEvdevGamepadBackend()
    : m_nextIndex(0)
{
}

bool QEvdevGamepadBackend::start()
{
    qCDebug(lcEGB) << "start";
    QByteArray device = qgetenv("QT_GAMEPAD_DEVICE");
    if (device.isEmpty()) {
        qCDebug(lcEGB) << "Using device discovery";
        m_discovery = QDeviceDiscovery::create(QDeviceDiscovery::Device_Joystick, this);
        if (m_discovery) {
            QStringList devices = m_discovery->scanConnectedDevices();
            foreach (const QString &devStr, devices) {
                device = devStr.toUtf8();
                m_devices.append(newDevice(device));
            }
            connect(m_discovery, SIGNAL(deviceDetected(QString)), this, SLOT(handleAddedDevice(QString)));
            connect(m_discovery, SIGNAL(deviceRemoved(QString)), this, SLOT(handleRemovedDevice(QString)));
        } else {
            qWarning("No device specified, set QT_GAMEPAD_DEVICE");
            return false;
        }
    } else {
        qCDebug(lcEGB) << "Using device" << device;
        m_devices.append(newDevice(device));
    }

    return true;
}

QEvdevGamepadDevice *QEvdevGamepadBackend::newDevice(const QByteArray &device)
{
    qCDebug(lcEGB) << "Opening device" << device;
    return new QEvdevGamepadDevice(device, this);
}

// To be called only when it is sure that there is a controller on-line.
int QEvdevGamepadBackend::indexForDevice(const QByteArray &device)
{
    int index;
    if (m_devIndex.contains(device)) {
        index = m_devIndex[device];
    } else {
        index = m_nextIndex++;
        m_devIndex[device] = index;
    }
    return index;
}

void QEvdevGamepadBackend::stop()
{
    qCDebug(lcEGB) << "stop";
    qDeleteAll(m_devices);
    m_devices.clear();
}

void QEvdevGamepadBackend::handleAddedDevice(const QString &device)
{
    // This does not imply that an actual controller is connected.
    // When connecting the wireless receiver 4 devices will show up right away.
    // Therefore gamepadAdded() will be emitted only later, when something is read.
    qCDebug(lcEGB) << "Connected device" << device;
    m_devices.append(newDevice(device.toUtf8()));
}

void QEvdevGamepadBackend::handleRemovedDevice(const QString &device)
{
    qCDebug(lcEGB) << "Disconnected device" << device;
    QByteArray dev = device.toUtf8();
    for (int i = 0; i < m_devices.count(); ++i) {
        if (m_devices[i]->deviceName() == dev) {
            delete m_devices[i];
            m_devices.removeAt(i);
            break;
        }
    }
}

QEvdevGamepadDevice::QEvdevGamepadDevice(const QByteArray &dev, QEvdevGamepadBackend *backend)
    : m_dev(dev),
      m_backend(backend),
      m_index(-1),
      m_fd(-1),
      m_notifier(0),
      m_prevYHatButton(QGamepadManager::ButtonInvalid),
      m_prevXHatButton(QGamepadManager::ButtonInvalid)
{
    openDevice(dev);
}

QEvdevGamepadDevice::~QEvdevGamepadDevice()
{
    if (m_fd != -1)
        QT_CLOSE(m_fd);
    if (m_index >= 0)
        emit m_backend->gamepadRemoved(m_index);
}

bool QEvdevGamepadDevice::openDevice(const QByteArray &dev)
{
    m_fd = QT_OPEN(dev.constData(), O_RDONLY | O_NDELAY, 0);

    if (m_fd >= 0) {
        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
        connect(m_notifier, SIGNAL(activated(int)), this, SLOT(readData()));
        qCDebug(lcEGB) << "Successfully opened" << dev;
    } else {
        qErrnoWarning(errno, "Gamepad: Cannot open input device %s", qPrintable(dev));
        return false;
    }

    // Defaults. To be replaced with queried values below.
    m_axisInfo.insert(ABS_X, AxisInfo(-32768, 32767));
    m_axisInfo.insert(ABS_Y, AxisInfo(-32768, 32767));
    m_axisInfo.insert(ABS_RX, AxisInfo(-32768, 32767));
    m_axisInfo.insert(ABS_RY, AxisInfo(-32768, 32767));
    m_axisInfo.insert(ABS_Z, AxisInfo(0, 255));
    m_axisInfo.insert(ABS_RZ, AxisInfo(0, 255));

    input_absinfo absInfo;
    memset(&absInfo, 0, sizeof(input_absinfo));
    if (ioctl(m_fd, EVIOCGABS(ABS_X), &absInfo) >= 0)
        m_axisInfo.insert(ABS_X, AxisInfo(absInfo.minimum, absInfo.maximum));
    if (ioctl(m_fd, EVIOCGABS(ABS_Y), &absInfo) >= 0)
        m_axisInfo.insert(ABS_Y, AxisInfo(absInfo.minimum, absInfo.maximum));
    if (ioctl(m_fd, EVIOCGABS(ABS_RX), &absInfo) >= 0)
        m_axisInfo.insert(ABS_RX, AxisInfo(absInfo.minimum, absInfo.maximum));
    if (ioctl(m_fd, EVIOCGABS(ABS_RY), &absInfo) >= 0)
        m_axisInfo.insert(ABS_RY, AxisInfo(absInfo.minimum, absInfo.maximum));
    if (ioctl(m_fd, EVIOCGABS(ABS_Z), &absInfo) >= 0)
        m_axisInfo.insert(ABS_Z, AxisInfo(absInfo.minimum, absInfo.maximum));
    if (ioctl(m_fd, EVIOCGABS(ABS_RZ), &absInfo) >= 0)
        m_axisInfo.insert(ABS_RZ, AxisInfo(absInfo.minimum, absInfo.maximum));

    qCDebug(lcEGB) << "Axis limits:" << m_axisInfo;

    return true;
}

QDebug operator<<(QDebug dbg, const QEvdevGamepadDevice::AxisInfo &axisInfo)
{
    dbg.nospace() << "AxisInfo(min=" << axisInfo.minValue << ", max=" << axisInfo.maxValue << ")";
    return dbg.space();
}

void QEvdevGamepadDevice::readData()
{
    input_event buffer[32];
    int events = 0, n = 0;
    for (; ;) {
        events = QT_READ(m_fd, reinterpret_cast<char*>(buffer) + n, sizeof(buffer) - n);
        if (events <= 0)
            goto err;
        n += events;
        if (n % sizeof(::input_event) == 0)
            break;
    }

    n /= sizeof(::input_event);

    for (int i = 0; i < n; ++i)
        processInputEvent(&buffer[i]);

    return;

err:
    if (!events) {
        qWarning("Gamepad: Got EOF from input device");
        return;
    } else if (events < 0) {
        if (errno != EINTR && errno != EAGAIN) {
            qErrnoWarning(errno, "Gamepad: Could not read from input device");
            if (errno == ENODEV) { // device got disconnected -> stop reading
                delete m_notifier;
                m_notifier = 0;
                QT_CLOSE(m_fd);
                m_fd = -1;
            }
        }
    }
}

double QEvdevGamepadDevice::AxisInfo::normalized(int value) const
{
    if (minValue >= 0)
        return (value - minValue) / double(maxValue - minValue);
    else
        return 2 * (value - minValue) / double(maxValue - minValue) - 1.0;
}

void QEvdevGamepadDevice::processInputEvent(input_event *e)
{
    if (m_index < 0) {
        m_index = m_backend->indexForDevice(m_dev);
        qCDebug(lcEGB) << "Adding gamepad" << m_dev << "with index" << m_index;
        emit m_backend->gamepadAdded(m_index);
    }

    if (e->type == EV_KEY) {
        const bool pressed = e->value;
        QGamepadManager::GamepadButton btn = QGamepadManager::ButtonInvalid;

        switch (e->code) {
        case BTN_START:
            btn = QGamepadManager::ButtonStart;
            break;
        case BTN_SELECT:
            btn = QGamepadManager::ButtonSelect;
            break;
        case BTN_MODE:
            btn = QGamepadManager::ButtonGuide;
            break;

        case BTN_X:
            btn = QGamepadManager::ButtonX;
            break;
        case BTN_Y:
            btn = QGamepadManager::ButtonY;
            break;
        case BTN_A:
            btn = QGamepadManager::ButtonA;
            break;
        case BTN_B:
            btn = QGamepadManager::ButtonB;
            break;

        case BTN_TL:
            btn = QGamepadManager::ButtonL1;
            break;
        case BTN_TR:
            btn = QGamepadManager::ButtonR1;
            break;

        case BTN_THUMB: // wireless
        case BTN_THUMBL: // wired
            btn = QGamepadManager::ButtonL3;
            break;
        case BTN_THUMBR:
            btn = QGamepadManager::ButtonR3;
            break;

        // Directional buttons for wireless
        case BTN_TRIGGER_HAPPY1:
            btn = QGamepadManager::ButtonLeft;
            break;
        case BTN_TRIGGER_HAPPY2:
            btn = QGamepadManager::ButtonRight;
            break;
        case BTN_TRIGGER_HAPPY3:
            btn = QGamepadManager::ButtonUp;
            break;
        case BTN_TRIGGER_HAPPY4:
            btn = QGamepadManager::ButtonDown;
            break;

        default:
            break;
        }
        if (btn != QGamepadManager::ButtonInvalid) {
            if (pressed)
                emit m_backend->gamepadButtonPressed(m_index, btn, 1.0);
            else
                emit m_backend->gamepadButtonReleased(m_index, btn);
        }
    } else if (e->type == EV_ABS) {
        if (e->code == ABS_HAT0X || e->code == ABS_HAT0Y) {
            //Special logic for digital direction buttons as it is treated as a hat axis
            //with the wired controller.
            double value =  m_axisInfo.value(e->code).normalized(e->value);
            QGamepadManager::GamepadButton btn = QGamepadManager::ButtonInvalid;
            bool pressed = false;
            if (e->code == ABS_HAT0X) {
                if (value == 1.0) {
                    btn = QGamepadManager::ButtonRight;
                    m_prevXHatButton = btn;
                    pressed = true;
                } else if (value == -1.0) {
                    btn = QGamepadManager::ButtonLeft;
                    m_prevXHatButton = btn;
                    pressed = true;
                } else {
                    //Release
                    btn = m_prevXHatButton;
                    m_prevXHatButton = QGamepadManager::ButtonInvalid;
                    pressed = false;
                }
            } else {
                if (value == 1.0) {
                    btn = QGamepadManager::ButtonDown;
                    m_prevYHatButton = btn;
                    pressed = true;
                } else if (value == -1.0) {
                    btn = QGamepadManager::ButtonUp;
                    m_prevYHatButton = btn;
                    pressed = true;
                } else {
                    //Release
                    btn = m_prevYHatButton;
                    m_prevYHatButton = QGamepadManager::ButtonInvalid;
                    pressed = false;
                }
            }
            if (btn != QGamepadManager::ButtonInvalid) {
                if (pressed)
                    emit m_backend->gamepadButtonPressed(m_index, btn, 1.0);
                else
                    emit m_backend->gamepadButtonReleased(m_index, btn);
            }

        } else {
            QGamepadManager::GamepadAxis axis = QGamepadManager::AxisInvalid;
            QGamepadManager::GamepadButton btn = QGamepadManager::ButtonInvalid;

            switch (e->code) {
            case ABS_X:
                axis = QGamepadManager::AxisLeftX;
                break;
            case ABS_Y:
                axis = QGamepadManager::AxisLeftY;
                break;

            case ABS_RX:
                axis = QGamepadManager::AxisRightX;
                break;
            case ABS_RY:
                axis = QGamepadManager::AxisRightY;
                break;

            case ABS_Z:
                btn = QGamepadManager::ButtonL2;
                break;
            case ABS_RZ:
                btn = QGamepadManager::ButtonR2;
                break;

            default:
                break;
            }

            AxisInfo axisInfo = m_axisInfo.value(e->code);
            const double value = axisInfo.normalized(e->value);

            if (axis != QGamepadManager::AxisInvalid) {
                emit m_backend->gamepadAxisMoved(m_index, axis, value);
            } else if (btn != QGamepadManager::ButtonInvalid) {
                if (!qFuzzyIsNull(value))
                    emit m_backend->gamepadButtonPressed(m_index, btn, value);
                else
                    emit m_backend->gamepadButtonReleased(m_index, btn);
            }
        }
    }
}

QT_END_NAMESPACE
