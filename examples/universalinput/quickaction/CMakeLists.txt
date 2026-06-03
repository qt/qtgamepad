# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(quickaction LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/universalinput/quickaction")

find_package(Qt6 REQUIRED COMPONENTS Core Gui UniversalInput Quick QuickActionStore QuickUniversalInput)

qt_add_executable(quickaction
    main.cpp
)

qt_add_qml_module(quickaction
    URI Example
    VERSION 1.0
    QML_FILES main.qml
    QML_FILES Controller.qml
    QML_FILES GameObject.qml
    QML_FILES Player.qml
    QML_FILES GameObjectList.qml
    QML_FILES Platform.qml
    NO_RESOURCE_TARGET_PATH
)

set_target_properties(quickaction PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(quickaction PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UniversalInput
    Qt::Quick
    Qt::QuickActionStore
    Qt::QuickUniversalInput
)

install(TARGETS quickaction
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
