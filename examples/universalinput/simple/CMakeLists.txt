# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(gamepad_simple LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/universalinput/simple")

find_package(Qt6 REQUIRED COMPONENTS Core Gui UniversalInput Gamepad)

qt_add_executable(gamepad_simple
    main.cpp
    gamepadmonitor.h
    gamepadmonitor.cpp
)

set_target_properties(gamepad_simple PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(gamepad_simple PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Gamepad
)

install(TARGETS gamepad_simple
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)


