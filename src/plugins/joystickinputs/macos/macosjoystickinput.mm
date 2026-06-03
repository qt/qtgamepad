// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "platform/macos/joypad_macos.cpp" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#include "macosjoystickinput.h"
#include <QtUniversalInput/QUniversalInput>
#include <QtEndian>

QT_BEGIN_NAMESPACE

// Bring the QUniversalInput input enums into scope for this file.
using HatDirection = QUniversalInput::HatDirection;
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

#define JOYPAD_LOOP_RUN_MODE CFSTR("QtUniversalInput")

static MacOsJoystickInput *self = nullptr;

Joypad::Joypad()
{
    forceFeedbackConstantForce.lMagnitude = 10000;
    forceFeedbackEffect.dwDuration = 0;
    forceFeedbackEffect.dwSamplePeriod = 0;
    forceFeedbackEffect.dwGain = 10000;
    forceFeedbackEffect.dwFlags = FFEFF_OBJECTOFFSETS;
    forceFeedbackEffect.dwTriggerButton = FFEB_NOTRIGGER;
    forceFeedbackEffect.dwStartDelay = 0;
    forceFeedbackEffect.dwTriggerRepeatInterval = 0;
    forceFeedbackEffect.lpEnvelope = nullptr;
    forceFeedbackEffect.cbTypeSpecificParams = sizeof(FFCONSTANTFORCE);
    forceFeedbackEffect.lpvTypeSpecificParams = &forceFeedbackConstantForce;
    forceFeedbackEffect.dwSize = sizeof(forceFeedbackEffect);
}

void Joypad::free()
{
    if (deviceRef)
        IOHIDDeviceUnscheduleFromRunLoop(deviceRef, CFRunLoopGetCurrent(), JOYPAD_LOOP_RUN_MODE);

    if (forceFeedbackDevice) {
        FFDeviceReleaseEffect(forceFeedbackDevice, forcefeedbackObject);
        FFReleaseDevice(forceFeedbackDevice);
        forceFeedbackDevice = nullptr;
        ::free(forceFeedbackAxes);
        ::free(forceFeedbackDirections);
    }
}

bool Joypad::has_element(IOHIDElementCookie p_cookie, QVector<RecElement> *p_list) const
{
    for (int i = 0; i < p_list->size(); i++)
        if (p_cookie == p_list->at(i).cookie)
            return true;

    return false;
}

int Joypad::get_hid_element_state(RecElement *p_element) const
{
    int value = 0;
    if (p_element && p_element->ref) {
        IOHIDValueRef valueRef;
        if (IOHIDDeviceGetValue(deviceRef, p_element->ref, &valueRef) == kIOReturnSuccess) {
            value = SInt32(IOHIDValueGetIntegerValue(valueRef));

            // Record min and max for auto calibration.
            if (value < p_element->min)
                p_element->min = value;

            if (value > p_element->max)
                p_element->max = value;
        }
    }
    return value;
}

