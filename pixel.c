#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "world.h"

int heatDist[HEAT_RADIUS] = {4, 4, 3, 2, 1};

char boundsOk(World *w, int x, int y)
{
    if (x <= 0 || x >= w->width)
    {
        return (0);
    }
    else if (y <= 0 || y >= w->height)
    {
        return (0);
    }
    return (1); //within bounds
}
void setAttributes(int arr[])
{
    printf("check arr%d\n", arr[0]);
    attributes[arr[0]].type = arr[0];
    attributes[arr[0]].density = arr[1];
    attributes[arr[0]].colour[0] = arr[2]; //r
    attributes[arr[0]].colour[1] = arr[3]; //g
    attributes[arr[0]].colour[2] = arr[4]; //b
    attributes[arr[0]].life = arr[5];
    attributes[arr[0]].flammable = arr[6];
    attributes[arr[0]].changeHeat = arr[7];
}
void initializeAttributes() //will need to load these from a file maybe
{
    FILE *fd;
    fd = fopen("pixelData.dat", "r");
    if (!fd)
    {
        printf("ERROR: Could not open file\n");
        exit(1);
    }
    char c;
    int lineCount = 0;
    int atrCount = 8; // number of attributes per pixel

    //get line count & rewind
    for (c = getc(fd); c != EOF; c = getc(fd))
        if (c == '\n')
            lineCount += 1;
    rewind(fd);

    int atrArray[atrCount];
    for (int i = 0; i < lineCount; i++)
    {
        for (int p = 0; p < atrCount; p++)
        {
            fscanf(fd, "%d", &atrArray[p]);
        }
        setAttributes(atrArray);
    }

    int fclose(FILE * fd);
}

void initializeWall(World *w, int x, int y, char newType)
{
    if (x < 0 || x >= w->width)
    {
        return;
    }
    else if (y < 0 || y >= w->height)
    {
        return;
    }
    // printf("%d  \n", newType);
    w->grid[x][y].type = newType;
    w->grid[x][y].density = attributes[newType].density;
    w->grid[x][y].life = attributes[newType].life;
    w->grid[x][y].flammable = attributes[newType].flammable;
    w->grid[x][y].burning = false;
}

void changePixel(World *w, int x, int y, char newType)
{
    if (x <= 0 || x >= w->width - 1) // check bounds
    {
        return;
    }
    else if (y <= 0 || y >= w->height - 1)
    {
        return;
    }

    w->grid[x][y].type = newType;
    w->grid[x][y].density = attributes[newType].density;
    w->grid[x][y].life = attributes[newType].life;
    w->grid[x][y].flammable = attributes[newType].flammable;
    w->grid[x][y].burning = false;
    w->grid[x][y].vx = 0;
    w->grid[x][y].vy = 0;

    if (newType == fan)
    {
        if (w->mxDir == 0)
            w->grid[x][y].vx = 0;
        else
            w->grid[x][y].vx = w->mxDir > 0 ? 4 : -4;
        if (w->myDir == 0)
            w->grid[x][y].vy = 0;
        else
            w->grid[x][y].vy = w->myDir > 0 ? 4 : -4;
    }
}

void addPixel(World *w, int x, int y, char pixType)
{
    // printf("%d %d\n", x, y);
    if (w->grid[x][y].type != blank)
        return;

    changePixel(w, x, y, pixType);
    w->powderBank[pixType] -= 1;
    w->pixelCount += 1;
}

void removePixel(World *w, int x, int y)
{
    w->pixelCount--;
    w->grid[x][y].type = blank;
    w->grid[x][y].density = attributes[blank].density;
    w->grid[x][y].updated = true;
    w->grid[x][y].life = attributes[blank].life;
    w->grid[x][y].flammable = attributes[blank].flammable;
    w->grid[x][y].burning = false;
}

void swapPixel(World *w, Pixel *p1, Pixel *p2, int x1, int y1, int x2, int y2)
{
    Pixel *temp1 = malloc(sizeof(Pixel)); //@ i don't need 2 temps
    *temp1 = *p1;
    Pixel *temp2 = malloc(sizeof(Pixel));

    // if (w->grid[x2][y2].type == blank)
    // { //if there's wind velocity apply it to the pixel moving into that space
    //     temp1->vx = w->grid[x2][y2].vx;
    //     temp1->vy = w->grid[x2][y2].vy;
    //     p2->vx = 0;
    //     p2->vy = 0;
    // }
    p2->vx = temp1->vx;
    p2->vy = temp1->vy;
    w->grid[x1][y1] = *p2;
    w->grid[x2][y2] = *temp1;
    free(temp1);

    w->grid[x2][y2].updated = true;
    w->grid[x1][y1].updated = true;
}

void doToNeighbours(World *w, int x, int y, void (*func)(World *, int, int))
{
    func(w, x + 1, y);
    func(w, x - 1, y);
    func(w, x, y + 1);
    func(w, x, y - 1);
    func(w, x + 1, y + 1);
    func(w, x - 1, y + 1);
    func(w, x + 1, y - 1);
    func(w, x - 1, y - 1);
}

