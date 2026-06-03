// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "platform/windows/joypad_windows.cpp" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#include "windowsjoystickinput.h"

#include <QtUniversalInput/private/quniversalinput_p.h>

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>

#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

#include <system_error>

DWORD WINAPI _xinput_get_state(DWORD dwUserIndex, XINPUT_STATE *pState)
{
    Q_UNUSED(dwUserIndex);
    Q_UNUSED(pState);
    return ERROR_DEVICE_NOT_CONNECTED;
}

DWORD WINAPI _xinput_set_state(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
{
    Q_UNUSED(dwUserIndex);
    Q_UNUSED(pVibration);
    return ERROR_DEVICE_NOT_CONNECTED;
}

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcUniversalInput, "qt.universalinput")

// Bring the QUniversalInput input enums into scope for this file.
using HatDirection = QUniversalInput::HatDirection;
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

// Function to convert HRESULT to QString
static QString convertHRESULTToQString(HRESULT hr)
{
    char *msgBuf = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuf, 0, nullptr);
    QString msg = QString::fromLocal8Bit(msgBuf);
    LocalFree(msgBuf);
    return msg;
}

WindowsJoystickInput::WindowsJoystickInput()
{
    auto qtApp = QGuiApplication::instance();
    if (qtApp && qApp->focusWindow())
        hWnd = (HWND)qApp->focusWindow()->winId();
    // ### Maybe we should delay the initialization until we know there is an available window?

    load_xinput();

    for (int i = 0; i < JOYPADS_MAX; i++)
        attached_joypads[i] = false;

    HRESULT result = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&dinput, nullptr);
    if (result == DI_OK) {
        probeJoypads();
    } else {
        qCWarning(lcUniversalInput) << "Couldn't initialize DirectInput. Error: " << convertHRESULTToQString(result);
        if (result == DIERR_OUTOFMEMORY) {
            qCWarning(lcUniversalInput, "The Windows DirectInput subsystem could not allocate sufficient memory.");
            qCWarning(lcUniversalInput, "Rebooting your PC may solve this issue.");
        }
        // Ensure dinput is still a nullptr.
        dinput = nullptr;
    }

    // ### Replace with Thread later
    startTimer(1);
}

WindowsJoystickInput::~WindowsJoystickInput()
{
    close_joypad();
    if (dinput)
        dinput->Release();
    unload_xinput();
}

void WindowsJoystickInput::probeJoypads()
{
    if (dinput == nullptr) {
        qCWarning(lcUniversalInput, "DirectInput not initialized. Rebooting your PC may solve this issue.");
        return;
    }
    DWORD dwResult;
    auto input = QUniversalInput::instance();
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        ZeroMemory(&x_joypads[i].state, sizeof(XINPUT_STATE));

        dwResult = xinput_get_state(i, &x_joypads[i].state);
        if (dwResult == ERROR_SUCCESS) {
            int id = input->unusedJoyId();
            if (id != -1 && !x_joypads[i].attached) {
                x_joypads[i].attached = true;
                x_joypads[i].id = id;
                x_joypads[i].ff_timestamp = 0;
                x_joypads[i].ff_end_timestamp = 0;
                x_joypads[i].vibrating = false;
                attached_joypads[id] = true;
                input->updateJoyConnection(id, true, "XInput Gamepad", "__XINPUT_DEVICE__");
            }
        } else if (x_joypads[i].attached) {
            x_joypads[i].attached = false;
            attached_joypads[x_joypads[i].id] = false;
            input->updateJoyConnection(x_joypads[i].id, false, "");
        }
    }

    for (int i = 0; i < joypad_count; i++) {
        d_joypads[i].confirmed = false;
    }

    dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback, this, DIEDFL_ATTACHEDONLY);

    for (int i = 0; i < joypad_count; i++) {
        if (!d_joypads[i].confirmed) {
            close_joypad(i);
        }
    }
}

