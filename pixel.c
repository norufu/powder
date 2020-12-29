#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "world.h"

int heatDist[HEAT_RADIUS] = {5, 4, 3, 2, 1};

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
    if (x <= 0 || x >= w->width - 1)
    {
        return;
    }
    else if (y <= 0 || y >= w->height - 1)
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
    Pixel *temp1 = malloc(sizeof(Pixel));
    *temp1 = *p1;
    Pixel *temp2 = malloc(sizeof(Pixel));

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
    if (boundsOk(w, x, y))
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

            // doToNeighbours(w, x, y, &light);

            int rx = rand() % (1 - (-1) + 1) + (-1);
            ;
            if (w->grid[x + rx][y - 1].type == blank)
            {
                changePixel(w, x, y - 1, fire);
            }
            // if (attributes[checkNeighbour(w, x, y, 4)].flammable == 1)
            // {
            //     w->grid[x - 1][y].burning = true;
            // }
            if (attributes[checkNeighbour(w, x, y, 6)].flammable == 1)
            {
                w->grid[x + 1][y].burning = true;
            }
            if (attributes[checkNeighbour(w, x, y, 8)].flammable == 1)
            {
                w->grid[x + 1][y].burning = true;
            }
            break;
        }
}