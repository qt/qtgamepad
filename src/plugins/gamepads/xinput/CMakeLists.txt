# Generated from xinput.pro.

#####################################################################
## xinputgamepad Plugin:
#####################################################################

add_qt_plugin(xinputgamepad
    TYPE gamepads
    CLASS_NAME QXInputGamepadBackendPlugin
    SOURCES
        main.cpp
        qxinputgamepadbackend.cpp qxinputgamepadbackend_p.h
    LIBRARIES
        Qt::GamepadPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gamepad
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:xinput.pro:<TRUE>:
# OTHER_FILES = "xinput.json"
# _LOADED = "qt_plugin"
