#ifndef WORLD_H
#define WORLD_H

//pixel stuff
typedef struct PixelAttributes
{
    char type;
    char density;
    char flammable;
    int colour[3];
    int life;
} PixelAttributes;

enum pixelTypes //0-70 is powders, 71-99 is fluids, 100+ is solid blocks
{
    blank = 0,
    powder = 1,
    fire = 50,
    water = 70,
    snow = 71,
    collector = 100,
    wall = 101,
    deleter = 102
};

enum flammableVals
{
    inflammable = 0,
    burns = 1,
    melts = 2
};

typedef struct Pixel
{
    int x;
    int y;
    int life;
    char density;
    char type;
    char flammable;
    bool updated;
} Pixel;

PixelAttributes attributes[127];

//world stuff
#define DISPLAY_WIDTH 512
#define DISPLAY_HEIGHT 712
#define WORLD_WIDTH 512
#define WORLD_HEIGHT 512

typedef struct World
{
    int pixelCount;
    char pixelsize;
    int width;
    int height;
    Pixel **grid;
    int powderBank[99];
} World;

//pixel functions
extern void swapPixel();
extern void swapPixel2();
extern void addPixel(World *w, int penSize, int x, int y, char pixType);
extern void removePixel();
extern void initializeAttributes();

extern void melt();

//world functions

extern void initializeWorld();
extern void updateWorld();
extern void drawWorld();
extern void drawPixel();
extern void removePixel();

#endif
