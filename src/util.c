#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

#include "util.h"
#include "debug.h"

// Contains various general utility functions
// And by "general" I actually mean "geometry and collision detection".
// EDIT: I guess there's a string function here now

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2) {
    int24_t dX = x2 - x1;
    int24_t dY = y2 - y1;
    return sqrt(dX * dX + dY * dY);
}

// thank you https://cp-algorithms.com/geometry/circle-line-intersection.html
// TODO: maybe remove the float stuff?
// TODO: handle trivial case (rectangle intersection)
bool circCollidesSeg(circle_t *cr, lineSeg_t *l, lineSeg_t *inside) {
    // Use to store stuff temporarily in case *inside is null
    lineSeg_t temp;
    lineSeg_t *range = inside ? inside : &temp;
    // Convert line to Ax + By + c = 0 form and center circle on 0,0
    int24_t a = l->y1 - l->y2;
    int24_t b = l->x2 - l->x1;
    int24_t c = (l->x1-cr->x)*(l->y2-cr->y) - (l->x2-cr->x)*(l->y1-cr->y);
    float x0, y0;

    float hyp = (a*a+b*b);

    x0 = -a*c/hyp;
    y0 = -b*c/hyp;
    //dbg_sprintf(dbgout, "\nLineseg (%i, %i), (%i, %i)\n", l->x1-cr->x, l->y1-cr->y, l->x2-cr->x, l->y2-cr->y);
    //dbg_sprintf(dbgout, "A: %i, B: %i, C: %i\n", a, b, c);
    if((float)c*(float)c > cr->radius*cr->radius*hyp) {
        // Line misses circle entirely
        //dbg_sprintf(dbgout, "Misses circle completely\n");
        return false;
    } else {
        // Two points

        float d = cr->radius*cr->radius - c*c/hyp;
        float mult = sqrt(d / hyp);
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

uint24_t length(lineSeg_t *ls) {
    return distBetween(ls->x1, ls->y1, ls->x2, ls->y2);
}

uint8_t clipString(char* str, int24_t width) {
    uint8_t index;
    for(index = 0; str[index] && width; index++) {
        width -= gfx_GetCharWidth(str[index]);
    }
    return index - 1;
}

bool linesCollide(lineSeg_t* l1, lineSeg_t* l2) {
    int24_t p0_x = l1->x1, p1_x = l1->x2, p2_x = l2->x1, p3_x = l2->x2;
    int24_t p0_y = l1->y1, p1_y = l1->y2, p2_y = l2->y1, p3_y = l2->y2;
    int24_t s1_x, s1_y, s2_x, s2_y;
    int24_t d, s, t;
    s1_x = p1_x - p0_x;
    s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;
    s2_y = p3_y - p2_y;

    d = -s2_x * s1_y + s1_x * s2_y;
    s = -s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y);
    t =  s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x);

    return ((s >= 0) == (d >= 0) && abs(s) <= abs(d) && (t >= 0) == (d >= 0) && abs(t) <= abs(d));
}

void setBit(uint8_t *array, uint24_t offset, bool value) {
    uint8_t pos = offset / 8;
    uint8_t shift = 7 - offset % 8;
    uint8_t mask = 1 << shift;
    if(value) {
        array[pos] |= mask;
    } else {
        array[pos] &= ~mask;
    }
}

bool getBit(uint8_t *array, uint24_t offset) {
    uint8_t pos = offset / 8;
    uint8_t shift = 7 - offset % 8;
    uint8_t mask = 1 << shift;

    return array[pos] & mask;
}

void insertBoolArray(bool value, uint8_t* array, uint8_t index, uint8_t size) {
    // Get index/offset of the first byte to change
    uint8_t first = index / 8;
    uint8_t offset = index % 8;

    // Copy the first offset bits of the first byte
    uint8_t copy = array[first] & (0xFF << 7 - offset);

    uint8_t bytes = size / 8 - first + 1;

    //dbg_sprintf(dbgout, "array: %p, index: %u, size: %u\n", array, index, size);
    //dbg_sprintf(dbgout, "first: %u, offset: %u, copy: %u\n", first, offset, copy);
    //dbg_sprintf(dbgout, "%p, %u\n", &array[first], bytes);

    // Shift all bytes over
    shiftBitsRight(&array[first], bytes);

    // Replace the first bits of the first byte
    array[first] = copy | (array[first] & (0xFF >> offset));
}

void removeBoolArray(uint8_t* array, uint8_t index, uint8_t size) {
    // Get index/offset of the first byte to change
    uint8_t first = index / 8;
    uint8_t offset = index % 8;

    // Copy the first offset bits of the first byte
    uint8_t copy = array[first] & (0xFF << 7 - offset);

    uint8_t bytes = size / 8 - first + 1;

    //dbg_sprintf(dbgout, "array: %p, index: %u, size: %u\n", array, index, size);
    //dbg_sprintf(dbgout, "first: %u, offset: %u, copy: %u\n", first, offset, copy);
    //dbg_sprintf(dbgout, "%p, %u\n", &array[first], bytes);

    // Shift all bytes over
    shiftBitsLeft(&array[first], bytes);

    // Replace the first bits of the first byte
    array[first] = copy | (array[first] & (0xFF >> offset));
}
