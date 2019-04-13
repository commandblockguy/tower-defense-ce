#ifndef H_DRAW
#define H_DRAW

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void drawTowers(uint24_t csrX, uint8_t csrY);
void drawPath(void);
void drawUI(void);
void drawPathBuffer(void);
void drawFunctions(void);

// Menu functions
// TODO: move to own file?
void mainMenu(void);
void highScores(void);

#endif
