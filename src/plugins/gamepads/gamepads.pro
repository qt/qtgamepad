TEMPLATE = subdirs
config_sdl:SUBDIRS += sdl2
contains(QT_CONFIG, evdev): SUBDIRS += evdev
win32: !wince*: SUBDIRS += xinput
ios: SUBDIRS += ios
