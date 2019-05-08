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

#include "tower.h"

void drawTowers(uint24_t csrX, uint8_t csrY, uint8_t selectedIndex);
void drawPath(void);
void drawUI(void);
void drawPathBuffer(void);
void drawFunctions(void);
void drawEnemies(void);
void blitLinesOffset(uint24_t start, uint24_t width, int24_t offset);

void drawButton(uint8_t posMin, uint8_t posMax, bool highlight, char *str);
void drawTower(tower_t *tower, uint24_t x, uint8_t y, uint8_t scale, bool range);
void printWithNewlines(char *string, uint24_t x, uint8_t y);

#endif
