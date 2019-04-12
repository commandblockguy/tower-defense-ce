#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#undef NDEBUG
#include "debug.h"

// Contains various general utility functions
// And by "general" I actually mean "geometry and collision detection".

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2) {
    int24_t dX = x2 - x1;
    int24_t dY = y2 - y1;
    return sqrt(dX * dX + dY * dY);
}

// thank you https://cp-algorithms.com/geometry/circle-line-intersection.html
// TODO: maybe remove the float stuff?
bool circCollidesSeg(circle_t *cr, lineSeg_t *l, lineSeg_t *inside) {
    // Use to store stuff temporarily in case *inside is null
    lineSeg_t temp;
    lineSeg_t *range = inside ? inside : &temp;
    // Convert line to Ax + By + c = 0 form and center circle on 0,0
    int24_t a = l->y1 - l->y2;
    int24_t b = l->x2 - l->x1;
    int24_t c = (l->x1-cr->x)*(l->y2-cr->y) - (l->x2-cr->x)*(l->y1-cr->y);
    float x0, y0;

    x0 = -a*c/(a*a+b*b);
    y0 = -b*c/(a*a+b*b);
    //dbg_sprintf(dbgout, "\nLineseg (%i, %i), (%i, %i)\n", l->x1-cr->x, l->y1-cr->y, l->x2-cr->x, l->y2-cr->y);
    //dbg_sprintf(dbgout, "A: %i, B: %i, C: %i\n", a, b, c);
    if((float)c*(float)c > cr->radius*cr->radius*(float)(a*a+b*b)) {
        // Line misses circle entirely
        //dbg_sprintf(dbgout, "Misses circle completely\n");
        return false;
    }
    else {
        // Two points

        float d = cr->radius*cr->radius - c*c/(a*a+b*b);
        float mult = sqrt(d / (a*a+b*b));
        uint24_t ax, bx;
        uint8_t ay, by;
        // These two points represent where the extension of the segment intersects the circle
        bx = x0 + b * mult + cr->x;
        ax = x0 - b * mult + cr->x;
        by = y0 - a * mult + cr->y;
        ay = y0 + a * mult + cr->y;

        //dbg_sprintf(dbgout, "Intersections: (%i, %i), (%i, %i)\n", (int24_t)ax-cr->x, (int24_t)ay-cr->y, (int24_t)bx-cr->x, (int24_t)by-cr->y);

        // If the original point is inside the circle use that
        if(ptInsideCirc(cr, l->x1, l->y1)) {
            range->x1 = l->x1;
            range->y1 = l->y1;
        } else {
            if(ptOnSegment(ax, ay, l)) {
                range->x1 = ax;
                range->y1 = ay;
            } else {
                return false;
            }
        }

        if(ptInsideCirc(cr, l->x2, l->y2)) {
            range->x2 = l->x2;
            range->y2 = l->y2;
        } else {
            if(ptOnSegment(bx, by, l)) {
                range->x2 = bx;
                range->y2 = by;
            } else {
                return false;
            }
        }

        // If the two points we have found are the same, just ignore the collision
        if(range->x1 == range->x2 && range->y1 == range->y2) return false;
        return true;
    }
}

// This assumes that the point is already on the line, but is not necessarily on the segment
// TODO: optimize?
bool ptOnSegment(uint24_t x, uint8_t y, lineSeg_t *l) {
    return (
            (x <= l->x1 && x >= l->x2) ||
            (x >= l->x1 && x <= l->x2)
        ) && (
            (y <= l->y1 && y >= l->y2) ||
            (y >= l->y1 && y <= l->y2)
        );
}

// Checks if a point is inside a circle
bool ptInsideCirc(circle_t *c, uint24_t x, uint8_t y) {
    int24_t dX = c->x - x;
    int24_t dY = c->y - y;
    return dX * dX + dY * dY <= c->radius * c->radius;
}

// Lazily checks if two line segments are equal
// Maybe rewrite so that it works if the two points are reversed?
bool lineSegEqu(lineSeg_t *ls1, lineSeg_t *ls2) {
    return memcmp(ls1, ls2, sizeof(*ls1)) == 0;
}

// Insert the bit value into the (offset)th bit of array[index], shifting the bit that was there
// and all others after it up until array[size] to the right by one
// TODO: write in assembly where this is actually easier due to how flags work
void insertBoolArray(bool value, char* array, uint8_t index, uint8_t offset, uint8_t size) {

}

// Delete the (offset)th bit of array[index], shifting all bits after it up until array[size] left by one
// TODO: write in assembly where this is actually easier due to how flags work
void removeBoolArray(char* array, uint8_t index, uint8_t offset, uint8_t size) {

}

uint24_t length(lineSeg_t *ls) {
    return distBetween(ls->x1, ls->y1, ls->x2, ls->y2);
}