void Joypad::add_hid_element(IOHIDElementRef p_element)
{
    const CFTypeID elementTypeID = p_element ? CFGetTypeID(p_element) : 0;

    if (p_element && (elementTypeID == IOHIDElementGetTypeID())) {
        const IOHIDElementCookie cookie = IOHIDElementGetCookie(p_element);
        const uint32_t usagePage = IOHIDElementGetUsagePage(p_element);
        const uint32_t usage = IOHIDElementGetUsage(p_element);
        QVector<RecElement> *list = nullptr;

        switch (IOHIDElementGetType(p_element)) {
            case kIOHIDElementTypeInput_Misc:
            case kIOHIDElementTypeInput_Button:
            case kIOHIDElementTypeInput_Axis: {
                switch (usagePage) {
                    case kHIDPage_GenericDesktop:
                        switch (usage) {
                            case kHIDUsage_GD_X:
                            case kHIDUsage_GD_Y:
                            case kHIDUsage_GD_Z:
                            case kHIDUsage_GD_Rx:
                            case kHIDUsage_GD_Ry:
                            case kHIDUsage_GD_Rz:
                            case kHIDUsage_GD_Slider:
                            case kHIDUsage_GD_Dial:
                            case kHIDUsage_GD_Wheel:
                                if (!has_element(cookie, &axisElements))
                                    list = &axisElements;
                                break;

                            case kHIDUsage_GD_Hatswitch:
                                if (!has_element(cookie, &hatElements))
                                    list = &hatElements;
                                break;
                            case kHIDUsage_GD_DPadUp:
                            case kHIDUsage_GD_DPadDown:
                            case kHIDUsage_GD_DPadRight:
                            case kHIDUsage_GD_DPadLeft:
                            case kHIDUsage_GD_Start:
                            case kHIDUsage_GD_Select:
                                if (!has_element(cookie, &buttonElements))
                                    list = &buttonElements;
                                break;
                        }
                        break;

                    case kHIDPage_Simulation:
                        switch (usage) {
                            case kHIDUsage_Sim_Rudder:
                            case kHIDUsage_Sim_Throttle:
                            case kHIDUsage_Sim_Accelerator:
                            case kHIDUsage_Sim_Brake:
                                if (!has_element(cookie, &axisElements))
                                    list = &axisElements;
                                break;

                            default:
                                break;
                        }
                        break;

                    case kHIDPage_Button:
                    case kHIDPage_Consumer:
                        if (!has_element(cookie, &buttonElements))
                            list = &buttonElements;
                        break;

                    default:
                        break;
                }
            } break;

            case kIOHIDElementTypeCollection: {
                CFArrayRef array = IOHIDElementGetChildren(p_element);
                if (array)
                    add_hid_elements(array);
            } break;

            default:
                break;
        }

        if (list) { // Add to list.
            RecElement element;

            element.ref = p_element;
            element.usage = usage;

            element.min = (SInt32)IOHIDElementGetLogicalMin(p_element);
            element.max = (SInt32)IOHIDElementGetLogicalMax(p_element);
            element.cookie = IOHIDElementGetCookie(p_element);
            list->push_back(element);
            std::sort(list->begin(), list->end(), RecElement::Comparator());
            // list->sort_custom<rec_element::Comparator>();
        }
    }
}

static void hid_element_added(const void *p_value, void *p_parameter)
{
    Joypad *joy = static_cast<Joypad *>(p_parameter);
    joy->add_hid_element(IOHIDElementRef(p_value));
}

void Joypad::add_hid_elements(CFArrayRef p_array)
{
    CFRange range = { 0, CFArrayGetCount(p_array) };
    CFArrayApplyFunction(p_array, range, hid_element_added, this);
}

static void joypad_removed_callback(void *ctx, IOReturn res, void *sender, IOHIDDeviceRef ioHIDDeviceObject)
{
    Q_UNUSED(ctx);
    Q_UNUSED(sender);
    self->deviceRemoved(res, ioHIDDeviceObject);
}

static void joypad_added_callback(void *ctx, IOReturn res, void *sender, IOHIDDeviceRef ioHIDDeviceObject)
{
    Q_UNUSED(ctx);
    Q_UNUSED(sender);
    self->deviceAdded(res, ioHIDDeviceObject);
}

static bool is_joypad(IOHIDDeviceRef p_device_ref)
{
    int usage_page = 0;
    int usage = 0;
    CFTypeRef refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDPrimaryUsagePageKey));
    if (refCF)
        CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &usage_page);

    if (usage_page != kHIDPage_GenericDesktop)
        return false;

    refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDPrimaryUsageKey));
    if (refCF)
        CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &usage);

    if ((usage != kHIDUsage_GD_Joystick && usage != kHIDUsage_GD_GamePad && usage != kHIDUsage_GD_MultiAxisController))
        return false;

    return true;
}

void MacOsJoystickInput::deviceAdded(IOReturn p_res, IOHIDDeviceRef p_device)
{
    if (p_res != kIOReturnSuccess || have_device(p_device))
        return;

    Joypad new_joypad;
    if (is_joypad(p_device)) {
        configure_joypad(p_device, &new_joypad);
        const io_service_t ioservice = IOHIDDeviceGetService(p_device);
        if ((ioservice) && (FFIsForceFeedback(ioservice) == FF_OK) && new_joypad.config_force_feedback(ioservice))
            new_joypad.forceFeedbackService = ioservice;
        m_deviceList.push_back(new_joypad);
    }
    IOHIDDeviceScheduleWithRunLoop(p_device, CFRunLoopGetCurrent(), JOYPAD_LOOP_RUN_MODE);
}

void MacOsJoystickInput::deviceRemoved(IOReturn p_res, IOHIDDeviceRef p_device)
{
    Q_UNUSED(p_res);
    int device = get_joy_ref(p_device);
    Q_ASSERT(device != -1);

    auto input = QUniversalInput::instance();

    input->updateJoyConnection(m_deviceList[device].id, false, "");
    m_deviceList[device].free();
    m_deviceList.removeAt(device);
}

