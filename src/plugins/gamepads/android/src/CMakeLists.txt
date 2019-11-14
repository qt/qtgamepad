# Generated from src.pro.

#####################################################################
## androidgamepad Plugin:
#####################################################################

qt_add_plugin(androidgamepad
    TYPE gamepads
    CLASS_NAME QAndroidGamepadBackendPlugin
    SOURCES
        main.cpp
        qandroidgamepadbackend.cpp qandroidgamepadbackend_p.h
    LIBRARIES
        Qt::CorePrivate
        Qt::GamepadPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gamepad
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:src.pro:<TRUE>:
# DISTFILES = "android.json"
