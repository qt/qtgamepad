// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// This file is never loaded. The test builds its QML from strings, which the
// QML import scanner cannot see, so the modules it needs would not be deployed
// on platforms that package the application. Listing them here makes them
// visible to the scanner.

import QtQml
import QtGamepad
import QtActionStore
import QtUniversalInput

QtObject {
}