static QByteArray _hex_str(uint8_t p_byte)
{
    static const char *dict = "0123456789abcdef";
    char ret[3];
    ret[2] = 0;

    ret[0] = dict[p_byte >> 4];
    ret[1] = dict[p_byte & 0xF];

    return ret;
}

bool MacOsJoystickInput::configure_joypad(IOHIDDeviceRef p_device_ref, Joypad *p_joy)
{
    p_joy->deviceRef = p_device_ref;
    // Get device name.
    QByteArray name;
    char c_name[256];
    CFTypeRef refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDProductKey));
    if (!refCF)
        refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDManufacturerKey));

    if ((!refCF) || (!CFStringGetCString((CFStringRef)refCF, c_name, sizeof(c_name), kCFStringEncodingUTF8)))
        name = "Unidentified Joypad";
    else
        name = c_name;

    auto input = QUniversalInput::instance();

    int id = input->unusedJoyId();
    if (id == -1)
        return false;

    p_joy->id = id;
    int vendor = 0;
    refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDVendorIDKey));
    if (refCF)
        CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &vendor);

    int product_id = 0;
    refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDProductIDKey));
    if (refCF)
        CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &product_id);

    int version = 0;
    refCF = IOHIDDeviceGetProperty(p_device_ref, CFSTR(kIOHIDVersionNumberKey));
    if (refCF)
        CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &version);

    if (vendor && product_id) {
          uint32_t big3 = qToBigEndian(3);
          uint32_t bigVendor = qToBigEndian(vendor);
          uint32_t bigProductId = qToBigEndian(product_id);
          uint32_t bigVersion = qToBigEndian(version);

          QString uid = QString("%1%2%3%4").arg(big3, 8, 16, QChar('0')).arg(bigVendor, 8, 16, QChar('0')).arg(bigProductId, 8, 16, QChar('0')).arg(bigVersion, 8, 16, QChar('0'));
          input->updateJoyConnection(id, true, name, uid);
    } else {
        // Bluetooth device.
        QByteArray guid = "05000000";
        for (int i = 0; i < 12; i++) {
            if (i < name.size())
                guid += _hex_str(name[i]);
            else
                guid += "00";
        }
        input->updateJoyConnection(id, true, name, guid);
    }

    CFArrayRef array = IOHIDDeviceCopyMatchingElements(p_device_ref, nullptr, kIOHIDOptionsTypeNone);
    if (array) {
        p_joy->add_hid_elements(array);
        CFRelease(array);
    }
    // Xbox controller hat values start at 1 rather than 0.
    p_joy->offsetHat = vendor == 0x45e &&
            (product_id == 0x0b05 ||
                    product_id == 0x02e0 ||
                    product_id == 0x02fd ||
                    product_id == 0x0b13);

    return true;
}

#define FF_ERR()                        \
    {                                   \
        if (ret != FF_OK) {             \
            FFReleaseDevice(forceFeedbackDevice); \
            forceFeedbackDevice = nullptr;        \
            return false;               \
        }                               \
    }
bool Joypad::config_force_feedback(io_service_t p_service)
{
    HRESULT ret = FFCreateDevice(p_service, &forceFeedbackDevice);

    if (ret != FF_OK)
        return false;

    ret = FFDeviceSendForceFeedbackCommand(forceFeedbackDevice, FFSFFC_RESET);
    FF_ERR();

    ret = FFDeviceSendForceFeedbackCommand(forceFeedbackDevice, FFSFFC_SETACTUATORSON);
    FF_ERR();

    if (check_ff_features()) {
        ret = FFDeviceCreateEffect(forceFeedbackDevice, kFFEffectType_ConstantForce_ID, &forceFeedbackEffect, &forcefeedbackObject);
        FF_ERR();
        return true;
    }
    FFReleaseDevice(forceFeedbackDevice);
    forceFeedbackDevice = nullptr;
    return false;
}
#undef FF_ERR

