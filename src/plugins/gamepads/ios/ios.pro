TARGET = iosgamepad
QT += gamepad gamepad-private

PLUGIN_TYPE = gamepads
PLUGIN_EXTENDS = gamepad
PLUGIN_CLASS_NAME = QIosGamepadBackendPlugin
load(qt_plugin)

LIBS += -framework GameController -framework Foundation

HEADERS += qiosgamepadbackend_p.h
OBJECTIVE_SOURCES += \
    qiosgamepadbackend.mm

SOURCES += \
    main.cpp

OTHER_FILES += \
    ios.json


