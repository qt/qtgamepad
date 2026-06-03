// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDJOYSTICKINPUT_H
#define ANDROIDJOYSTICKINPUT_H

#include <QtUniversalInput/private/qjoystickinput_p.h>
#include <QtCore/QMutex>
#include <QtCore/QJniObject>
#include <QtCore/qjnitypes.h>
#include <QtCore/private/qjnihelpers_p.h>

Q_DECLARE_JNI_CLASS(QtJoystickInputHandler, "org/qtproject/qt/android/universalinput/QtJoystickInputHandler");

QT_BEGIN_NAMESPACE

class AndroidJoystickInput : public QJoystickInput, public QtAndroidPrivate::GenericMotionEventListener, public QtAndroidPrivate::KeyEventListener
{
    Q_OBJECT
public:
    AndroidJoystickInput();
    ~AndroidJoystickInput();

    // KeyEventListener interface
    bool handleKeyEvent(jobject event) override;

    // GenericMotionEventListener interface
    bool handleGenericMotionEvent(jobject event) override;

private:
    void start();
    void stop();

    QtJniTypes::QtJoystickInputHandler m_qtJoystickInputHandler;
};

QT_END_NAMESPACE

#endif // ANDROIDJOYSTICKINPUT_H
