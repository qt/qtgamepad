// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "platform/android/java_godot_lib_jni.cpp" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#include "androidjoystickinput.h"
#include <QtCore/qnativeinterface.h>
#include <QtCore/QLoggingCategory>
#include <QtUniversalInput/QUniversalInput>

#include <QtUniversalInput/private/qtuniversalinputglobal_p.h>

// Bring the QUniversalInput input enums into scope for this file.
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

using namespace QtJniTypes;

Q_DECLARE_JNI_CLASS(KeyEvent, "android/view/KeyEvent")
Q_DECLARE_JNI_CLASS(MotionEvent, "android/view/MotionEvent")

static void joyConnectionChanged(JNIEnv *, jclass, int deviceId, bool connected, jstring name)
{
    QUniversalInput::instance()->updateJoyConnection(deviceId, connected, QJniObject(name).toString());
}
Q_DECLARE_JNI_NATIVE_METHOD(joyConnectionChanged)

static void joyButton(JNIEnv *, jclass, int deviceId, int button, bool pressed)
{
    QUniversalInput::instance()->joyButton(deviceId, JoyButton(button), pressed);
}
Q_DECLARE_JNI_NATIVE_METHOD(joyButton)

static void joyAxis(JNIEnv *, jclass, int deviceId, int axis, float value)
{
    QUniversalInput::instance()->joyAxis(deviceId, JoyAxis(axis), value);
}
Q_DECLARE_JNI_NATIVE_METHOD(joyAxis)

static void joyHat(JNIEnv *, jclass, int deviceId, int hatX, int hatY)
{
    HatMask hat = HatMask::Center;
    if (hatX != 0) {
        if (hatX < 0)
            hat |= HatMask::Left;
        else
            hat |= HatMask::Right;
    }
    if (hatY != 0) {
        if (hatY < 0)
            hat |= HatMask::Up;
        else
            hat |= HatMask::Down;
    }

    QUniversalInput::instance()->joyHat(deviceId, hat);
}
Q_DECLARE_JNI_NATIVE_METHOD(joyHat)

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcUniversalInput, "qt.universalinput")

const char keyEventClass[] = "android/view/KeyEvent";
inline int keyField(const char *field)
{
    return QJniObject::getStaticField<jint>(keyEventClass, field);
}


static void initJNI()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    if (!QtJoystickInputHandler::registerNativeMethods({
                                                        Q_JNI_NATIVE_METHOD(joyConnectionChanged),
                                                        Q_JNI_NATIVE_METHOD(joyButton),
                                                        Q_JNI_NATIVE_METHOD(joyAxis),
                                                        Q_JNI_NATIVE_METHOD(joyHat)}))
        qCCritical(lcUniversalInput, "Failed to register native methods for QtJoystickInputHandler");
}

AndroidJoystickInput::AndroidJoystickInput()
{
    initJNI();
    m_qtJoystickInputHandler = QtJoystickInputHandler(QtAndroidPrivate::activity());
    QtAndroidPrivate::registerGenericMotionEventListener(this);
    QtAndroidPrivate::registerKeyEventListener(this);
    start();
}

AndroidJoystickInput::~AndroidJoystickInput()
{
    stop();
    QtAndroidPrivate::unregisterGenericMotionEventListener(this);
    QtAndroidPrivate::unregisterKeyEventListener(this);
}

bool AndroidJoystickInput::handleKeyEvent(jobject event)
{
    static int ACTION_DOWN = keyField("ACTION_DOWN");
    static int ACTION_UP = keyField("ACTION_UP");
    QJniObject ev(event);

    // Pass the event to the QtJoystickInputHandler Java class.
    if (m_qtJoystickInputHandler.isValid()) {
        const int keyCode = ev.callMethod<jint>("getKeyCode", "()I");
        const int action = ev.callMethod<jint>("getAction", "()I");

        if (action == ACTION_UP)
            return m_qtJoystickInputHandler.callMethod<bool>("onKeyUp", keyCode, KeyEvent(event));
        else if (action == ACTION_DOWN)
            return m_qtJoystickInputHandler.callMethod<bool>("onKeyDown", keyCode, KeyEvent(event));
    }

    return false;
}

bool AndroidJoystickInput::handleGenericMotionEvent(jobject event)
{
    if (m_qtJoystickInputHandler.isValid())
        return m_qtJoystickInputHandler.callMethod<bool>("onGenericMotionEvent", MotionEvent(event));

    return false;
}

void AndroidJoystickInput::start()
{
    if (QtAndroidPrivate::androidSdkVersion() >= 16)
        m_qtJoystickInputHandler.callMethod<void>("register", jlong(this));
}

void AndroidJoystickInput::stop()
{
    if (QtAndroidPrivate::androidSdkVersion() >= 16)
        m_qtJoystickInputHandler.callMethod<void>("unregister");
}

QT_END_NAMESPACE
