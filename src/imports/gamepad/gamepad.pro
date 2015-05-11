CXX_MODULE = gamepad
TARGET  = declarative_gamepad
TARGETPATH = QtGamepad
IMPORT_VERSION = 1.0

QT += qml quick gamepad
SOURCES += \
    $$PWD/qtgamepad.cpp \
    qgamepadmouseitem.cpp

load(qml_plugin)

OTHER_FILES += qmldir

HEADERS += \
    qgamepadmouseitem.h