void WindowsJoystickInput::processJoypads()
{
    HRESULT hr;
    auto input = QUniversalInput::instance();
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        xinput_gamepad &joy = x_joypads[i];
        if (!joy.attached) {
            continue;
        }
        ZeroMemory(&joy.state, sizeof(XINPUT_STATE));

        xinput_get_state(i, &joy.state);
        if (joy.state.dwPacketNumber != joy.last_packet) {
            int button_mask = XINPUT_GAMEPAD_DPAD_UP;
            for (int j = 0; j <= 16; j++) {
                input->joyButton(joy.id, (JoyButton)j, joy.state.Gamepad.wButtons & button_mask);
                button_mask = button_mask * 2;
            }

            input->joyAxis(joy.id, JoyAxis::LeftX, axis_correct(joy.state.Gamepad.sThumbLX, true));
            input->joyAxis(joy.id, JoyAxis::LeftY, axis_correct(joy.state.Gamepad.sThumbLY, true, false, true));
            input->joyAxis(joy.id, JoyAxis::RightX, axis_correct(joy.state.Gamepad.sThumbRX, true));
            input->joyAxis(joy.id, JoyAxis::RightY, axis_correct(joy.state.Gamepad.sThumbRY, true, false, true));
            input->joyAxis(joy.id, JoyAxis::TriggerLeft, axis_correct(joy.state.Gamepad.bLeftTrigger, true, true));
            input->joyAxis(joy.id, JoyAxis::TriggerRight, axis_correct(joy.state.Gamepad.bRightTrigger, true, true));
            joy.last_packet = joy.state.dwPacketNumber;
        }
        uint64_t timestamp = input->joyVibrationTimestamp(joy.id);
        if (timestamp > joy.ff_timestamp) {
            QVector2D strength = input->joyVibrationStrength(joy.id);
            float duration = input->joyVibrationDuration(joy.id);
            if (strength.x() == 0 && strength.y() == 0) {
                joypad_vibration_stop_xinput(i, timestamp);
            } else {
                joypad_vibration_start_xinput(i, strength.x(), strength.y(), duration, timestamp);
            }
        } else if (joy.vibrating && joy.ff_end_timestamp != 0) {
            uint64_t currentTime = QDateTime::currentMSecsSinceEpoch();
            if (currentTime >= joy.ff_end_timestamp)
                joypad_vibration_stop_xinput(i, currentTime);
        }
    }

    for (int i = 0; i < JOYPADS_MAX; i++) {
        dinput_gamepad *joy = &d_joypads[i];

        if (!joy->attached) {
            continue;
        }

        DIJOYSTATE2 js;
        hr = joy->di_joy->Poll();
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            IDirectInputDevice8_Acquire(joy->di_joy);
            joy->di_joy->Poll();
        }

        hr = joy->di_joy->GetDeviceState(sizeof(DIJOYSTATE2), &js);
        if (FAILED(hr)) {
            continue;
        }

        post_hat(joy->id, js.rgdwPOV[0]);

        for (int j = 0; j < 128; j++) {
            if (js.rgbButtons[j] & 0x80) {
                if (!joy->last_buttons[j]) {
                    input->joyButton(joy->id, (JoyButton)j, true);
                    joy->last_buttons[j] = true;
                }
            } else {
                if (joy->last_buttons[j]) {
                    input->joyButton(joy->id, (JoyButton)j, false);
                    joy->last_buttons[j] = false;
                }
            }
        }

        // on mingw, these constants are not constants
        int count = 8;
        const LONG axes[] = { DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, (LONG)DIJOFS_SLIDER(0), (LONG)DIJOFS_SLIDER(1) };
        int values[] = { js.lX, js.lY, js.lZ, js.lRx, js.lRy, js.lRz, js.rglSlider[0], js.rglSlider[1] };

        for (int j = 0; j < joy->joy_axis.size(); j++) {
            for (int k = 0; k < count; k++) {
                if (joy->joy_axis[j] == axes[k]) {
                    input->joyAxis(joy->id, (JoyAxis)j, axis_correct(values[k]));
                    break;
                }
            }
        }
    }
}

void WindowsJoystickInput::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    processJoypads();
}

bool WindowsJoystickInput::have_device(const GUID &p_guid)
{
    for (int i = 0; i < JOYPADS_MAX; i++) {
        if (d_joypads[i].guid == p_guid) {
            d_joypads[i].confirmed = true;
            return true;
        }
    }
    return false;
}

