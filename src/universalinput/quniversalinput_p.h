// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QUNIVERSALINPUT_P_H
#define QUNIVERSALINPUT_P_H

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

#include <QtUniversalInput/private/qtuniversalinputglobal_p.h>

#include <QtUniversalInput/quniversalinput.h>


#include <QtCore/private/qobject_p.h>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtGui/QVector3D>
#include <QtGui/QVector2D>
#include <QtCore/QHash>
#include <QtCore/QRecursiveMutex>


QT_BEGIN_NAMESPACE
class QJoystickInput;
class QMouseInput;
class QUniversalInputPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QUniversalInput)

public:
    QUniversalInputPrivate();
    ~QUniversalInputPrivate();

    // private slots
    void _q_init();

    QJoystickInput *joystickInput = nullptr;
    QMouseInput *mouseInput = nullptr;


    QSet<Qt::Key> keysPressed;
    QSet<QUniversalInput::JoyButton> joystickButtonsPressed;
    QMap<QUniversalInput::JoyAxis, qreal> joystickAxes;

    QVector3D gravity;
    QVector3D acceleration;
    QVector3D magnetometer;
    QVector3D gyroscope;

    QHash<QString, QUniversalInput::Action> actionState;

    bool useInputBuffering = false;
    bool useAccumulatedInput = true;

    QHash<int, QUniversalInput::VibrationInfo> joystickVibrations;

    QUniversalInput::VelocityTrack mouseVelocityTrack;
    QHash<int, QUniversalInput::VelocityTrack> touchVelocityTrack;
    QHash<int, QUniversalInput::Joypad> joypadNames;
    int fallbackMapping = -1;

    QVector<QUniversalInput::JoyDeviceMapping> mappingDatabase;

    QRecursiveMutex mutex;

    // mouse disable
    bool mouseDisabled = false;
    // mouse disable

private:
    void loadMappingDatabase();
};

QT_END_NAMESPACE

#endif // QUNIVERSALINPUT_P_H
