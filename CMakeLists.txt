# Generated from qtgamepad.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtGamepad
    VERSION 6.0.0
    DESCRIPTION "Qt Gamepad Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Gui Widgets)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Quick QuickControls2 DeviceDiscoverySupport)
# special case end

if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
qt_build_repo()