bool WindowsJoystickInput::is_xinput_device(const GUID *p_guid)
{
    static GUID IID_ValveStreamingGamepad = { MAKELONG(0x28DE, 0x11FF), 0x28DE, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } };
    static GUID IID_X360WiredGamepad = { MAKELONG(0x045E, 0x02A1), 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } };
    static GUID IID_X360WirelessGamepad = { MAKELONG(0x045E, 0x028E), 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } };

    if (::memcmp(p_guid, &IID_ValveStreamingGamepad, sizeof(*p_guid)) == 0 ||
            ::memcmp(p_guid, &IID_X360WiredGamepad, sizeof(*p_guid)) == 0 ||
            ::memcmp(p_guid, &IID_X360WirelessGamepad, sizeof(*p_guid)) == 0)
        return true;

    PRAWINPUTDEVICELIST dev_list = nullptr;
    unsigned int dev_list_count = 0;

    if (GetRawInputDeviceList(nullptr, &dev_list_count, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1)
        return false;

    dev_list = (PRAWINPUTDEVICELIST)::malloc(sizeof(RAWINPUTDEVICELIST) * dev_list_count);
    if (dev_list == nullptr) {
        qCWarning(lcUniversalInput, "Out of memory.");
        return false;
    }

    if (GetRawInputDeviceList(dev_list, &dev_list_count, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) {
        ::free(dev_list);
        return false;
    }
    for (unsigned int i = 0; i < dev_list_count; i++) {
        RID_DEVICE_INFO rdi;
        char dev_name[128];
        UINT rdiSize = sizeof(rdi);
        UINT nameSize = sizeof(dev_name);

        rdi.cbSize = rdiSize;
        if ((dev_list[i].dwType == RIM_TYPEHID) &&
                (GetRawInputDeviceInfoA(dev_list[i].hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) != (UINT)-1) &&
                (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) == (LONG)p_guid->Data1) &&
                (GetRawInputDeviceInfoA(dev_list[i].hDevice, RIDI_DEVICENAME, &dev_name, &nameSize) != (UINT)-1) &&
                (strstr(dev_name, "IG_") != nullptr)) {
            ::free(dev_list);
            return true;
        }
    }
    ::free(dev_list);
    return false;
}

static inline uint16_t BSWAP16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

bool WindowsJoystickInput::setup_dinput_joypad(const DIDEVICEINSTANCE *instance)
{
    if (dinput == nullptr) {
        qCWarning(lcUniversalInput, "DirectInput not initialized. Rebooting your PC may solve this issue.");
        return false;
    }

    HRESULT hr;
    auto input = QUniversalInput::instance();
    int num = input->unusedJoyId();

    if (have_device(instance->guidInstance) || num == -1)
        return false;

    d_joypads[num] = dinput_gamepad();
    dinput_gamepad *joy = &d_joypads[num];

    const DWORD devtype = (instance->dwDevType & 0xFF);

    if ((devtype != DI8DEVTYPE_JOYSTICK) && (devtype != DI8DEVTYPE_GAMEPAD) && (devtype != DI8DEVTYPE_1STPERSON) && (devtype != DI8DEVTYPE_DRIVING))
        return false;

    hr = dinput->CreateDevice(instance->guidInstance, &joy->di_joy, nullptr);

    if (FAILED(hr))
        return false;

    const GUID &guid = instance->guidProduct;
    char uid[128];

    if (memcmp(&guid.Data4[2], "PIDVID", 6) != 0) {
        qCWarning(lcUniversalInput, "DirectInput device not recognized.");
        return false;
    }

    WORD type = BSWAP16(0x03);
    WORD vendor = BSWAP16(LOWORD(guid.Data1));
    WORD product = BSWAP16(HIWORD(guid.Data1));
    WORD version = 0;
    sprintf_s(uid, "%04x%04x%04x%04x%04x%04x%04x%04x", type, 0, vendor, 0, product, 0, version, 0);

    id_to_change = num;
    slider_count = 0;

    joy->di_joy->SetDataFormat(&c_dfDIJoystick2);
    joy->di_joy->SetCooperativeLevel(hWnd, DISCL_FOREGROUND);
    joy->di_joy->EnumObjects(objectsCallback, this, 0);
    std::sort(joy->joy_axis.begin(), joy->joy_axis.end());

    joy->guid = instance->guidInstance;
    input->updateJoyConnection(num, true, QString::fromWCharArray(instance->tszProductName), QString::fromLocal8Bit(uid));
    joy->attached = true;
    joy->id = num;
    attached_joypads[num] = true;
    joy->confirmed = true;
    joypad_count++;
    return true;
}

