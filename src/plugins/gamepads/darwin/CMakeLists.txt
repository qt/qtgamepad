# Generated from darwin.pro.

#####################################################################
## darwingamepad Plugin:
#####################################################################

add_qt_plugin(darwingamepad
    TYPE gamepads
    CLASS_NAME QDarwinGamepadBackendPlugin
    SOURCES
        main.cpp
        qdarwingamepadbackend.mm qdarwingamepadbackend_p.h
    LIBRARIES
        Qt::GamepadPrivate
    PUBLIC_LIBRARIES
        ${FWFoundation}
        ${FWGameController}
        Qt::Core
        Qt::Gamepad
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:darwin.pro:<TRUE>:
# OTHER_FILES = "darwin.json"
# PLUGIN_EXTENDS = "gamepad"
# _LOADED = "qt_plugin"
