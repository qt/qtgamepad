# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# The test reaches private QUniversalInput members through a friend declaration
# that is only present in internal builds, so it can only be built there.
qt_internal_add_test(tst_mappingtransform
    SOURCES
        tst_mappingtransform.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::UniversalInput
)
