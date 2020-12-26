#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "world.h"
#include "selector.h"

void newSelection(Selector *sel, int newType)
{
    sel->type = newType;
}