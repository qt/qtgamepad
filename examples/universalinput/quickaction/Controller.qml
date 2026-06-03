// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtActionStore

ActionStore {
    id: actionStore
    property int device: ActionStore.Controller.Device0

    InputAction {
        title: "Left"

        JoyAxisEvent {
            axis: UniversalInput.JoyAxis.LeftX
            direction: ActionStore.AxisDirection.Left
            deadzone: 0
        }

        KeyboardEvent {
            key: Qt.Key_A
            isPressed: true
        }

        KeyboardEvent {
            key: Qt.Key_Left
            isPressed: true
        }
    }

    InputAction {
        title: "Right"

        JoyAxisEvent {
            axis: UniversalInput.JoyAxis.LeftX
            direction: ActionStore.AxisDirection.Right
            deadzone: 0
        }

        KeyboardEvent {
            key: Qt.Key_D
            isPressed: true
        }

        KeyboardEvent {
            key: Qt.Key_Right
            isPressed: true
        }
    }

    InputAction {
        title: "Up"

        JoyAxisEvent {
            axis: UniversalInput.JoyAxis.LeftY
            direction: ActionStore.AxisDirection.Up
            deadzone: 0
        }
    }

    InputAction {
        title: "Down"

        JoyAxisEvent {
            axis: UniversalInput.JoyAxis.LeftY
            direction: ActionStore.AxisDirection.Down
            deadzone: 0
        }
    }

    InputAction {
        title: "Shoot"

        JoyButtonEvent {
            button: UniversalInput.JoyButton.RightShoulder
            isPressed: true
        }
    }

    InputAction {
        title: "Jump"

        JoyButtonEvent {
            button: UniversalInput.JoyButton.A
            isPressed: true
        }

        KeyboardEvent {
            key: Qt.Key_Space
            isPressed: true
        }
    }
}
