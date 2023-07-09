#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef float       f32;
typedef double      f64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i64;
typedef size_t      usize;
typedef ssize_t     isize;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static u8 MAPDATA [8 * 8] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 3, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};



struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;
} state;

static void verline(int x, int y0, int y1, u32 color){
    for (int y = y0; y <= y1; y++){
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

static void render(){
    for (usize x = 0; x < SCREEN_WIDTH; x++){
        verline(x, 20, SCREEN_HEIGHT - 20, 0xFFFF0000 | (x & 0xFF));
    }
}

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

    state.texture = SDL_CreateTexture(state.renderer, 
                                      SDL_PIXELFORMAT_ABGR8888, 
                                      SDL_TEXTUREACCESS_STREAMING, 
                                      SCREEN_WIDTH, 
                                      SCREEN_HEIGHT);
    
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

        render();

        state.pixels[(10 * SCREEN_WIDTH + 5)] = 0xFFFF0FF;

        SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(state.renderer, state.texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyTexture(state.texture);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
    printf("SDL successfully quit.");
    return EXIT_SUCCESS;
}