#ifndef H_PATH
#define H_PATH

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_WIDTH 10

typedef struct {
    uint24_t posX;
    uint8_t posY;
    uint24_t distance; // Distance from the start point.
} pathPoint_t;

struct pathRange {
    uint24_t dis1;
    uint24_t dis2;
};

void resetPathBuffer(void);
int8_t updatePath(void);
void initBuffer(void);
void reverseBuffer(void);
bool checkPath(void);

#endif