#include <SDL.h>
#include <SDL_gamecontroller.h>

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    SDL_Quit();
    return 0;
}
