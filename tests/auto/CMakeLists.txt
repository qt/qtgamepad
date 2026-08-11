# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_BUILD_MINIMAL_STATIC_TESTS)
    return()
endif()

add_subdirectory(qjoydevicemappingparser)
add_subdirectory(qactionstore)
add_subdirectory(enums)
add_subdirectory(quniversalinput)
add_subdirectory(qgamepad)
add_subdirectory(factory)

# The mapping transform test reaches private QUniversalInput members and can
# only be built when the internal API is available.
if(QT_FEATURE_private_tests)
    add_subdirectory(mappingtransform)
endif()

# The QML layer tests need Qt Quick, which is an optional dependency.
if(TARGET Qt::Qml AND TARGET Qt::Quick)
    add_subdirectory(qmlgamepad)
endif()
