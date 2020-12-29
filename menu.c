#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "menu.h"
#include "world.h"

void initializeMenu(Menu *m)
{
    int menuTypeOrder[BUTTON_COUNT] = {powder, coal, fire, water, snow, oil};

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        m->menuButtons[i].bType = attributes[menuTypeOrder[i]].type;
        m->menuButtons[i].bDensity = attributes[menuTypeOrder[i]].density;
        m->menuButtons[i].x = 5;
        m->menuButtons[i].y = 520 + 25 * i;
        m->menuButtons[i].width = BUTTON_WIDTH;
        m->menuButtons[i].height = BUTTON_HEIGHT;
    }
}

void drawMenu(SDL_Renderer *r, Menu *m)
{
    for (int i = 0; i < sizeof(m->menuButtons) / sizeof(Button); i++)
    {
        SDL_Rect b;
        b.x = m->menuButtons[i].x;
        b.y = m->menuButtons[i].y;
        b.w = BUTTON_WIDTH;
        b.h = BUTTON_HEIGHT;
        SDL_SetRenderDrawColor(r, attributes[m->menuButtons[i].bType].colour[0], attributes[m->menuButtons[i].bType].colour[1], attributes[m->menuButtons[i].bType].colour[2], 255);
        SDL_RenderFillRect(r, &b);
    }
}
//
PixelAttributes checkMenu(int mx, int my, Menu *m) //checks what button is clicked
{
    for (int i = 0; i < sizeof(m->menuButtons) / sizeof(Button); i++)
    {
        if ((m->menuButtons[i].x < mx && mx < m->menuButtons[i].x + m->menuButtons[i].width) && (m->menuButtons[i].y < my && my < m->menuButtons[i].y + m->menuButtons[i].height))
        {
            return (attributes[m->menuButtons[i].bType]); //return the menu button clicked
        }
    }
    return (attributes[blank]); //return blank
}