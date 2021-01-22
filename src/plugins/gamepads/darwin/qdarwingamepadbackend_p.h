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

#ifndef QDARWINGAMEPADBACKEND_P_H
#define QDARWINGAMEPADBACKEND_P_H

#include <QtCore/QTimer>
#include <QtCore/QMap>

#include <QtGamepad/QGamepadManager>
#include <QtGamepad/private/qgamepadbackend_p.h>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(DarwinGamepadManager));

QT_BEGIN_NAMESPACE

class QDarwinGamepadBackend : public QGamepadBackend
{
    Q_OBJECT
public:
    explicit QDarwinGamepadBackend(QObject *parent = nullptr);
    ~QDarwinGamepadBackend();

protected:
    bool start() override;
    void stop() override;

public Q_SLOTS:
    void darwinGamepadAdded(int index);
    void darwinGamepadRemoved(int index);
    void darwinGamepadAxisMoved(int index, QGamepadManager::GamepadAxis axis, double value);
    void darwinGamepadButtonPressed(int index, QGamepadManager::GamepadButton button, double value);
    void darwinGamepadButtonReleased(int index, QGamepadManager::GamepadButton button);
    void handlePauseButton(int index);

private:
    QT_MANGLE_NAMESPACE(DarwinGamepadManager) *m_darwinGamepadManager;
    bool m_isMonitoringActive;
    QMap<int, bool> m_pauseButtonMap;
};

QT_END_NAMESPACE

#endif // QDARWINGAMEPADBACKEND_P_H
