# Generated from gamepad.pro.

#####################################################################
## declarative_gamepad Plugin:
#####################################################################

qt_add_qml_module(declarative_gamepad
    URI "QtGamepad"
    VERSION "1.${CMAKE_PROJECT_VERSION_MINOR}"
    CLASSNAME QGamepadModule
    SKIP_TYPE_REGISTRATION
    SOURCES
        qgamepadmouseitem.cpp qgamepadmouseitem.h
        qtgamepad.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gamepad
        Qt::Gui
        Qt::Qml
        Qt::Quick
)

#### Keys ignored in scope 1:.:.:gamepad.pro:<TRUE>:
# CXX_MODULE = "gamepad"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# OTHER_FILES = "qmldir"
# TARGETPATH = "QtGamepad"
