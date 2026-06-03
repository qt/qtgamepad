// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
  Originally based on code from "platform/ios/joypad_ios.mm" from Godot Engine v4.0
  Copyright (c) 2014-present Godot Engine contributors
  Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
*/

#include "iosjoystickinput.h"

// Bring the QUniversalInput input enums into scope for this file.
using HatMask = QUniversalInput::HatMask;
using JoyAxis = QUniversalInput::JoyAxis;
using JoyButton = QUniversalInput::JoyButton;

#import <GameController/GameController.h>

@interface JoypadIOSObserver : NSObject

- (void)startObserving;
- (void)startProcessing;
- (void)finishObserving;

@property(assign, nonatomic) BOOL isObserving;
@property(assign, nonatomic) BOOL isProcessing;
@property(strong, nonatomic) NSMutableDictionary *connectedJoypads;
@property(strong, nonatomic) NSMutableArray *joypadsQueue;

@end

@implementation JoypadIOSObserver

- (instancetype)init
{
    self = [super init];

    if (self) {
        self.isObserving = NO;
        self.isProcessing = NO;
    }

    return self;
}

- (void)startProcessing
{
    self.isProcessing = YES;

    for (GCController *controller in self.joypadsQueue) {
        [self addiOSJoypad:controller];
    }

    [self.joypadsQueue removeAllObjects];
}

- (void)startObserving
{
    if (self.isObserving)
        return;

    self.isObserving = YES;

    self.connectedJoypads = [NSMutableDictionary dictionary];
    self.joypadsQueue = [NSMutableArray array];

    // get told when controllers connect, this will be called right away for
    // already connected controllers
    [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(controllerWasConnected:)
                   name:GCControllerDidConnectNotification
                 object:nil];

    // get told when controllers disconnect
    [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(controllerWasDisconnected:)
                   name:GCControllerDidDisconnectNotification
                 object:nil];
}

- (void)finishObserving
{
    if (self.isObserving)
        [[NSNotificationCenter defaultCenter] removeObserver:self];

    self.isObserving = NO;
    self.isProcessing = NO;

    self.connectedJoypads = nil;
    self.joypadsQueue = nil;
}

- (void)dealloc
{
    [self finishObserving];
    [super dealloc];
}

- (int)getJoyIdForController:(GCController *)controller
{
    NSArray *keys = [self.connectedJoypads allKeysForObject:controller];

    for (NSNumber *key in keys) {
        int joy_id = [key intValue];
        return joy_id;
    }

    return -1;
}

- (void)addiOSJoypad:(GCController *)controller
{
    //     get a new id for our controller
    int joy_id = QUniversalInput::instance()->unusedJoyId();

    if (joy_id == -1) {
        printf("Couldn't retrieve new joy id\n");
        return;
    }

    // assign our player index
    if (controller.playerIndex == GCControllerPlayerIndexUnset)
        controller.playerIndex = [self getFreePlayerIndex];


    // Report the new controller
    QUniversalInput::instance()->updateJoyConnection(joy_id, true, QString::fromUtf8([controller.vendorName UTF8String]));

    // add it to our dictionary, this will retain our controllers
    [self.connectedJoypads setObject:controller forKey:[NSNumber numberWithInt:joy_id]];

    // set our input handler
    [self setControllerInputHandler:controller];
}

- (void)controllerWasConnected:(NSNotification *)notification
{
    // get our controller
    GCController *controller = (GCController *)notification.object;

    if (!controller) {
        printf("Couldn't retrieve new controller\n");
        return;
    }

    if ([[self.connectedJoypads allKeysForObject:controller] count] > 0) {
        printf("Controller is already registered\n");
    } else if (!self.isProcessing) {
        [self.joypadsQueue addObject:controller];
    } else {
        [self addiOSJoypad:controller];
    }
}

- (void)controllerWasDisconnected:(NSNotification *)notification
{
    // find our joystick, there should be only one in our dictionary
    GCController *controller = (GCController *)notification.object;

    if (!controller)
        return;

    NSArray *keys = [self.connectedJoypads allKeysForObject:controller];
    for (NSNumber *key in keys) {
        // Report this joystick is no longer there
        int joy_id = [key intValue];
        QUniversalInput::instance()->updateJoyConnection(joy_id, false, "");

        // and remove it from our dictionary
        [self.connectedJoypads removeObjectForKey:key];
    }
}

