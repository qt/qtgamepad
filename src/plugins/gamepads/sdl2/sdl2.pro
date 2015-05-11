TARGET = sdl2gamepad
QT += gamepad gamepad-private

PLUGIN_TYPE = gamepads
PLUGIN_CLASS_NAME = QSdl2GamepadBackendPlugin
load(qt_plugin)

osx {
    INCLUDEPATH += /Library/Frameworks/SDL2.framework/Headers
    LIBS += -F/Library/Frameworks/ -framework SDL2
}

unix:!osx{
    CONFIG += link_pkgconfig
    PKGCONFIG += sdl2
}

win32: LIBS += -lSDL2

HEADERS += qsdlgamepadbackend_p.h
SOURCES += \
    qsdlgamepadbackend.cpp \
    main.cpp

OTHER_FILES += \
    sdl2.json

