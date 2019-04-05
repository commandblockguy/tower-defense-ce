#ifndef H_UTIL
#define H_UTIL

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2);
bool ptInsideCirc(circle_t *c, uint24_t x, uint8_t y);
bool circCollidesSeg(circle_t *c, lineSeg_t *l, lineSeg_t *inside);
void closestOnLine(lineSeg_t *l, uint24_t x, uint8_t y, uint24_t *rX, uint8_t *rY);

#endif
