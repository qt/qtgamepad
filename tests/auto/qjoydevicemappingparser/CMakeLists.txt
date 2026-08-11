# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qjoydevicemappingparser
    SOURCES
        tst_qjoydevicemappingparser.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::UniversalInput
        Qt::UniversalInputPrivate
)

set_source_files_properties(testdata/gamecontrollerdb-fixture.txt PROPERTIES
    QT_RESOURCE_ALIAS gamecontrollerdb-fixture.txt
)

# Embed the test fixture so it is available at :/gamecontrollerdb-fixture.txt
# regardless of the working directory or installed test layout.
qt_internal_add_resource(tst_qjoydevicemappingparser "fixture"
    PREFIX
        "/"
    FILES
        "testdata/gamecontrollerdb-fixture.txt"
)