#define TEST_FF(ff) (features.supportedEffects & (ff))
bool Joypad::check_ff_features()
{
    FFCAPABILITIES features;
    HRESULT ret = FFDeviceGetForceFeedbackCapabilities(forceFeedbackDevice, &features);
    if (ret == FF_OK && (features.supportedEffects & FFCAP_ET_CONSTANTFORCE)) {
        uint32_t val;
        ret = FFDeviceGetForceFeedbackProperty(forceFeedbackDevice, FFPROP_FFGAIN, &val, sizeof(val));
        if (ret != FF_OK) {
            return false;
        }
        int num_axes = features.numFfAxes;
        forceFeedbackAxes = (DWORD *)::malloc(sizeof(DWORD) * num_axes);
        forceFeedbackDirections = (LONG *)::malloc(sizeof(LONG) * num_axes);

        for (int i = 0; i < num_axes; i++) {
            forceFeedbackAxes[i] = features.ffAxes[i];
            forceFeedbackDirections[i] = 0;
        }

        forceFeedbackEffect.cAxes = num_axes;
        forceFeedbackEffect.rgdwAxes = forceFeedbackAxes;
        forceFeedbackEffect.rglDirection = forceFeedbackDirections;
        return true;
    }
    return false;
}

static HatMask process_hat_value(int p_min, int p_max, int p_value, bool p_offset_hat)
{
    int range = (p_max - p_min + 1);
    int value = p_value - p_min;
        HatMask hat_value = HatMask::Center;
    if (range == 4)
        value *= 2;

    if (p_offset_hat)
        value -= 1;

    switch (value) {
        case 0:
            hat_value = HatMask::Up;
            break;
        case 1:
            hat_value = (HatMask::Up | HatMask::Right);
            break;
        case 2:
            hat_value = HatMask::Right;
            break;
        case 3:
            hat_value = (HatMask::Down | HatMask::Right);
            break;
        case 4:
            hat_value = HatMask::Down;
            break;
        case 5:
            hat_value = (HatMask::Down | HatMask::Left);
            break;
        case 6:
            hat_value = HatMask::Left;
            break;
        case 7:
            hat_value = (HatMask::Up | HatMask::Left);
            break;
        default:
            hat_value = HatMask::Center;
            break;
    }
    return hat_value;
}

void MacOsJoystickInput::poll_joypads() const
{
    while (CFRunLoopRunInMode(JOYPAD_LOOP_RUN_MODE, 0, TRUE) == kCFRunLoopRunHandledSource) {
        // No-op. Pending callbacks will fire.
    }
}

static float axis_correct(int p_value, int p_min, int p_max)
{
    // Convert to a value between -1.0f and 1.0f.
    return 2.0f * (p_value - p_min) / (p_max - p_min) - 1.0f;
}

void MacOsJoystickInput::processJoypads()
{
    auto input = QUniversalInput::instance();
    poll_joypads();

    for (auto &joy : m_deviceList) {
        for (int j = 0; j < joy.axisElements.size(); j++) {
            RecElement &elem = joy.axisElements[j];
            int value = joy.get_hid_element_state(&elem);
            input->joyAxis(joy.id, JoyAxis(j), axis_correct(value, elem.min, elem.max));
        }
        for (int j = 0; j < joy.buttonElements.size(); j++) {
            int value = joy.get_hid_element_state(&joy.buttonElements[j]);
            input->joyButton(joy.id, JoyButton(j), (value >= 1));
        }
        for (int j = 0; j < joy.hatElements.size(); j++) {
            RecElement &elem = joy.hatElements[j];
            int value = joy.get_hid_element_state(&elem);
            HatMask hat_value = process_hat_value(elem.min, elem.max, value, joy.offsetHat);
            input->joyHat(joy.id, hat_value);
        }

        if (joy.forceFeedbackService) {
            uint64_t timestamp = input->joyVibrationTimestamp(joy.id);
            if (timestamp > joy.forceFeedbackTimestamp) {
                QVector2D strength = input->joyVibrationStrength(joy.id);
                float duration = input->joyVibrationDuration(joy.id);
                if (strength.x() == 0 && strength.y() == 0) {
                    joypad_vibration_stop(joy.id, timestamp);
                } else {
                    float gain = qMax(strength.x(), strength.y());
                    joypad_vibration_start(joy.id, gain, duration, timestamp);
                }
            }
        }

    }
}

void MacOsJoystickInput::joypad_vibration_start(int p_id, float p_magnitude, float p_duration, uint64_t p_timestamp)
{
    Joypad *joy = &m_deviceList[get_joy_index(p_id)];
    joy->forceFeedbackTimestamp = p_timestamp;
    joy->forceFeedbackEffect.dwDuration = p_duration * FF_SECONDS;
    joy->forceFeedbackEffect.dwGain = p_magnitude * FF_FFNOMINALMAX;
    FFEffectSetParameters(joy->forcefeedbackObject, &joy->forceFeedbackEffect, FFEP_DURATION | FFEP_GAIN);
    FFEffectStart(joy->forcefeedbackObject, 1, 0);
}

