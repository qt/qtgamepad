# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(QuickUniversalInput
    URI "QtUniversalInput"
    VERSION "${PROJECT_VERSION}"
    DEPENDENCIES QtQuick
    SOURCES
        qquickuniversalinput.cpp qquickuniversalinput_p.h
    QML_FILES
        ${qml_files}
    LIBRARIES
        Qt::Core
        Qt::Quick
        Qt::QuickPrivate
        Qt::UniversalInput
)
