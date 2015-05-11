SOURCES += main.cpp
QT =
CONFIG -= qt

osx:{
    INCLUDEPATH += /Library/Frameworks/SDL2.framework/Headers
    LIBS += -F/Library/Frameworks/ -framework SDL2
}

unix:!osx{
    CONFIG += link_pkgconfig
    PKGCONFIG += sdl2
}

win32: LIBS += -lSDL2 -lSDL2main
