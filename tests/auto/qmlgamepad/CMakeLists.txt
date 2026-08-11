# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qmlgamepad
    SOURCES
        tst_qmlgamepad.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::UniversalInput
)

# The QML modules build no on-screen content here, so run headless.
set_tests_properties(tst_qmlgamepad PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
