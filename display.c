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
    int penSize = 1;

    initializeMenu(&menu);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event e;
    bool mouseDown = false;
    bool midMouseDown = false;
    bool quit = false;
    int mx, my;
    int oldmx, oldmy;
    int zoomAdjustX, zoomAdjustY;

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
                    world.zoom = 1;
                }
                else if (e.key.keysym.sym == SDLK_2)
                {
                    mx = 0;
                    my = 0;
                    SDL_GetMouseState(&mx, &my);
                    world.zoom = 2;
                    adjustZoom(&world, mx, my);

                    // printf("zoomx and y %d %d\n", world.zoomX, world.zoomY);
                }
                else if (e.key.keysym.sym == SDLK_UP)
                {
                    penSize > 5 ? 5 : penSize++;
                }
                else if (e.key.keysym.sym == SDLK_DOWN)
                {
                    penSize == 1 ? 1 : penSize--;
                }
                else if (e.key.keysym.sym == SDLK_SPACE) // debug, prints types for the grid
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
                else if (e.key.keysym.sym == SDLK_8)
                {
                    world.grid[5][WORLD_HEIGHT - 2].vx = 20;
                    world.grid[5][WORLD_HEIGHT - 2].vy = -10;
                }
                else if (e.key.keysym.sym == SDLK_9)
                {
                    world.heatDebugOn = world.heatDebugOn ? false : true;
                    world.windDebugOn = world.windDebugOn ? false : true;
                }
                else if (e.key.keysym.sym == SDLK_p)
                {
                    world.paused = world.paused ? false : true;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                mx = 0;
                my = 0;
                SDL_GetMouseState(&mx, &my);
                if (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT)
                {
                    if (my < world.height)
                    { // click on board
                        mouseDown = true;
                    }
                    else if (my > world.height)
                    { //click on menu
                        mouseDown = false;
                        PixelAttributes buttonClicked;
                        buttonClicked = checkMenu(mx, my, &menu);
                        if (buttonClicked.type != blank)
                        {
                            sel.type = buttonClicked.type;
                            printf("%d\n", sel.type);
                        }
                    }
                }
                else if (e.button.button == SDL_BUTTON_MIDDLE && world.zoom > 1)
                {
                    midMouseDown = true;
                    zoomAdjustX = mx;
                    zoomAdjustY = my;
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                mouseDown = false;
                midMouseDown = false;
            }
        }

        if (!world.paused)
            updateWorld(&world);

        if (mouseDown) // if clicking, add pixels
        {
            if (oldmx - e.button.x != 0 || oldmy - e.button.y != 0) //avoid both being 0
            {
                world.mxDir = oldmx - e.button.x; //for fan direction
                world.myDir = oldmy - e.button.y;
            }

            if (world.zoom == 1)
            {
                addPixelRadius(&world, e.button.x, e.button.y, 0, penSize, sel.type); //penSize is radius of the area to draw
            }
            else if (world.zoom == 2)
            {
                addPixelRadius(&world, (e.button.x / 2) + world.zoomX, (e.button.y / 2) + world.zoomY, 0, penSize, sel.type);
            }
        }
        else if (midMouseDown) // if holding mid mouse button, adjust zoom by change in mouse
        {
            SDL_GetMouseState(&mx, &my);
            printf("zoom %d\n", zoomAdjustX - mx);
            adjustZoom(&world, (world.zoomX + world.width / 4) - (zoomAdjustX - mx), (world.zoomY + world.height / 4) - (zoomAdjustY - my)); // adds size/4 to counter the - in adjustzoom function
            zoomAdjustX = mx;
            zoomAdjustY = my;
        }
        SDL_GetMouseState(&oldmx, &oldmy);

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