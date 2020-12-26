#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "world.h"

void drawPixel(SDL_Renderer *r, int x, int y, int type)
{
    SDL_SetRenderDrawColor(r, attributes[type].colour[0], attributes[type].colour[1], attributes[type].colour[2], 255);
    SDL_RenderDrawPoint(r, x, y);
}

void initializeWorld(World *w)
{
    w->grid = malloc(w->width * sizeof(Pixel));

    for (int s = 0; s < w->width; s++)
    {
        w->grid[s] = malloc(w->height * sizeof(Pixel));
    }

    //make it a blank pixel
    for (int s = 0; s < w->width; s++)
    {
        for (int h = 0; h < w->height; h++)
        {
            // spawn walls around outside
            if (s == 0 || s == w->width - 1 || h == 0 || h == w->height - 1)
            {
                w->grid[s][h].type = wall;
                w->grid[s][h].updated = true;
                w->grid[s][h].density = attributes[wall].density;
                w->grid[s][h].x = s;
                w->grid[s][h].y = h;
            }
            else
            { // make the rest blank
                w->grid[s][h].type = blank;
                w->grid[s][h].updated = true;
                w->grid[s][h].density = attributes[blank].density;
                w->grid[s][h].x = s;
                w->grid[s][h].y = h;
            }
        }
    }

    //spawn initial collector
    for (int x = 0; x < 30; x++)
    {
        for (int y = 0; y < 30; y++)
        {
            w->grid[WORLD_WIDTH / 2 - 15 + x][WORLD_HEIGHT / 2 - 15 + y].type = 100;
            w->grid[WORLD_WIDTH / 2 - 15 + x][WORLD_HEIGHT / 2 - 15 + y].density = attributes[collector].density;
            w->grid[WORLD_WIDTH / 2 - 15 + x][WORLD_HEIGHT / 2 - 15 + y].updated = true;
        }
    }

    //intialize the powder bank
    for (int i = 0; i < sizeof(w->powderBank) / sizeof(int); i++)
    {
        w->powderBank[i] = 0;
    }
    // set the powders you start with
    w->powderBank[powder] = 10000;
    w->powderBank[water] = 10000;
    w->powderBank[fire] = 10000;
    w->powderBank[snow] = 10000;

    w->pixelCount = 0;
}

void drawWorld(World *w, SDL_Renderer *r)
{
    for (int x = 0; x < w->width; x++)
    {
        for (int y = 0; y < w->height; y++)
        {
            if (w->grid[x][y].type != blank)
            {
                drawPixel(r, x, y, w->grid[x][y].type);
                w->grid[x][y].updated = false;
            }
        }
    }
}

