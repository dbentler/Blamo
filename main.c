#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

typedef float       f32;
typedef double      f64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef size_t      usize;
typedef ssize_t     isize;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static u8 MAPDATA [8 * 8] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 4, 0, 0, 4, 0, 1,
    1, 0, 0, 0, 0, 4, 0, 1,
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

f32 playerX = 4.5f;
f32 playerY = 4.5f;
f32 playerDirX = -1.0f;
f32 playerDirY = 0.0f;
f32 playerPlaneX = 0.0f;
f32 playerPlaneY = 0.66f;
f32 moveSpeed = 0.05f;

f32 prevDirX;
f32 prevPlaneX;

static void verline(int x, int y0, int y1, u32 color) {
    for (int y = y0; y <= y1; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

static void render() {
    for (usize x = 0; x < SCREEN_WIDTH; x++) {
        // Calculate the ray direction and initial position
        f32 cameraX = 2 * x / (f32)SCREEN_WIDTH - 1;
        f32 rayDirX = playerDirX + playerPlaneX * cameraX;
        f32 rayDirY = playerDirY + playerPlaneY * cameraX;

        // Map position
        usize mapX = (usize)playerX;
        usize mapY = (usize)playerY;

        // Length of ray from the current position to the next x or y side
        f32 sideDistX;
        f32 sideDistY;

        // Length of ray from one side to the next in the x or y direction
        f32 deltaDistX = fabsf(1 / rayDirX);
        f32 deltaDistY = fabsf(1 / rayDirY);

        // The direction to step in x and y (either +1 or -1)
        isize stepX;
        isize stepY;

        // Used to determine whether the ray hit a horizontal or vertical wall
        isize side = 0;

        // Calculate step and initial sideDist values
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (playerX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - playerX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (playerY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - playerY) * deltaDistY;
        }

        // Perform DDA (Digital Differential Analysis) algorithm
        while (true) {
            // Jump to next map square in x or y direction
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            // Check if the ray hit a wall
            if (MAPDATA[mapY * 8 + mapX] > 0) {
                break;
            }
        }

        // Calculate distance projected on camera direction
        f32 perpWallDist;
        if (side == 0) {
            perpWallDist = (mapX - playerX + (1 - stepX) / 2) / rayDirX;
        } else {
            perpWallDist = (mapY - playerY + (1 - stepY) / 2) / rayDirY;
        }

        // Calculate height of the line to draw on the screen
        usize lineHeight = (usize)(SCREEN_HEIGHT / perpWallDist);

        // Calculate lowest and highest pixel to fill in current stripe
        isize drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) {
            drawStart = 0;
        }
        isize drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) {
            drawEnd = SCREEN_HEIGHT - 1;
        }

        // Choose the wall color based on the tile value
        u32 wallColor;
        switch (MAPDATA[mapY * 8 + mapX]) {
            case 1: // Red walls
                wallColor = 0xFFAA0000;
                break;
            case 3: // Blue walls
                wallColor = 0xFF0000AA;
                break;
            case 4: // Green walls
                wallColor = 0xFF00AA00;
                break;
            default: // Empty spaces or other values
                wallColor = 0xFFFFFFFF;
                break;
        }

        // Draw the ceiling
        verline(x, 0, drawStart - 1, 0xFFCCCCCC);

        // Draw the wall slice
        verline(x, drawStart, drawEnd, wallColor);

        // Draw the floor
        verline(x, drawEnd + 1, SCREEN_HEIGHT - 1, 0xFF222222);
    }
}

