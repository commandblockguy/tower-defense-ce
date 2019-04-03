#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2) {
	int24_t dX = x2 - x1;
	int24_t dY = y2 - y1;
    return sqrt(dX * dX + dY * dY);
}