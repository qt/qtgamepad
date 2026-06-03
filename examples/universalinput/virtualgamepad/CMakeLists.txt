# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(virtualgamepad LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/universalinput/virtualgamepad")

find_package(Qt6 REQUIRED COMPONENTS Core Gui UniversalInput Quick QuickActionStore QuickUniversalInput)

qt_add_executable(virtualgamepad
    main.cpp
)

qt_add_qml_module(virtualgamepad
    URI Example
    VERSION 1.0
    QML_FILES main.qml
    NO_RESOURCE_TARGET_PATH
    QML_FILES Joystick.qml
    QML_FILES VirtualGamepad.qml
    QML_FILES JoyButton.qml
    QML_FILES ButtonIsland.qml
)

set_target_properties(virtualgamepad PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(virtualgamepad PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UniversalInput
    Qt::Quick
    Qt::QuickActionStore
    Qt::QuickUniversalInput
)

install(TARGETS virtualgamepad
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
