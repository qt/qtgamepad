# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(QuickGamepad
    URI "QtGamepad"
    VERSION "${PROJECT_VERSION}"
    DEPENDENCIES QtQuick
    SOURCES
        qquickgamepad.cpp qquickgamepad_p.h
    QML_FILES
        ${qml_files}
    LIBRARIES
        Qt::Core
        Qt::GamepadPrivate
        Qt::Quick
        Qt::QuickPrivate
)
