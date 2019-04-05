#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "util.h"

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2) {
    int24_t dX = x2 - x1;
    int24_t dY = y2 - y1;
    return sqrt(dX * dX + dY * dY);
}

// TODO: remove
uint24_t dot(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2) {
    return x1 * y1 + x2 * y2;
}

// Returns to inside the segment containing all points on ls which are inside c
bool circCollidesSeg(circle_t *c, lineSeg_t *l, lineSeg_t *inside) {
    uint24_t closestX;
    uint8_t closestY;

    bool collides;

    closestOnLine(l, c->x, c->y, &closestX, &closestY);

    collides = distBetween(closestX, closestY, c->x, c->y) <= c->radius;

    if(inside != NULL) {
    	// TODO: this bit
    }

    return collides;
}

// Returns the closest point on the line to (rX, rY)
void closestOnLine(lineSeg_t *l, uint24_t x, uint8_t y, uint24_t *rX, uint8_t *rY) {
    // This was copied from the internet somewhere and slighly optimized
    // Because copy-pasted code is often stupid
    int24_t A1 = l->y2 - l->y1;
    int24_t B1 = l->x1 - l->x2;
    int24_t C1 = A1 * l->x1 + B1 * l->y1;
    int24_t C2 = A1 * y - B1 * x;
    int24_t det = A1 * A1 + B1 * B1;
    if(det != 0) {
        *rX = (A1 * C1 - B1 * C2) / det;
        *rY = (A1 * C2 + B1 * C1) / det;
    } else {
        *rX = x;
        *rY = y;
    }
}

bool ptInsideCirc(circle_t *c, uint24_t x, uint8_t y) {
    int24_t dX = c->x - x;
    int24_t dY = c->y - y;
    return dX * dX + dY * dY < c->radius * c->radius;
}

bool lineSegEqu(lineSeg_t *ls1, lineSeg_t *ls2) {
    return memcmp(ls1, ls2, sizeof(*ls1)) == 0;
}
