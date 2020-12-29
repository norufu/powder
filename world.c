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

void drawHeat(SDL_Renderer *r, int x, int y, int heat)
{
    SDL_SetRenderDrawColor(r, 100 + (heat * 2), 100, 100, 255);
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
                initializeWall(w, s, h, wall);
            }
            else
            { // make the rest blank
                initializeWall(w, s, h, blank);
            }
        }
    }

    //spawn initial collector
    for (int x = 0; x < 30; x++)
    {
        for (int y = 0; y < 30; y++)
        {
            changePixel(w, WORLD_WIDTH / 2 - 15 + x, WORLD_HEIGHT / 2 - 15 + y, collector);
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
    w->powderBank[coal] = 10000;
    w->powderBank[oil] = 10000;
    w->pixelCount = 0;
    w->zoom = 1;
}

void drawWorld(World *w, SDL_Renderer *r)
{
    // int offset = 0;
    if (w->zoom == 1)
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
                // if (w->grid[x][y].heat > 0) // for debug
                //     drawHeat(r, x, y, w->grid[x][y].heat);
            }
        }
    }
    else if (w->zoom == 2)
    {
        int drawX = 0;
        int drawY = 0;
        for (int x = w->zoomX; x < w->zoomX + w->width / 2; x++)
        {
            for (int y = w->zoomY; y < w->zoomY + w->height / 2; y++)
            {
                if (w->grid[x][y].type != blank)
                {
                    drawPixel(r, drawX * 2, drawY * 2, w->grid[x][y].type);
                    drawPixel(r, drawX * 2 + 1, drawY * 2, w->grid[x][y].type);
                    drawPixel(r, drawX * 2, drawY * 2 + 1, w->grid[x][y].type);
                    drawPixel(r, drawX * 2 + 1, drawY * 2 + 1, w->grid[x][y].type);
                }
                // if (w->grid[x][y].heat > 0)  //for debug
                // {
                //     drawHeat(r, drawX * 2, drawY * 2, w->grid[x][y].heat);
                //     drawHeat(r, drawX * 2 + 1, drawY * 2, w->grid[x][y].heat);
                //     drawHeat(r, drawX * 2, drawY * 2 + 1, w->grid[x][y].heat);
                //     drawHeat(r, drawX * 2 + 1, drawY * 2 + 1, w->grid[x][y].heat);
                // }

                drawY++;
            }
            drawY = 0;
            drawX++;
        }
        for (int x = 0; x < w->width; x++) //need this so it updates off screen as well, probably a better way to do this
        {
            for (int y = 0; y < w->height; y++)
            {
                w->grid[x][y].updated = false;
            }
        }
    }
}

void updateWorld(World *w) // @ I need to tick everything forward then apply the updates to the world, as is now oil spreads to the right instantly butslowly to the left
{
    for (int x = 0; x < w->width; x++)
    {
        for (int y = 0; y < w->height; y++)
        {
            if (w->grid[x][y].life < 1)
            {
                removePixel(w, x, y);
            }
            else if (!w->grid[x][y].updated)
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
                    if (w->grid[x][y].heat > attributes[w->grid[x][y].type].changeHeat)
                    {
                        light(w, x, y, 0);
                    }
                    if (w->grid[x][y].burning)
                    {
                        burn(w, x, y, 0);
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
                case coal:
                {
                    if (w->grid[x][y + 1].type == collector)
                    {
                        //collect the pixel
                        removePixel(w, x, y);
                        w->powderBank[powder] += 2;
                    }
                    if (w->grid[x][y + 1].density < attributes[coal].density)
                    { //check below
                        swapPixel(w, &w->grid[x][y], &w->grid[x][y + 1], x, y, x, y + 1);
                    }
                    else if (w->grid[x + 1][y + 1].density < attributes[coal].density)
                    { // check right/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y + 1], x, y, x + 1, y + 1);
                    }
                    else if (w->grid[x - 1][y + 1].density < attributes[coal].density)
                    { // check left/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x - 1][y + 1], x, y, x - 1, y + 1);
                    }
                    break;
                }
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
                                melt(w, x + 1, y, 0);
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
                                melt(w, x - 1, y, 0);
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
                    if (w->grid[x][y].heat > attributes[w->grid[x][y].type].changeHeat)
                    {
                        melt(w, x, y, 0);
                        break;
                    }
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
                        if (w->grid[x][y + 1].type == water)
                        {
                            int willMelt = rand() % (1 - (-60) + 1) + (-60);
                            if (willMelt > 0)
                            {
                                melt(w, x, y, 0);
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
                case oil:
                {
                    if (w->grid[x][y + 1].type == collector)
                    {
                        //collect the pixel
                        removePixel(w, x, y);
                        w->powderBank[oil] += 2; // ~ for debug, change to powder later
                    }

                    if (w->grid[x][y].heat > attributes[w->grid[x][y].type].changeHeat)
                    {
                        light(w, x, y, 0);
                    }
                    if (w->grid[x][y].burning)
                    {
                        // doToNeighbours(w, x, y, &light);
                        burn(w, x, y, 0);
                    }

                    if (w->grid[x][y + 1].density < attributes[oil].density)
                    { //check below
                        swapPixel(w, &w->grid[x][y], &w->grid[x][y + 1], x, y, x, y + 1);
                    }
                    else if (w->grid[x + 1][y + 1].density < attributes[oil].density)
                    { // check right/down
                        swapPixel(w, &w->grid[x][y], &w->grid[x + 1][y + 1], x, y, x + 1, y + 1);
                    }
                    else if (w->grid[x - 1][y + 1].density < attributes[oil].density)
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
                        // doToNeighbours(w, x + rx, y + ry, &light);
                        doToRadius(w, x + rx, y + ry, 0, HEAT_RADIUS, &heat);
                        break;
                    }
                    else if (w->grid[x + rx][y + ry].type == water) // put out fire
                    {
                        removePixel(w, x, y);
                        break;
                    }
                    doToRadius(w, x + rx, y + ry, 0, HEAT_RADIUS, &heat);

                    // else if (w->grid[x + rx][y + ry].flammable == melts)
                    // {
                    //     light(w, x + rx, y + ry);
                    // }
                    // else if (w->grid[x + rx][y + ry].flammable == burns)
                    // {
                    //     w->grid[x][y].life = attributes[fire].life;
                    //     // removePixel(w, x + rx, y + ry);
                    //     // w->grid[x + rx][y + ry].type = blank;
                    //     // addPixel(w, 0, x + rx, y + ry, fire);
                    //     // swapPixel(w, &w->grid[x][y], &w->grid[x + rx][y + ry], x, y, x + rx, y + ry);
                    //     light(w, x + rx, y + ry);
                    // }
                }

                break;
                }
            }
            w->grid[x][y].updated = true;
            if (!w->grid[x][y].burning && w->grid[x][y].heat > 0)
                w->grid[x][y].heat -= 1;
        }
    }
}

void adjustZoom(World *w, int x, int y)
{
    w->zoomX = x - w->width / 4;
    w->zoomY = y - w->height / 4;
    if (w->zoomX < 0)
    {
        w->zoomX = 0;
    }
    else if ((w->zoomX + w->width / 2) > w->width)
    {
        w->zoomX = w->width - w->width / 2;
    }
    if (w->zoomY < 0)
    {
        w->zoomY = 0;
    }
    else if ((w->zoomY + w->height / 2) > w->height)
    {
        w->zoomY = w->height - w->height / 2;
    }
}