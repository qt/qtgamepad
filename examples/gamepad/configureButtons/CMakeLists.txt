# Generated from configureButtons.pro.

cmake_minimum_required(VERSION 3.14)
project(configureButtons LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Gamepad)

add_qt_gui_executable(configureButtons
    main.cpp
)
target_link_libraries(configureButtons PUBLIC
    Qt::Core
    Qt::Gamepad
    Qt::Gui
    Qt::Qml
    Qt::Quick
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt6_add_resources(configureButtons "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


install(TARGETS configureButtons
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