void WindowsJoystickInput::setup_joypad_object(const DIDEVICEOBJECTINSTANCE *ob, int p_joy_id)
{
    if (ob->dwType & DIDFT_AXIS) {
        HRESULT res;
        DIPROPRANGE prop_range;
        DIPROPDWORD dilong;
        LONG ofs;
        if (ob->guidType == GUID_XAxis) {
            ofs = DIJOFS_X;
        } else if (ob->guidType == GUID_YAxis) {
            ofs = DIJOFS_Y;
        } else if (ob->guidType == GUID_ZAxis) {
            ofs = DIJOFS_Z;
        } else if (ob->guidType == GUID_RxAxis) {
            ofs = DIJOFS_RX;
        } else if (ob->guidType == GUID_RyAxis) {
            ofs = DIJOFS_RY;
        } else if (ob->guidType == GUID_RzAxis) {
            ofs = DIJOFS_RZ;
        } else if (ob->guidType == GUID_Slider) {
            if (slider_count < 2) {
                ofs = DIJOFS_SLIDER(slider_count);
                slider_count++;
            } else {
                return;
            }
        } else {
            return;
        }
        prop_range.diph.dwSize = sizeof(DIPROPRANGE);
        prop_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        prop_range.diph.dwObj = ob->dwType;
        prop_range.diph.dwHow = DIPH_BYID;
        prop_range.lMin = -MAX_JOY_AXIS;
        prop_range.lMax = +MAX_JOY_AXIS;

        dinput_gamepad &joy = d_joypads[p_joy_id];

        res = IDirectInputDevice8_SetProperty(joy.di_joy, DIPROP_RANGE, &prop_range.diph);
        if (FAILED(res)) {
            return;
        }

        dilong.diph.dwSize = sizeof(dilong);
        dilong.diph.dwHeaderSize = sizeof(dilong.diph);
        dilong.diph.dwObj = ob->dwType;
        dilong.diph.dwHow = DIPH_BYID;
        dilong.dwData = 0;

        res = IDirectInputDevice8_SetProperty(joy.di_joy, DIPROP_DEADZONE, &dilong.diph);
        if (FAILED(res)) {
            return;
        }

        joy.joy_axis.push_back(ofs);
    }
}

BOOL CALLBACK WindowsJoystickInput::enumCallback(const DIDEVICEINSTANCE *p_instance, void *p_context)
{
    WindowsJoystickInput *self = static_cast<WindowsJoystickInput *>(p_context);
    if (self->is_xinput_device(&p_instance->guidProduct)) {
        return DIENUM_CONTINUE;
    }
    self->setup_dinput_joypad(p_instance);
    return DIENUM_CONTINUE;
}

BOOL CALLBACK WindowsJoystickInput::objectsCallback(const DIDEVICEOBJECTINSTANCE *p_instance, void *p_context)
{
    WindowsJoystickInput *self = static_cast<WindowsJoystickInput *>(p_context);
    self->setup_joypad_object(p_instance, self->id_to_change);

    return DIENUM_CONTINUE;
}

void WindowsJoystickInput::close_joypad(int id)
{
    if (id == -1) {
        for (int i = 0; i < JOYPADS_MAX; i++) {
            close_joypad(i);
        }
        return;
    }

    if (!d_joypads[id].attached) {
        return;
    }

    d_joypads[id].di_joy->Unacquire();
    d_joypads[id].di_joy->Release();
    d_joypads[id].attached = false;
    attached_joypads[d_joypads[id].id] = false;
    d_joypads[id].guid.Data1 = d_joypads[id].guid.Data2 = d_joypads[id].guid.Data3 = 0;
    QUniversalInput::instance()->updateJoyConnection(d_joypads[id].id, false, "");
    joypad_count--;
}

