# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(QuickActionStore
    URI "QtActionStore"
    VERSION "${PROJECT_VERSION}"
    DEPENDENCIES QtQuick
    SOURCES
        qquickactionstore.cpp qquickactionstore_p.h
        qquickeventlistener.cpp qquickeventlistener_p.h
        qquickactionhandler.cpp qquickactionhandler_p.h
    QML_FILES
        ${qml_files}
    LIBRARIES
        Qt::Core
        Qt::Quick
        Qt::QuickPrivate
        Qt::UniversalInput
)
