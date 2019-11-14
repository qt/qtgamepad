# Generated from evdev.pro.

#####################################################################
## evdevgamepad Plugin:
#####################################################################

qt_add_plugin(evdevgamepad
    TYPE gamepads
    CLASS_NAME QEvdevGamepadBackendPlugin
    SOURCES
        main.cpp
        qevdevgamepadbackend.cpp qevdevgamepadbackend_p.h
    LIBRARIES
        Qt::CorePrivate
        Qt::DeviceDiscoverySupportPrivate
        Qt::GamepadPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::DeviceDiscoverySupport
        Qt::Gamepad
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:evdev.pro:<TRUE>:
# OTHER_FILES = "evdev.json"
