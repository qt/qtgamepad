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

#ifndef QEVDEVGAMEPADCONTROLLER_H
#define QEVDEVGAMEPADCONTROLLER_H

#include <QtGamepad/QGamepadManager>
#include <QtGamepad/private/qgamepadbackend_p.h>
#include <QtCore/QHash>
#include <QtCore/QVector>

struct input_event;

QT_BEGIN_NAMESPACE

class QSocketNotifier;
class QDeviceDiscovery;
class QEvdevGamepadBackend;

class QEvdevGamepadDevice : public QObject
{
    Q_OBJECT

public:
    QEvdevGamepadDevice(const QByteArray &dev, QEvdevGamepadBackend *backend);
    ~QEvdevGamepadDevice();
    QByteArray deviceName() const { return m_dev; }

private slots:
    void readData();

private:
    void processInputEvent(input_event *e);
    bool openDevice(const QByteArray &dev);

    QByteArray m_dev;
    QEvdevGamepadBackend *m_backend;
    int m_index;
    int m_fd;
    QSocketNotifier *m_notifier;
    struct AxisInfo {
        AxisInfo() : minValue(0), maxValue(1), flat(0) { }
        AxisInfo(int minValue, int maxValue, int flat) : minValue(minValue), maxValue(maxValue), flat(fabs((maxValue-minValue) ? flat/double(maxValue-minValue) : 0)) { }
        double normalized(int value) const;

        int minValue;
        int maxValue;
        double flat;
    };
    QHash<int, AxisInfo> m_axisInfo;
    QGamepadManager::GamepadButton m_prevYHatButton;
    QGamepadManager::GamepadButton m_prevXHatButton;
    friend QDebug operator<<(QDebug dbg, const QEvdevGamepadDevice::AxisInfo &axisInfo);
};

QDebug operator<<(QDebug dbg, const QEvdevGamepadDevice::AxisInfo &axisInfo);

class QEvdevGamepadBackend : public QGamepadBackend
{
    Q_OBJECT

public:
    QEvdevGamepadBackend();
    bool start() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    int idForDevice(const QByteArray &device);

private slots:
    void handleAddedDevice(const QString &device);
    void handleRemovedDevice(const QString &device);

private:
    QEvdevGamepadDevice *newDevice(const QByteArray &device);

    QDeviceDiscovery *m_discovery;
    QVector<QEvdevGamepadDevice *> m_devices;
    QHash<QByteArray, int> m_devIndex;
    int m_nextIndex;
};

QT_END_NAMESPACE

#endif // QEVDEVGAMEPADCONTROLLER_H