char checkNeighbour(World *w, int x, int y, char dir) // dir is fighting game notation, number refers to direction on numpad eg 2 = up
{
    switch (dir)
    {
    case 1:
        return (w->grid[x - 1][y + 1].type); // up left
        break;
    case 2:
        return (w->grid[x][y + 1].type); // up
        break;
    case 3:
        return (w->grid[x + 1][y + 1].type); // up right
        break;
    case 4:
        return (w->grid[x - 1][y].type); // left
        break;
    case 6:
        return (w->grid[x + 1][y].type); // right
        break;
    case 7:
        return (w->grid[x - 1][y - 1].type); // down left
        break;
    case 8:
        return (w->grid[x][y - 1].type); // down
        break;
    case 9:
        return (w->grid[x + 1][y - 1].type); // down right
        break;
    }
    return (0);
}

void addPixelRadius(World *w, int x, int y, int tick, int r, int pixType)
{
    if (tick >= r)
    {
        return;
    }
    int frame = tick;
    int diag = frame >= 2 ? frame - 1 : 0;
    int shift = frame > 0 ? 1 : 0;

    int cx = x;
    int cy = y;

    //bottom right
    cx = cx + frame;

    addPixel(w, cx, cy, pixType);
    cy = cy + shift;
    addPixel(w, cx, cy, pixType);
    for (int i = 0; i < diag; i++)
    {
        cx = cx - 1;
        cy = cy + 1;
        addPixel(w, cx, cy, pixType);
    }

    // bottom left
    cx = cx - shift;
    addPixel(w, cx, cy, pixType);
    cx = cx - shift;
    addPixel(w, cx, cy, pixType);
    for (int i = 0; i < diag; i++)
    {
        cx = cx - 1;
        cy = cy - 1;
        addPixel(w, cx, cy, pixType);
    }

    //upper left
    cy = cy - shift;
    addPixel(w, cx, cy, pixType);
    cy = cy - shift;
    addPixel(w, cx, cy, pixType);
    for (int i = 0; i < diag; i++)
    {
        cx = cx + 1;
        cy = cy - 1;
        addPixel(w, cx, cy, pixType);
    }

    //top right
    cx = cx + shift;
    addPixel(w, cx, cy, pixType);
    cx = cx + shift;
    addPixel(w, cx, cy, pixType);
    for (int i = 0; i < diag; i++)
    {
        cx = cx + 1;
        cy = cy + 1;
        addPixel(w, cx, cy, pixType);
    }

    addPixelRadius(w, x, y, frame + 1, r, pixType);
}

void doToRadius(World *w, int x, int y, int tick, int r, void (*func)(World *, int, int, char)) //currently becomes more square past 5 radius, might need to find a more circular way
{
    if (tick >= r)
    {
        return;
    }
    int frame = tick;
    int diag = frame >= 2 ? frame - 1 : 0;
    int shift = frame > 0 ? 1 : 0;

    int cx = x;
    int cy = y;

    //bottom right
    cx = cx + frame;

    func(w, cx, cy, frame);
    cy = cy + shift;
    func(w, cx, cy, frame);
    for (int i = 0; i < diag; i++)
    {
        cx = cx - 1;
        cy = cy + 1;
        func(w, cx, cy, frame);
    }

    //bottom left
    cx = cx - shift;
    func(w, cx, cy, frame);
    cx = cx - shift;
    func(w, cx, cy, frame);
    for (int i = 0; i < diag; i++)
    {
        cx = cx - 1;
        cy = cy - 1;
        func(w, cx, cy, frame);
    }

    //upper left
    cy = cy - shift;
    func(w, cx, cy, frame);
    cy = cy - shift;
    func(w, cx, cy, frame);
    for (int i = 0; i < diag; i++)
    {
        cx = cx + 1;
        cy = cy - 1;
        func(w, cx, cy, frame);
    }

    //top right
    cx = cx + shift;
    func(w, cx, cy, frame);
    cx = cx + shift;
    func(w, cx, cy, frame);
    for (int i = 0; i < diag; i++)
    {
        cx = cx + 1;
        cy = cy + 1;
        func(w, cx, cy, frame);
    }
    doToRadius(w, x, y, frame + 1, r, func);
}

char hasAir(World *w, int x, int y) // checks up/down/left/right to see if there is a blank square touching it
{
    if (w->grid[x][y + 1].type == blank || w->grid[x - 1][y].type == blank ||
        w->grid[x + 1][y].type == blank || w->grid[x][y - 1].type == blank)
    {
        return (1);
    }
    else
    {
        return (0);
    }
}

//functions affecting pixels

