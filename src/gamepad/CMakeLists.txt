# Generated from gamepad.pro.

#####################################################################
## Gamepad Module:
#####################################################################

add_qt_module(Gamepad
    PLUGIN_TYPES gamepads
    SOURCES
        qgamepad.cpp qgamepad.h
        qgamepadbackend.cpp qgamepadbackend_p.h
        qgamepadbackendfactory.cpp qgamepadbackendfactory_p.h
        qgamepadbackendplugin.cpp qgamepadbackendplugin_p.h
        qgamepadkeynavigation.cpp qgamepadkeynavigation.h
        qgamepadmanager.cpp qgamepadmanager.h
        qtgamepadglobal.h
    DEFINES
        QT_BUILD_GAMEPAD_LIB
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
)

if(ANDROID)
    set_property(TARGET Gamepad APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/QtAndroidGamepad.jar
    )
    set_property(TARGET Gamepad APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        plugins/gamepads/libandroidgamepad.so
    )
endif()

#### Keys ignored in scope 1:.:.:gamepad.pro:<TRUE>:
# ANDROID_FEATURES = "android.hardware.gamepad"
# _LOADED = "qt_module"
add_qt_docs(Gamepad
    doc/qtgamepad.qdocconf
)

