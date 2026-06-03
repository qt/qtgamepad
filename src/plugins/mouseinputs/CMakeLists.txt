# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# for each platform, add a subdirectory with the platform name.
# Android and iOS have no mouse backend.
if(MACOS)
    add_subdirectory(macos)
endif()
if(WIN32)
    add_subdirectory(windows)
endif()
if(LINUX)
    add_subdirectory(linux)
endif()
