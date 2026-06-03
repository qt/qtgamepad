# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(actionpong LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/universalinput/actionpong")

find_package(Qt6 REQUIRED COMPONENTS Core Gui UniversalInput Quick QuickActionStore QuickUniversalInput)

qt_add_executable(actionpong
    main.cpp
)

qt_add_qml_module(actionpong
    URI Example
    VERSION 1.0
    QML_FILES main.qml
    QML_FILES ActionMap.qml
    QML_FILES Paddle.qml
    QML_FILES Ball.qml
    QML_FILES PaddleController.qml
    NO_RESOURCE_TARGET_PATH
)

set_target_properties(actionpong PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(actionpong PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UniversalInput
    Qt::Quick
    Qt::QuickActionStore
    Qt::QuickUniversalInput
)

install(TARGETS actionpong
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
