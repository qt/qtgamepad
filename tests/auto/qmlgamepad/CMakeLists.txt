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
        # Link the modules whose QML types the test imports. Their qmldir marks
        # the plugin optional, so having the types registered up front means the
        # plugin never has to be loaded, and linking them also makes them a
        # dependency of the test rather than something a deployment tool has to
        # infer.
        Qt::QuickGamepad
        Qt::QuickActionStore
        Qt::QuickUniversalInput
)


# The test imports the QML modules from strings, which the import scanner cannot
# see. Point it at this directory, where imports.qml names them, so that they
# are deployed on platforms that package the test into a bundle.
set_target_properties(tst_qmlgamepad PROPERTIES
    QT_QML_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}"
)
