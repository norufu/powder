#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "world.h"
#include "pixel.h"
#include "selector.h"
#include "menu.h"

int main(void)
{
    initializeAttributes();
    World world;
    Selector sel;
    Menu menu;
    world.width = WORLD_WIDTH;
    world.height = WORLD_HEIGHT;
    initializeWorld(&world);

    sel.type = powder;
    int penSize = 0;

    initializeMenu(&menu);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event e;
    bool mouseDown = 0;
    bool quit = 0;
    int mx, my;

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return (1);
    }

    // Create the window where we will draw.
    window = SDL_CreateWindow("p",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              DISPLAY_WIDTH, DISPLAY_HEIGHT,
                              0);

    // We must call SDL_CreateRenderer in order for draw calls to affect this window.
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    /* Create texture for display */
    SDL_Texture *display = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    SDL_SetRenderTarget(renderer, display);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    while (!quit)
    {
        SDL_SetRenderTarget(renderer, display);

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_1)
                {
                    printf("selector is now powder\n");
                    newSelection(&sel, powder);
                }
                else if (e.key.keysym.sym == SDLK_2)
                {
                    printf("selector is now water\n");
                    newSelection(&sel, water);
                }
                else if (e.key.keysym.sym == SDLK_SPACE)
                {
                    for (int i = 0; i < world.width; i++)
                    {
                        for (int h = world.height; h > 500; h--)
                        {
                            printf("%d ", world.grid[i][h].type);
                        }
                        printf("\n");
                    }
                    sleep(30);
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                mx = 0;
                my = 0;
                SDL_GetMouseState(&mx, &my);
                if (my < 512)
                { // click on board
                    mouseDown = 1;
                }
                else if (my > 512)
                { //click on menu
                    PixelAttributes buttonClicked;
                    buttonClicked = checkMenu(mx, my, &menu);
                    if (buttonClicked.type != 0)
                    {
                        sel.type = buttonClicked.type;
                        printf("%d\n", sel.type);
                    }
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                mouseDown = 0;
            }
        }

        updateWorld(&world);

        if (mouseDown)
        {
            addPixel(&world, penSize, e.button.x, e.button.y, sel.type);
        }

        SDL_SetRenderTarget(renderer, NULL);
        // SDL_RenderCopy(renderer, display, NULL, NULL);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        drawWorld(&world, renderer);
        drawMenu(renderer, &menu);
        SDL_RenderPresent(renderer);
        // sleep(1);
    }

    //cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}