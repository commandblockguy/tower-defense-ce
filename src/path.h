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
#define REGULATE_ENDPOINTS

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
bool checkPath(bool buffer);
bool checkBufPoint(uint8_t index);
bool checkBufSegment(uint8_t index);

extern uint16_t pathBufX[255]; // These are used when editing the path
extern uint8_t pathBufY[255];
extern uint8_t pathBufSegErr[32]; // Each bit corresponds to whether the segment following that point has an error
extern uint8_t pathBufPtErr[32]; // Each bit corresponds to whether the point has an error
extern uint8_t bufSize; // Number of elements in the path buffer
extern pathPoint_t *path; // Path rendering is done with this

#endif