- (GCControllerPlayerIndex)getFreePlayerIndex {
    bool have_player_1 = false;
    bool have_player_2 = false;
    bool have_player_3 = false;
    bool have_player_4 = false;

    if (self.connectedJoypads == nil) {
        NSArray *keys = [self.connectedJoypads allKeys];
        for (NSNumber *key in keys) {
            GCController *controller = [self.connectedJoypads objectForKey:key];
            if (controller.playerIndex == GCControllerPlayerIndex1) {
                have_player_1 = true;
            } else if (controller.playerIndex == GCControllerPlayerIndex2) {
                have_player_2 = true;
            } else if (controller.playerIndex == GCControllerPlayerIndex3) {
                have_player_3 = true;
            } else if (controller.playerIndex == GCControllerPlayerIndex4) {
                have_player_4 = true;
            }
        }
    }

    if (!have_player_1) {
        return GCControllerPlayerIndex1;
    } else if (!have_player_2) {
        return GCControllerPlayerIndex2;
    } else if (!have_player_3) {
        return GCControllerPlayerIndex3;
    } else if (!have_player_4) {
        return GCControllerPlayerIndex4;
    } else {
        return GCControllerPlayerIndexUnset;
    }
}

- (void)setControllerInputHandler:(GCController *)controller
{
    // Hook in the callback handler for the correct gamepad profile.
    // This is a bit of a weird design choice on Apples part.
    // You need to select the most capable gamepad profile for the
    // gamepad attached.
    if (controller.extendedGamepad != nil) {
        // The extended gamepad profile has all the input you could possibly find on
        // a gamepad but will only be active if your gamepad actually has all of
        // these...

        controller.extendedGamepad.valueChangedHandler = ^(GCExtendedGamepad *gamepad, GCControllerElement *element) {

            int joy_id = [self getJoyIdForController:controller];

            if (element == gamepad.buttonA) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::A,
                        gamepad.buttonA.isPressed);
            } else if (element == gamepad.buttonB) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::B,
                        gamepad.buttonB.isPressed);
            } else if (element == gamepad.buttonX) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::X,
                        gamepad.buttonX.isPressed);
            } else if (element == gamepad.buttonY) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::Y,
                        gamepad.buttonY.isPressed);
            } else if (element == gamepad.leftShoulder) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::LeftShoulder,
                        gamepad.leftShoulder.isPressed);
            } else if (element == gamepad.rightShoulder) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::RightShoulder,
                        gamepad.rightShoulder.isPressed);
            } else if (element == gamepad.dpad) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadUp,
                        gamepad.dpad.up.isPressed);
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadDown,
                        gamepad.dpad.down.isPressed);
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadLeft,
                        gamepad.dpad.left.isPressed);
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadRight,
                        gamepad.dpad.right.isPressed);
            }

            if (element == gamepad.leftThumbstick) {
                float value = gamepad.leftThumbstick.xAxis.value;
                QUniversalInput::instance()->joyAxis(joy_id, JoyAxis::LeftX, value);
                value = -gamepad.leftThumbstick.yAxis.value;
                QUniversalInput::instance()->joyAxis(joy_id, JoyAxis::LeftY, value);
            } else if (element == gamepad.rightThumbstick) {
                float value = gamepad.rightThumbstick.xAxis.value;
                QUniversalInput::instance()->joyAxis(joy_id, JoyAxis::RightX, value);
                value = -gamepad.rightThumbstick.yAxis.value;
                QUniversalInput::instance()->joyAxis(joy_id, JoyAxis::RightY, value);
            } else if (element == gamepad.leftTrigger) {
                float value = gamepad.leftTrigger.value;
                QUniversalInput::instance()->joyAxis(joy_id, JoyAxis::TriggerLeft, value);
            } else if (element == gamepad.rightTrigger) {
                float value = gamepad.rightTrigger.value;
                QUniversalInput::instance()->joyAxis(joy_id, JoyAxis::TriggerRight, value);
            }
        };
    } else if (controller.microGamepad != nil) {
        // micro gamepads were added in OS 9 and feature just 2 buttons and a d-pad

        controller.microGamepad.valueChangedHandler = ^(GCMicroGamepad *gamepad, GCControllerElement *element) {

            int joy_id = [self getJoyIdForController:controller];

            if (element == gamepad.buttonA) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::A,
                        gamepad.buttonA.isPressed);
            } else if (element == gamepad.buttonX) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::X,
                        gamepad.buttonX.isPressed);
            } else if (element == gamepad.dpad) {
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadUp,
                        gamepad.dpad.up.isPressed);
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadDown,
                        gamepad.dpad.down.isPressed);
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadLeft, gamepad.dpad.left.isPressed);
                QUniversalInput::instance()->joyButton(joy_id, JoyButton::DpadRight, gamepad.dpad.right.isPressed);
            }
        };
    }
}

@end

QT_BEGIN_NAMESPACE

IosJoystickInput::IosJoystickInput()
{
    m_observer = [[JoypadIOSObserver alloc] init];
    [m_observer startObserving];
    startProcessing();
}

IosJoystickInput::~IosJoystickInput()
{
    if (m_observer) {
        [m_observer finishObserving];
        m_observer = nil;
    }
}

void IosJoystickInput::startProcessing()
{
    if (m_observer)
        [m_observer startProcessing];
}

QT_END_NAMESPACE