void WindowsJoystickInput::post_hat(int p_device, DWORD p_dpad)
{
    HatMask dpad_val = (HatMask)0;

    // Should be -1 when centered, but according to docs:
    // "Some drivers report the centered position of the POV indicator as 65,535. Determine whether the indicator is centered as follows:
    //  BOOL POVCentered = (LOWORD(dwPOV) == 0xFFFF);"
    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416628(v%3Dvs.85)#remarks
    if (LOWORD(p_dpad) == 0xFFFF)
        dpad_val = (HatMask)HatMask::Center;

    if (p_dpad == 0) {
        dpad_val = (HatMask)HatMask::Up;

    } else if (p_dpad == 4500) {
        dpad_val = (HatMask)(HatMask::Up | HatMask::Right);

    } else if (p_dpad == 9000) {
        dpad_val = (HatMask)HatMask::Right;

    } else if (p_dpad == 13500) {
        dpad_val = (HatMask)(HatMask::Right | HatMask::Down);

    } else if (p_dpad == 18000) {
        dpad_val = (HatMask)HatMask::Down;

    } else if (p_dpad == 22500) {
        dpad_val = (HatMask)(HatMask::Down | HatMask::Left);

    } else if (p_dpad == 27000) {
        dpad_val = (HatMask)HatMask::Left;

    } else if (p_dpad == 31500) {
        dpad_val = (HatMask)(HatMask::Left | HatMask::Up);
    }
    QUniversalInput::instance()->joyHat(p_device, dpad_val);
}

float WindowsJoystickInput::axis_correct(int p_val, bool p_xinput, bool p_trigger, bool p_negate) const
{
    if (qFabs(p_val) < MIN_JOY_AXIS)
        return p_trigger ? -1.0f : 0.0f;

    if (!p_xinput)
        return (float)p_val / MAX_JOY_AXIS;

    // Convert to a value between -1.0f and 1.0f.
    if (p_trigger)
        return 2.0f * p_val / MAX_TRIGGER - 1.0f;

    float value;
    if (p_val < 0)
        value = (float)p_val / MAX_JOY_AXIS;
    else
        value = (float)p_val / (MAX_JOY_AXIS - 1);

    if (p_negate)
        value = -value;

    return value;
}

void WindowsJoystickInput::joypad_vibration_start_xinput(int p_device, float p_weak_magnitude, float p_strong_magnitude, float p_duration, uint64_t p_timestamp)
{
    xinput_gamepad &joy = x_joypads[p_device];
    if (joy.attached) {
        XINPUT_VIBRATION effect;
        effect.wLeftMotorSpeed = (65535 * p_strong_magnitude);
        effect.wRightMotorSpeed = (65535 * p_weak_magnitude);
        if (xinput_set_state(p_device, &effect) == ERROR_SUCCESS) {
            joy.ff_timestamp = p_timestamp;
            joy.ff_end_timestamp = p_duration == 0 ? 0 : p_timestamp + (uint64_t)(p_duration * 1000);
            joy.vibrating = true;
        }
    }
}

void WindowsJoystickInput::joypad_vibration_stop_xinput(int p_device, uint64_t p_timestamp)
{
    xinput_gamepad &joy = x_joypads[p_device];
    if (joy.attached) {
        XINPUT_VIBRATION effect;
        effect.wLeftMotorSpeed = 0;
        effect.wRightMotorSpeed = 0;
        if (xinput_set_state(p_device, &effect) == ERROR_SUCCESS) {
            joy.ff_timestamp = p_timestamp;
            joy.vibrating = false;
        }
    }
}

void WindowsJoystickInput::load_xinput()
{
    xinput_get_state = &_xinput_get_state;
    xinput_set_state = &_xinput_set_state;
    xinput_dll.setFileName("xinput1_4.dll");
    if (!xinput_dll.load()) {
        xinput_dll.setFileName("xinput1_3.dll");
        if (!xinput_dll.load()) {
            xinput_dll.setFileName("xinput9_1_0.dll");
            xinput_dll.load();
        }
    }

    if (!xinput_dll.isLoaded()) {
        qCWarning(lcUniversalInput, "Could not find XInput, using DirectInput only");
        return;
    }

    XInputGetState_t func = (XInputGetState_t)xinput_dll.resolve("XInputGetState");
    XInputSetState_t set_func = (XInputSetState_t)xinput_dll.resolve("XInputSetState");

    if (!func || !set_func) {
        unload_xinput();
        return;
    }
    xinput_get_state = func;
    xinput_set_state = set_func;
}

void WindowsJoystickInput::unload_xinput()
{
    if (xinput_dll.isLoaded())
        xinput_dll.unload();
}


QT_END_NAMESPACE