void push(World *w, int x, int y, char dist)
{
    float smaller;
    float tracker;
    int lastWhole = 0;
    int bigger;
    bool xIsBigger;
    if (abs(w->grid[x][y].vx) > abs(w->grid[x][y].vy))
    {
        smaller = (float)w->grid[x][y].vy / w->grid[x][y].vx;
        if ((w->grid[x][y].vy < 0 && smaller > 0) || (w->grid[x][y].vy > 0 && smaller < 0))
        {
            smaller *= -1;
        }
        tracker = smaller;
        bigger = w->grid[x][y].vx;
        xIsBigger = true;
    }
    else
    {
        smaller = (float)w->grid[x][y].vx / w->grid[x][y].vy;
        if ((w->grid[x][y].vx < 0 && smaller > 0) || (w->grid[x][y].vx > 0 && smaller < 0))
        {
            smaller *= -1;
        }
        tracker = smaller;
        bigger = w->grid[x][y].vy;
        xIsBigger = false;
    }

    int newx = x;
    int newy = y;
    int oldx = newx;
    int oldy = newy;
    for (int i = 0; i < abs(bigger); i++)
    {
        //check place
        if (xIsBigger)
        {
            newx = bigger > 0 ? newx + 1 : newx - 1;
            tracker += smaller;
            if (abs((int)floor(tracker)) > abs(lastWhole))
            {
                lastWhole = (int)floor(tracker);
                newy = smaller > 0 ? newy + 1 : newy - 1;
            }
        }
        else
        {
            newy = bigger > 0 ? newy + 1 : newy - 1;
            tracker += smaller;
            if (abs((int)floor(tracker)) > abs(lastWhole))
            {
                lastWhole = (int)floor(tracker);
                newx = smaller > 0 ? newx + 1 : newx - 1;
            }
        }

        if (w->grid[newx][newy].type == blank)
        { //check if pos is ok
            oldx = newx;
            oldy = newy;
        }
        else
        {
            break;
        }
    }
    swapPixel(w, &w->grid[x][y], &w->grid[oldx][oldy], x, y, oldx, oldy);
}

//10
//6
//6/10 = .6
//.6 1.2 1.8 2.4 3 3.6 4.2 4.8 5.4 6
//

void melt(World *w, int x, int y, char dist)
{
    if (boundsOk(w, x, y))
        changePixel(w, x, y, water);
}

void heat(World *w, int x, int y, char dist)
{
    // if (w->grid[x][y].heated && w->grid[x][y].heat >= dist)
    //     return;
    // else
    if (boundsOk(w, x, y) && w->grid[x][y].heat < MAX_HEAT)
        w->grid[x][y].heat += heatDist[dist];
    // if (w->grid[x][y].heat > attributes[w->grid[x][y].type].changeHeat )
    // {
    //     light(w, x, y, dist);
    // }
}

void light(World *w, int x, int y, char dist) // sets a pixel to burning
{
    //     switch (w->grid[x][y].flammable)
    // {
    // case (melts):
    //     melt(w, x, y, dist);
    //     break;
    // case (burns):
    //     burn(w, x, y, dist);
    //     break;
    // case (evaporates):
    //     //not yet added
    //     break;
    // }
    if (boundsOk(w, x, y))
        if (w->grid[x][y].flammable == 1 && !w->grid[x][y].burning && hasAir(w, x, y))
        {
            w->grid[x][y].burning = true; //@ could add heat factor here, need x heat before burning happens as insta burning is probably too fast
        }
}

void burn(World *w, int x, int y, char dist)
{
    if (boundsOk(w, x, y))
        switch (w->grid[x][y].type)
        {
        case powder:
            changePixel(w, x, y, fire);
            break;
        case coal:
            // doToNeighbours(w, x, y, &light);
            break;
        case oil:
            w->grid[x][y].life -= 1;
            w->grid[x][y].burning = true;

            //spread fire randomly
            int rx = rand() % (1 - (-1) + 1) + (-1);
            if (w->grid[x + rx][y - 1].type == blank)
            {
                changePixel(w, x, y - 1, fire);
            }

            //spread to neighbouring oil on same y level
            if (attributes[checkNeighbour(w, x, y, 4)].flammable == 1 && !w->grid[x - 1][y].burning)
            {
                w->grid[x - 1][y].burning = true;
                w->grid[x + 1][y].updated = true;
            }
            if (attributes[checkNeighbour(w, x, y, 6)].flammable == 1 && !w->grid[x + 1][y].burning)
            {
                w->grid[x + 1][y].burning = true;
                w->grid[x + 1][y].updated = true;
            }
            break;
        }
}

void blow(World *w, int x, int y)
{
    char dirx, diry;
    if (w->grid[x][y].vx == 0)
        dirx = 0;
    else
        dirx = w->grid[x][y].vx > 0 ? -1 : 1;
    if (w->grid[x][y].vy == 0)
        diry = 0;
    else
        diry = w->grid[x][y].vy > 0 ? -1 : 1;
    // printf("%d %d \n", dirx, diry);
    //change velocity for 1 pixel in the given direction
    w->grid[x + dirx][y + diry].vx = 4 * dirx;
    w->grid[x + dirx][y + diry].vy = 4 * diry;
}