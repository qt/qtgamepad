# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(Gamepad
    NO_PRIVATE_MODULE
    SOURCES
        qgamepad.cpp qgamepad.h
    DEFINES
        QT_BUILD_GAMEPAD_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::UniversalInput
)
