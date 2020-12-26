#ifndef MENU_H
#define MENU_H
#include "world.h"

#define BUTTON_HEIGHT 20
#define BUTTON_WIDTH 75
#define BUTTON_COUNT 4
typedef struct Button
{
    int x;
    int y;
    int width;
    int height;
    // SDL_Rect r;
    char bType;
    char bDensity;
} Button;

typedef struct Menu
{
    Button menuButtons[BUTTON_COUNT];
} Menu;

extern void drawButtons();
extern void drawMenu();
extern void initializeMenu();
extern PixelAttributes checkMenu();

#endif