# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(UniversalInput
    PLUGIN_TYPES joystickinputs mouseinputs
    SOURCES
        qjoystickinput_p.h
        qjoystickinputplugin_p.h
        qjoystickinputfactory.cpp qjoystickinputfactory_p.h
        quniversalinput.cpp quniversalinput.h quniversalinput_p.h
        qtuniversalinputglobal_p.h
        qtuniversalinputglobal.h
        qjoydevicemappingparser.cpp qjoydevicemappingparser_p.h
        qactionstore.cpp qactionstore.h
        qmouseinput_p.h
        qmouseinputfactory.cpp qmouseinputfactory_p.h
        qmouseinputplugin_p.h
    DEFINES
        QT_BUILD_UNIVERSALINPUT_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
)

set_source_files_properties(../3rdparty/sdlgamecontrollerdb/gamecontrollerdb.txt PROPERTIES
    QT_RESOURCE_ALIAS gamecontrollerdb.txt
)

# Embed the SDL game controller database. The QT_RESOURCE_ALIAS above
# makes it available at
# :/qt-project.org/qtuniversalinput/gamecontrollerdb.txt
qt_internal_add_resource(UniversalInput "qtuniversalinputresources"
    PREFIX
        "/qt-project.org/qtuniversalinput/"
    FILES
        "../3rdparty/sdlgamecontrollerdb/gamecontrollerdb.txt"
)
