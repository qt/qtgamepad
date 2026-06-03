# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(LinuxJoystickInputPlugin
    OUTPUT_NAME linuxjoystickinput
    PLUGIN_TYPE joystickinputs
    SOURCES
        linuxjoystickinput.cpp linuxjoystickinput.h
        linuxjoystickinputplugin.cpp linuxjoystickinputplugin.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::UniversalInput
        Qt::UniversalInputPrivate
        # ### should check for libudev availability in CMake
        udev
)
