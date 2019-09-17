# Generated from src.pro.

add_subdirectory(gamepad)
add_subdirectory(plugins)

# special case begin
if (TARGET Qt::Quick)
    add_subdirectory(imports)
endif()
# special case end
