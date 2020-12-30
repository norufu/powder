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
    int changeHeat;
} PixelAttributes;

enum pixelTypes //0-70 is powders, 71-99 is fluids, 100+ is solid blocks
{
    blank = 0,
    powder = 1,
    coal = 2,
    fire = 50,
    water = 70,
    snow = 71,
    oil = 72,
    collector = 100,
    wall = 101,
    deleter = 102
};

enum flammableVals
{
    inflammable = 0,
    burns = 1,
    melts = 2,
    evaporates = 3
};

typedef struct Pixel //@ can most of these be used just by using attributes[type].x? maybe more efficient
{
    int life;
    char density;
    char type;
    char flammable;
    unsigned char heat;
    bool updated;
    bool heated;
    bool burning;
} Pixel;

PixelAttributes attributes[127];

//world stuff
#define DISPLAY_WIDTH 512
#define DISPLAY_HEIGHT 712
#define WORLD_WIDTH 512
#define WORLD_HEIGHT 512
#define HEAT_RADIUS 5
#define MAX_HEAT 200
typedef struct World
{
    int pixelCount;
    char pixelsize;
    int width;
    int height;
    char zoom;
    int zoomX;
    int zoomY;
    Pixel **grid;
    int powderBank[99];
    bool heatDebugOn;
    bool paused;
} World;

//pixel functions
extern void swapPixel();
extern void addPixel(World *w, int x, int y, char pixType);
extern void removePixel();
extern void changePixel(World *w, int x, int y, char newType);
extern void initializeAttributes();
extern void initializeWall(World *w, int x, int y, char newType);

extern void doToNeighbours();
extern void doToRadius();
extern void melt(World *w, int x, int y, char dist);
extern void burn(World *w, int x, int y, char dist);
extern void light(World *w, int x, int y, char dist);
extern void heat(World *w, int x, int y, char dist);

//world functions

extern void initializeWorld();
extern void updateWorld();
extern void drawWorld();
extern void drawPixel();
extern void removePixel();
extern void adjustZoom();
extern void addPixelRadius();

#endif
