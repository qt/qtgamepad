# Generated from jar.pro.

#####################################################################
## QtAndroidGamepad Binary:
#####################################################################

# special case begin

set(path_prefix ${CMAKE_CURRENT_SOURCE_DIR}/src/org/qtproject/qt5/android/)

set(java_sources
    ${path_prefix}/gamepad/QtGamepad.java)

add_jar(QtAndroidGamepad
    INCLUDE_JARS ${QT_ANDROID_JAR}
    SOURCES ${java_sources}
    )

install_jar(QtAndroid
    DESTINATION jar
    COMPONENT Devel)

# special case end

#### Keys ignored in scope 1:.:.:jar.pro:<TRUE>:
# API_VERSION = "android-16"
# JAVACLASSPATH = "$$PWD/src"
# JAVASOURCES = "$$PWD/src/org/qtproject/qt5/android/gamepad/QtGamepad.java"
# OTHER_FILES = "$$JAVASOURCES"
# _LOADED = "qt_build_paths"
