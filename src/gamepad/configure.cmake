

#### Inputs



#### Libraries

qt_find_package(SDL2 PROVIDED_TARGETS SDL2::SDL2)


#### Tests



#### Features

qt_feature("sdl2" PRIVATE
    LABEL "SDL2"
    CONDITION SDL2_FOUND
)
