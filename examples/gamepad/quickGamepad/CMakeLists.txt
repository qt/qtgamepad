# Generated from quickGamepad.pro.

cmake_minimum_required(VERSION 3.14)
project(quickGamepad LANGUAGES CXX)

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

add_qt_gui_executable(quickGamepad
    main.cpp
)
target_link_libraries(quickGamepad PUBLIC
    Qt::Core
    Qt::Gamepad
    Qt::Gui
    Qt::Qml
    Qt::Quick
)

# Resources:
set(qml_resource_files
    "qml/ButtonImage.qml"
    "qml/DPad.qml"
    "qml/JoystickViewer.qml"
    "qml/LeftThumbstick.qml"
    "qml/RightThumbstick.qml"
    "qml/main.qml"
    "qml/xboxControllerBack.png"
    "qml/xboxControllerButtonA.png"
    "qml/xboxControllerButtonB.png"
    "qml/xboxControllerButtonGuide.png"
    "qml/xboxControllerButtonX.png"
    "qml/xboxControllerButtonY.png"
    "qml/xboxControllerDPad.png"
    "qml/xboxControllerLeftShoulder.png"
    "qml/xboxControllerLeftThumbstick.png"
    "qml/xboxControllerLeftTrigger.png"
    "qml/xboxControllerRightShoulder.png"
    "qml/xboxControllerRightThumbstick.png"
    "qml/xboxControllerRightTrigger.png"
    "qml/xboxControllerStart.png"
)

qt6_add_resources(quickGamepad "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


install(TARGETS quickGamepad
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
