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
#undef NDEBUG
#include "debug.h"

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2) {
    int24_t dX = x2 - x1;
    int24_t dY = y2 - y1;
    return sqrt(dX * dX + dY * dY);
}
/*
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
}*/

// todo: debug all of this
// thank you https://cp-algorithms.com/geometry/circle-line-intersection.html
bool circCollidesSeg(circle_t *cr, lineSeg_t *l, lineSeg_t *inside) {
    // Use to store stuff temporarily in case *inside is null
    static lineSeg_t temp;
    lineSeg_t *range = inside ? inside : &temp;
    // Convert line to Ax + By + c = 0 form and center circle on 0,0
    int24_t a = l->y1 - l->y2;
    int24_t b = l->x2 - l->x1;
    int24_t c = (l->x1-cr->x)*(l->y2-cr->y) - (l->x2-cr->x)*(l->y1-cr->y);
    float x0, y0;

    x0 = -a*c/(a*a+b*b);
    y0 = -b*c/(a*a+b*b);
    //dbg_sprintf(dbgout, "\nLineseg (%i, %i), (%i, %i)\n", l->x1-cr->x, l->y1-cr->y, l->x2-cr->x, l->y2-cr->y);
    //dbg_sprintf(dbgout, "A: %i, B: %i, C: %i\n", a, b, (int24_t)c);
    if(c*c > cr->radius*cr->radius*(a*a+b*b)) {
        // Line misses circle entirely
        //dbg_sprintf(dbgout, "Misses circle completely\n");
        return false;
    }
    else {
        // Two points

        uint8_t foundPts = 0; // Number of points that have already been found

        float d = cr->radius*cr->radius - c*c/(a*a+b*b);
        float mult = sqrt(d / (a*a+b*b));
        uint24_t ax, bx;
        uint8_t ay, by;
        // These two points represent where the extension of the segment intersects the circle
        ax = x0 + b * mult + cr->x;
        bx = x0 - b * mult + cr->x;
        ay = y0 - a * mult + cr->y;
        by = y0 + a * mult + cr->y;

        //dbg_sprintf(dbgout, "Intersections: (%i, %i), (%i, %i)\n", (int24_t)ax-cr->x, (int24_t)ay-cr->y, (int24_t)bx-cr->x, (int24_t)by-cr->y);

        // Basically, if a point is on both the circle and the segment, it will be the endpoint
        // The same is true for any segment endpoint which is inside the circle
        // It's possible that a point fulfills both of those, so we should check halfway through if there are already 2 points
        if(ptInsideCirc(cr, l->x1, l->y1)) {
            (&range->x1)[foundPts] = l->x1;
            (&range->y1)[foundPts] = l->y1;
            //dbg_sprintf(dbgout, "Point %u: Endpoint 1\n", foundPts);
            foundPts++;
        }
        if(ptInsideCirc(cr, l->x2, l->y2)) {
            (&range->x1)[foundPts] = l->x2;
            (&range->y1)[foundPts] = l->y2;
            //dbg_sprintf(dbgout, "Point %u: Endpoint 2\n", foundPts);
            foundPts++;
        }
        if(foundPts < 2) {
            if(ptOnSegment(ax, ay, l)) {
                (&range->x1)[foundPts] = ax;
                (&range->y1)[foundPts] = ay;
                //dbg_sprintf(dbgout, "Point %u: Intersection 1\n", foundPts);
                foundPts++;
            }
            if(ptOnSegment(bx, by, l)) {
                (&range->x1)[foundPts] = bx;
                (&range->y1)[foundPts] = by;
                //dbg_sprintf(dbgout, "Point %u: Intersection 2\n", foundPts);
                foundPts++;
            }
        }
        // If we have not found 2 points, no collision occurs
        if(foundPts != 2) return false;
        // If the two points we have found are the same, just ignore the collision
        if(range->x1 == range->x2 && range->y1 == range->y2) return false;
        return true;
    }
}

// This assumes that the point is already on the line, but is not necessarily on the segment
bool ptOnSegment(uint24_t x, uint8_t y, lineSeg_t *l) {
    return (
            (x <= l->x1 && x >= l->x2) ||
            (x >= l->x1 && x <= l->x2)
        ) && (
            (y <= l->y1 && y >= l->y2) ||
            (y >= l->y1 && y <= l->y2)
        );
}

bool ptInsideCirc(circle_t *c, uint24_t x, uint8_t y) {
    int24_t dX = c->x - x;
    int24_t dY = c->y - y;
    return dX * dX + dY * dY <= c->radius * c->radius;
}

bool lineSegEqu(lineSeg_t *ls1, lineSeg_t *ls2) {
    return memcmp(ls1, ls2, sizeof(*ls1)) == 0;
}

void insertBoolArray(bool value, char* array, uint8_t index, uint8_t offset, uint8_t size) {

}

void removeBoolArray(char* array, uint8_t index, uint8_t offset, uint8_t size) {

}
