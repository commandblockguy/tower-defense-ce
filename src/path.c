#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

#include "globals.h"

uint16_t pathBufX[255]; // These are used when editing the path
uint8_t pathBufY[255];
uint8_t pathBufSegErr[32]; // Each bit corresponds to whether the segment following that point has an error
uint8_t pathBufPtErr[32]; // Each bit corresponds to whether the point has an error
uint8_t bufSize; // Number of elements in the path buffer
pathPoint_t *path; // Path rendering is done with this

void resetPathBuffer(void) {
    // Set path length to 2
    bufSize = 2;
    // Set points
    pathBufX[0] = LCD_WIDTH / 2;
    pathBufY[0] = LCD_HEIGHT - F_BTN_HEIGHT;

    pathBufX[1] = LCD_WIDTH / 2;
    pathBufY[1] = 0;

    // The points themselves should always be valid
    pathBufPtErr[0] = 0;
    // If the segment is not valid, set the error bit
    setBit(pathBufSegErr, 0, !checkBufSegment(0));
}

void initBuffer(void) {
    // Copy the contents of the path into the buffer
    int i;
    for(i = 0; i < game.numPathPoints; i++) {
        pathBufX[i] = path[i].posX;
        pathBufY[i] = path[i].posY;
    }

    bufSize = game.numPathPoints;
    checkPath(true);
}

void reverseBuffer(void) {
    // Reverse the direction of the path buffer
    int i;
    for(i = 0; i < bufSize / 2; i++) {
        uint24_t tempX;
        uint8_t tempY;

        tempX = pathBufX[i];
        tempY = pathBufY[i];
    
        pathBufX[i] = pathBufX[bufSize - i - 1];
        pathBufY[i] = pathBufY[bufSize - i - 1];

        pathBufX[bufSize - i - 1] = tempX;
        pathBufY[bufSize - i - 1] = tempY;
    }
}

// Move the contents of the path buffer to the actual path
int8_t updatePath(void) {
    uint24_t distance = 0;
    uint24_t lastX = pathBufX[0];
    uint8_t lastY = pathBufY[0];
    int i;

    // Handle memory stuff
    free(path);
    path = malloc(sizeof(path[0]) * bufSize);

    game.numPathPoints = bufSize;

    // Loop through each point
    for(i = 0; i < bufSize; i++) {
        // Update the distance
        distance += distBetween(pathBufX[i], pathBufY[i], lastX, lastY);
        path[i].distance = distance;
        // Set the x and y values of the point
        lastX = path[i].posX = pathBufX[i];
        lastY = path[i].posY = pathBufY[i];
    }

    return 0;
}

// returns true if path is valid
bool checkPath(bool buffer) {
    uint8_t i;
    bool foundError = false;
    bool valid;
    // Initialize the buffer to mirror the path
    if(!buffer)
        initBuffer();
    // Check each point
    for(i = 0; i < bufSize - 1; i++) {
        // Set error bits
        valid = checkBufPoint(i);
        setBit(pathBufPtErr, i, !valid);
        if(!valid) {
            foundError = true;
            dbg_sprintf(dbgout, "Invalid point %u\n", i);
        }

        valid = checkBufSegment(i);
        setBit(pathBufSegErr, i, !valid);
        if(!valid) {
            foundError = true;
            dbg_sprintf(dbgout, "Invalid segment %u\n", i);
        }
    }

    // Get the last segment
    valid = checkBufPoint(i);
    setBit(pathBufPtErr, i, !valid);
    if(!valid) {
        foundError = true;
        dbg_sprintf(dbgout, "Invalid last point %u\n", i);
    }

    //dbg_sprintf(dbgout, "foundError: %u\n", foundError);

    return foundError;
}


// return true if point in buffer is valid
bool checkBufPoint(uint8_t index) {
    // Special case if first or last point
    if(!index || index == bufSize - 1) {
        // Check if at least one coordinate is 0 or max
        return !pathBufX[index] ||
            pathBufX[index] == LCD_WIDTH ||
            !pathBufY[index] ||
            pathBufY[index] == LCD_HEIGHT - F_BTN_HEIGHT;
    }
    // Check if on screen
    if(pathBufX[index] > LCD_WIDTH) return false;
    if(pathBufY[index] > LCD_HEIGHT - F_BTN_HEIGHT) return false;

    // TODO: angle stuff?

    return true;
}

// return true if segment in buffer is valid
bool checkBufSegment(uint8_t index) {
    uint8_t i;
    lineSeg_t seg;

    //dbg_sprintf(dbgout, "Checking segment %u\n", index);

    seg.x1 = pathBufX[index];
    seg.x2 = pathBufX[index+1];
    seg.y1 = pathBufY[index];
    seg.y2 = pathBufY[index+1];

    // Special case if last point
    if(index == bufSize - 1) {
        return true;
    }
    // Check if the segment overlaps any other line
    for(i = 0; i < bufSize - 1; i++) {
        lineSeg_t other;

        if(index - 1 <= i && i <= index + 1) continue;

        other.x1 = pathBufX[i];
        other.x2 = pathBufX[i+1];
        other.y1 = pathBufY[i];
        other.y2 = pathBufY[i+1];

        if(linesCollide(&seg, &other))
            return false;
    }
    //dbg_sprintf(dbgout, "Line is not colliding\n");
    // Check if the segment overlaps any tower
    for(i = 0; i < NUM_TOWERS; i++) {
        circle_t c;
        c.x = towers[i].posX;
        c.y = towers[i].posY;
        c.radius = TOWER_RADIUS + PATH_WIDTH / 2;

        if(circCollidesSeg(&c, &seg, NULL))
            return false;
    }

    //dbg_sprintf(dbgout, "Segment's fine.\n");

    return true;
}