void updateWorld(World *w)
{
    for (int x = 0; x < w->width; x++)
    {
        for (int y = 0; y < w->height; y++)
        {
            if (!w->grid[x][y].updated)
            {
                switch (w->grid[x][y].type)
                {
                case powder:
                    // could put this in a move function maybe @
                    if (w->grid[x][y + 1].type == collector)
                    {
                        //collect the pixel
                        removePixel(w, x, y);
                        w->powderBank[powder] += 2;
                    }
                    if (w->grid[x][y + 1].density < attributes[powder].density)
                    { //check below
                        swapPixel(w, &w->grid[x][y], &w->grid[x][y + 1], x, y, x, y + 1);
                    }
                    else if (w->grid[x + 1][y + 1].density < attributes[powder].density)
                    { // check right/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y + 1], x, y, x + 1, y + 1);
                    }
                    else if (w->grid[x - 1][y + 1].density < attributes[powder].density)
                    { // check left/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x - 1][y + 1], x, y, x - 1, y + 1);
                    }
                    break;

                case water:
                {
                    if (w->grid[x][y + 1].type == collector)
                    {
                        //collect the pixel
                        removePixel(w, x, y);
                        w->powderBank[water] += 2; // ~ for debug, change to powder later
                    }
                    if (w->grid[x][y + 1].density < attributes[water].density)
                    { //check below
                        swapPixel(w, &w->grid[x][y], &w->grid[x][y + 1], x, y, x, y + 1);
                    }
                    else if (w->grid[x + 1][y + 1].density < attributes[water].density)
                    { // check right/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y + 1], x, y, x + 1, y + 1);
                    }
                    else if (w->grid[x - 1][y + 1].density < attributes[water].density)
                    { // check left/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x - 1][y + 1], x, y, x - 1, y + 1);
                    }
                    else
                    {
                        int r = rand() % (1 + 1 - 0) + 0; //randomize wether we check left or right first
                        if (r)
                        {
                            if (w->grid[x - 1][y].type == blank)
                            { // check left
                                swapPixel(w, &w->grid[x][y], &w->grid[x - 1][y], x, y, x - 1, y);
                            }
                            else if (w->grid[x + 1][y].type == blank)
                            { // check right
                                swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y], x, y, x + 1, y);
                            }
                            else if (w->grid[x + 1][y].type == snow && rand() % (1 - (-40) + 1) + (-40) > 0)
                            {
                                melt(w, x + 1, y);
                            }
                        }
                        else
                        {
                            if (w->grid[x + 1][y].type == blank)
                            { // check right
                                swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y], x, y, x + 1, y);
                            }
                            else if (w->grid[x - 1][y].type == 0)
                            { // check left
                                swapPixel(w, &w->grid[x][y], &w->grid[x - 1][y], x, y, x - 1, y);
                            }
                            else if (w->grid[x - 1][y].type == snow && (rand() % (1 - (-40) + 1) + (-40)) > 0)
                            {
                                melt(w, x - 1, y);
                            }
                        }
                    }
                    break;
                }
                case snow:
                {
                    int snowDir = rand() % (1 - 0 + 1) + 0; //0 to 1
                    int snowPause = rand() % (1 - (-1) + 1) + (-1);
                    // int snowPause = rand() % (1 - (-5) + 1) + (-5); //-5 to 1
                    int xDir;
                    int yDir;
                    if (snowPause > 0)
                    {
                        yDir = 1;
                    }
                    else
                    {
                        yDir = 0;
                    }
                    if (snowDir)
                    {
                        xDir = 1;
                    }
                    else
                    {
                        xDir = -1;
                    }
                    if (w->grid[x + xDir][y + yDir].type == blank && w->grid[x][y + 1].type == blank)
                    {
                        swapPixel(w, &w->grid[x][y], &w->grid[x + xDir][y + yDir], x, y, x + xDir, y + yDir);
                    }
                    else if (w->grid[x][y + 1].type != blank)
                    {
                        if (w->grid[x][y + 1].type == water || w->grid[x][y + 1].type == fire)
                        {
                            int willMelt = rand() % (1 - (-60) + 1) + (-60);
                            if (willMelt > 0)
                            {
                                melt(w, x, y);
                            }
                        }
                        if (w->grid[x + 1][y + 1].density == blank)
                        { // check right/down
                            swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y + 1], x, y, x + 1, y + 1);
                        }
                        else if (w->grid[x - 1][y + 1].density == blank)
                        { // check left/down
                            swapPixel(w, &w->grid[x][y], &w->grid[x - 1][y + 1], x, y, x - 1, y + 1);
                        }
                    }
                    break;
                }
                case fire:
                {
                    int rx = rand() % (1 - (-1) + 1) + (-1); //-1 to 1
                    int ry = rand() % (1 - (-1) + 1) + (-1); //-1 to 1

                    if (w->grid[x + rx][y + ry].type == blank)
                    {
                        w->grid[x][y].life -= 1;
                        if (w->grid[x][y].life <= 0)
                        {
                            removePixel(w, x, y);
                        }
                        swapPixel(w, &w->grid[x][y], &w->grid[x + rx][y + ry], x, y, (x + rx), (y + ry));
                    }
                    else if (w->grid[x + rx][y + ry].flammable == burns)
                    {
                        w->grid[x][y].life = attributes[fire].life;
                        removePixel(w, x + rx, y + ry);
                        // w->grid[x + rx][y + ry].type = blank;
                        addPixel(w, 0, x + rx, y + ry, fire);
                        swapPixel(w, &w->grid[x][y], &w->grid[x + rx][y + ry], x, y, x + rx, y + ry);
                    }
                    else if (w->grid[x + rx][y + ry].type == water)
                    {
                        removePixel(w, x, y);
                    }
                    else if (w->grid[x + rx][y + ry].flammable == melts)
                    {
                        melt(w, x + rx, y + ry);
                    }
                }

                break;
                }
            }

            w->grid[x][y].updated = true;
        }
    }
}