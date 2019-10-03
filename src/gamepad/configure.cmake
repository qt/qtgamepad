

#### Inputs



#### Libraries

qt_find_package(WrapSDL2 PROVIDED_TARGETS WrapSDL2::WrapSDL2)


#### Tests



#### Features

qt_feature("sdl2" PRIVATE
    LABEL "SDL2"
    CONDITION WrapSDL2_FOUND
)
