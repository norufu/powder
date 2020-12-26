all: display world pixel selector menu
	gcc -o powder display.o world.o pixel.o selector.o menu.o -lSDL2

test: test
	gcc -o test test.o
	
display: display.c world.h
	gcc -c display.c

world: world.c world.h
	gcc -c world.c

pixel: pixel.c pixel.h
	gcc -c pixel.c

selector: selector.c selector.h
	gcc -c selector.c

menu: menu.c menu.h
	gcc -c menu.c

test: test.c
	gcc -c menu.c