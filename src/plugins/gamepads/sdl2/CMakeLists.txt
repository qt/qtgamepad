# Generated from sdl2.pro.

#####################################################################
## sdl2gamepad Plugin:
#####################################################################

qt_add_plugin(sdl2gamepad
    TYPE gamepads
    CLASS_NAME QSdl2GamepadBackendPlugin
    SOURCES
        main.cpp
        qsdlgamepadbackend.cpp qsdlgamepadbackend_p.h
    LIBRARIES
        Qt::GamepadPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gamepad
        Qt::Gui
        WrapSDL2::WrapSDL2
)

#### Keys ignored in scope 1:.:.:sdl2.pro:<TRUE>:
# OTHER_FILES = "sdl2.json"
