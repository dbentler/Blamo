#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

const int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    bool quit;
} state;

int main(int argc, char*argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    state.window = SDL_CreateWindow("Blamo", 
                                    SDL_WINDOWPOS_CENTERED_DISPLAY(1), 
                                    SDL_WINDOWPOS_CENTERED_DISPLAY(1), 
                                    SCREEN_WIDTH, 
                                    SCREEN_HEIGHT, 
                                    SDL_WINDOW_ALLOW_HIGHDPI);

    if(NULL == state.window)
    {
        printf("Could not create window:\n %s", SDL_GetError() );
        return 1;
    }

    state.renderer = SDL_CreateRenderer(state.window, 
                                        -1, 
                                        SDL_RENDERER_PRESENTVSYNC);

    state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    if(state.texture == NULL){
        printf("Failed to create SDL texture:\n %s", SDL_GetError());
    }

    while(!state.quit)
    {
        SDL_Event windowEvent;
        if (SDL_PollEvent(&windowEvent))
        {
            if(SDL_QUIT == windowEvent.type)
            { break; }
        }
        SDL_SetRenderDrawColor(state.renderer, 0xFF, 0x00, 0xFF, 0xFF);
        SDL_RenderClear(state.renderer);
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyWindow(state.window);
    SDL_Quit();
    printf("SDL successfully quit.");
    return EXIT_SUCCESS;
}