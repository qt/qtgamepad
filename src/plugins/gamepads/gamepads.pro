TEMPLATE = subdirs
config_sdl:SUBDIRS += sdl2
!android: contains(QT_CONFIG, evdev): SUBDIRS += evdev
win32: !wince*: SUBDIRS += xinput
ios: SUBDIRS += ios
android: SUBDIRS += android
