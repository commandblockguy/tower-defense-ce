#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

uint16_t pathBufX[255]; // These are used when editing the path
uint8_t pathBufY[255];
char pathBufErr[32]; // Each bit corresponds to whether the segment following that point has an error
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

    pathBufErr[0] = 0;
}

void initBuffer(void) {
    // Copy the contents of the path into the buffer
    int i;
    for(i = 0; i < game.numPathPoints; i++) {
        pathBufX[i] = path[i].posX;
        pathBufY[i] = path[i].posY;
    }
    // TODO: error bits
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

    //TODO: If an endpoint is not on the edge of the playing area, extend that line out until it is

    //TODO: Verify that the path is valid, e.g.
    //  no crossings - maybe enforce this while editing?
    //  no sharp angles (>90)
    //  minimum line length (angle (in circleints) / 8)

    // These conditions should be shown using red lines in the editor

    // If invalid, return a positive number

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

// TODO: returns true if path is valid
bool checkPath(void) {
    return true;
}
