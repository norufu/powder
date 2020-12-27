#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "world.h"

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
    int atrCount = 7; // number of attributes per pixel

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

char checkNeighbour(World *w, int x, int y)
{

    return (0);
}

void changePixel(World *w, int x, int y, char newType)
{
    // printf("%d  \n", newType);
    w->grid[x][y].type = newType;
    w->grid[x][y].density = attributes[newType].density;
    w->grid[x][y].life = attributes[newType].life;
    w->grid[x][y].flammable = attributes[newType].flammable;
}

void addPixel(World *w, int penSize, int x, int y, char pixType)
{
    if (penSize == 1 && w->grid[x][y].type == 0)
    {
        changePixel(w, x, y, pixType);
        w->powderBank[pixType] -= 1;
        w->pixelCount += 1;
    }
    else if (penSize == 2 && w->grid[x][y].type == 0)
    {
        changePixel(w, x, y, pixType);
        w->powderBank[pixType] -= 1;
        w->pixelCount += 1;
        if (w->grid[x + 1][y].type == 0)
        {
            changePixel(w, x + 1, y, pixType);
            w->powderBank[pixType] -= 1;
            w->pixelCount += 1;
        }
        if (w->grid[x][y + 1].type == 0)
        {
            changePixel(w, x, y + 1, pixType);
            w->powderBank[pixType] -= 1;
            w->pixelCount += 1;
        }
        if (w->grid[x + 1][y + 1].type == 0)
        {
            changePixel(w, x + 1, y + 1, pixType);
            w->powderBank[pixType] -= 1;
            w->pixelCount += 1;
        }
    }
}

void removePixel(World *w, int x, int y)
{
    w->pixelCount--;
    w->grid[x][y].type = blank;
    w->grid[x][y].density = attributes[blank].density;
    w->grid[x][y].updated = true;
    w->grid[x][y].life = attributes[blank].life;
    w->grid[x][y].flammable = attributes[blank].flammable;
}

void melt(World *w, int x, int y)
{
    // printf("melt?\n");
    // printf("grid x y%d\n", w->grid[x][y].type);
    changePixel(w, x, y, water);
}