# Generated from gamepad.pro.

add_subdirectory(simple)
if(TARGET Qt::Quick)
    add_subdirectory(mouseItem)
    add_subdirectory(keyNavigation)
endif()
if(TARGET Qt::Quick AND TARGET Qt::QuickControls2)
    add_subdirectory(configureButtons)
    add_subdirectory(quickGamepad)
endif()
