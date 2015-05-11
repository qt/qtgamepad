Qt Gamepad

A Qt 5 module that adds support for getting events from gamepad devices on multiple platforms.

Currently supports Linux (evdev), Windows (xinput) and OS X (via SDL2).

This module provides classes that can:
 - Read input events from game controllers (Button and Axis events), both from C++ and Qt Quick (QML)
 - Provide a queryable input state (by processing events)
 - Provide key bindings