void MacOsJoystickInput::joypad_vibration_stop(int p_id, uint64_t p_timestamp)
{
    Joypad *joy = &m_deviceList[get_joy_index(p_id)];
    joy->forceFeedbackTimestamp = p_timestamp;
    FFEffectStop(joy->forcefeedbackObject);
}

int MacOsJoystickInput::get_joy_index(int p_id) const
{
    for (int i = 0; i < m_deviceList.size(); i++)
        if (m_deviceList[i].id == p_id)
            return i;

    return -1;
}

int MacOsJoystickInput::get_joy_ref(IOHIDDeviceRef p_device) const
{
    for (int i = 0; i < m_deviceList.size(); i++)
        if (m_deviceList[i].deviceRef == p_device)
            return i;

    return -1;
}

bool MacOsJoystickInput::have_device(IOHIDDeviceRef p_device) const
{
    for (int i = 0; i < m_deviceList.size(); i++)
        if (m_deviceList[i].deviceRef == p_device)
            return true;

    return false;
}

static CFDictionaryRef create_match_dictionary(const UInt32 page, const UInt32 usage, int *okay)
{
    CFDictionaryRef retval = nullptr;
    CFNumberRef pageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
    CFNumberRef usageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);

    if (pageNumRef && usageNumRef) {
        const void *keys[2] = { (void *)CFSTR(kIOHIDDeviceUsagePageKey), (void *)CFSTR(kIOHIDDeviceUsageKey) };
        const void *vals[2] = { (void *)pageNumRef, (void *)usageNumRef };
        retval = CFDictionaryCreate(kCFAllocatorDefault, keys, vals, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }

    if (pageNumRef) {
        CFRelease(pageNumRef);
    }
    if (usageNumRef) {
        CFRelease(usageNumRef);
    }

    if (!retval) {
        *okay = 0;
    }

    return retval;
}

void MacOsJoystickInput::config_hid_manager(CFArrayRef p_matching_array) const
{
    CFRunLoopRef runloop = CFRunLoopGetCurrent();
    IOReturn ret = IOHIDManagerOpen(m_hidManager, kIOHIDOptionsTypeNone);
    Q_ASSERT(ret == kIOReturnSuccess);

    IOHIDManagerSetDeviceMatchingMultiple(m_hidManager, p_matching_array);
    IOHIDManagerRegisterDeviceMatchingCallback(m_hidManager, joypad_added_callback, nullptr);
    IOHIDManagerRegisterDeviceRemovalCallback(m_hidManager, joypad_removed_callback, nullptr);
    IOHIDManagerScheduleWithRunLoop(m_hidManager, runloop, JOYPAD_LOOP_RUN_MODE);

    while (CFRunLoopRunInMode(JOYPAD_LOOP_RUN_MODE, 0, TRUE) == kCFRunLoopRunHandledSource) {
        // No-op. Callback fires once per existing device.
    }
}

MacOsJoystickInput::MacOsJoystickInput()
{
    self = this;

    int okay = 1;
    const void *vals[] = {
        (void *)create_match_dictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick, &okay),
        (void *)create_match_dictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad, &okay),
        (void *)create_match_dictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController, &okay),
    };
    const size_t n_elements = sizeof(vals) / sizeof(vals[0]);
    CFArrayRef array = okay ? CFArrayCreate(kCFAllocatorDefault, vals, n_elements, &kCFTypeArrayCallBacks) : nullptr;

    for (size_t i = 0; i < n_elements; i++)
        if (vals[i])
            CFRelease((CFTypeRef)vals[i]);

    if (array) {
        m_hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        if (m_hidManager)
            config_hid_manager(array);

        CFRelease(array);
    }

    startTimer(1);
}

MacOsJoystickInput::~MacOsJoystickInput()
{
    for (auto &joy : m_deviceList)
        joy.free();
    m_deviceList.clear();

    IOHIDManagerUnscheduleFromRunLoop(m_hidManager, CFRunLoopGetCurrent(), JOYPAD_LOOP_RUN_MODE);
    IOHIDManagerClose(m_hidManager, kIOHIDOptionsTypeNone);
    CFRelease(m_hidManager);
    m_hidManager = nullptr;
}

void MacOsJoystickInput::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    processJoypads();
}

QT_END_NAMESPACE
