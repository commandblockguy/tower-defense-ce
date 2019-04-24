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

// Number of elements in an array
#define elems(ARR) (sizeof(ARR)/sizeof(ARR[0]))

struct FixedPoint {
	uint8_t fPart;
	uint16_t iPart;
};

typedef union {
	struct FixedPoint fp;
	uint24_t combined;
} ufix_t;

typedef uint8_t angle_t; // Angle where 256 = 360 degrees or 128 = 180 degrees

typedef struct circle {
    uint24_t x;
    uint8_t y;
    uint8_t radius;
} circle_t;

typedef struct lineSeg {
    uint24_t x1;
    uint24_t x2;
    uint8_t y1;
    uint8_t y2;
} lineSeg_t;

uint24_t distBetween(uint24_t x1, uint8_t y1, uint24_t x2, uint8_t y2);
bool ptInsideCirc(circle_t *c, uint24_t x, uint8_t y);
bool circCollidesSeg(circle_t *c, lineSeg_t *l, lineSeg_t *inside);
bool ptOnSegment(uint24_t x, uint8_t y, lineSeg_t *l);
void closestOnLine(lineSeg_t *l, uint24_t x, uint8_t y, uint24_t *rX, uint8_t *rY);
bool lineSegEqu(lineSeg_t *ls1, lineSeg_t *ls2);
uint24_t length(lineSeg_t *ls);

uint8_t clipString(char* str, int24_t width);

#endif