int main(int argc, char*argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    state.window = SDL_CreateWindow("Blamo",
                                    SDL_WINDOWPOS_CENTERED_DISPLAY(1),
                                    SDL_WINDOWPOS_CENTERED_DISPLAY(1),
                                    SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    SDL_WINDOW_ALLOW_HIGHDPI);

    if (NULL == state.window) {
        printf("Could not create window:\n %s", SDL_GetError());
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

    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (state.texture == NULL) {
        printf("Failed to create SDL texture:\n %s", SDL_GetError());
    }

    int prevMouseX = 0;
    int prevMouseY = 0;

    while (!state.quit) {
        SDL_RaiseWindow(state.window);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                state.quit = true;
            }
            if (event.type == SDL_MOUSEMOTION) {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;

                // Calculate the difference in mouse position
                int deltaX = mouseX - prevMouseX;

                // Update the previous mouse position
                prevMouseX = mouseX;

                // Calculate the rotation speed
                float rotationSpeed = 0.002f;

                // Update camera direction and plane based on mouse movement
                float prevDirX = playerDirX;
                playerDirX = playerDirX * cosf(-deltaX * rotationSpeed) - playerDirY * sinf(-deltaX * rotationSpeed);
                playerDirY = prevDirX * sinf(-deltaX * rotationSpeed) + playerDirY * cosf(-deltaX * rotationSpeed);
                float prevPlaneX = playerPlaneX;
                playerPlaneX = playerPlaneX * cosf(-deltaX * rotationSpeed) - playerPlaneY * sinf(-deltaX * rotationSpeed);
                playerPlaneY = prevPlaneX * sinf(-deltaX * rotationSpeed) + playerPlaneY * cosf(-deltaX * rotationSpeed);
            }

            if (event.type == SDL_KEYDOWN) {
                // Handle camera movement
                switch (event.key.keysym.sym) {
                    case SDLK_w: // Move forward
                        {
                            f32 nextX = playerX + playerDirX * moveSpeed;
                            f32 nextY = playerY + playerDirY * moveSpeed;
                            usize mapIndexX = (usize)nextX;
                            usize mapIndexY = (usize)nextY;

                            // Check if the next position is within a walkable area
                            if (MAPDATA[mapIndexY * 8 + mapIndexX] == 0 &&
                                MAPDATA[mapIndexY * 8 + (usize)(playerX + playerDirX * 0.1f)] == 0 &&
                                MAPDATA[(usize)(playerY + playerDirY * 0.1f) * 8 + mapIndexX] == 0) {
                                playerX = nextX;
                                playerY = nextY;
                            }
                        }
                        break;
                    case SDLK_s: // Move backward
                        {
                            f32 nextX = playerX - playerDirX * moveSpeed;
                            f32 nextY = playerY - playerDirY * moveSpeed;
                            usize mapIndexX = (usize)nextX;
                            usize mapIndexY = (usize)nextY;

                            // Check if the next position is within a walkable area
                            if (MAPDATA[mapIndexY * 8 + mapIndexX] == 0 &&
                                MAPDATA[mapIndexY * 8 + (usize)(playerX - playerDirX * 0.1f)] == 0 &&
                                MAPDATA[(usize)(playerY - playerDirY * 0.1f) * 8 + mapIndexX] == 0) {
                                playerX = nextX;
                                playerY = nextY;
                            }
                        }
                        break;
                    case SDLK_a: // Strafe left
                        {
                            f32 nextX = playerX - playerPlaneX * moveSpeed;
                            f32 nextY = playerY - playerPlaneY * moveSpeed;
                            usize mapIndexX = (usize)nextX;
                            usize mapIndexY = (usize)nextY;

                            // Check if the next position is within a walkable area
                            if (MAPDATA[mapIndexY * 8 + mapIndexX] == 0 &&
                                MAPDATA[mapIndexY * 8 + (usize)(playerX - playerPlaneX * 0.1f)] == 0 &&
                                MAPDATA[(usize)(playerY - playerPlaneY * 0.1f) * 8 + mapIndexX] == 0) {
                                playerX = nextX;
                                playerY = nextY;
                            }
                        }
                        break;
                    case SDLK_d: // Strafe right
                        {
                            f32 nextX = playerX + playerPlaneX * moveSpeed;
                            f32 nextY = playerY + playerPlaneY * moveSpeed;
                            usize mapIndexX = (usize)nextX;
                            usize mapIndexY = (usize)nextY;

                            // Check if the next position is within a walkable area
                            if (MAPDATA[mapIndexY * 8 + mapIndexX] == 0 &&
                                MAPDATA[mapIndexY * 8 + (usize)(playerX + playerPlaneX * 0.1f)] == 0 &&
                                MAPDATA[(usize)(playerY + playerPlaneY * 0.1f) * 8 + mapIndexX] == 0) {
                                playerX = nextX;
                                playerY = nextY;
                            }
                        }
                    break;
                }

            }
        }

        render();

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