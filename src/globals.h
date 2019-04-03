#ifndef H_GLOBALS
#define H_GLOBALS

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F_BTN_HEIGHT 16
#define TEXT_HEIGHT  8

enum status {
	PRE_WAVE,
	PAUSED,
	WAVE,
	PATH_EDIT
};

typedef struct {
    uint24_t posX;
    uint8_t posY;
    uint24_t distance; // Distance from the start point.
} pathPoint_t;

// Data that will be included in the save
// Lines and enemies are also included but handled differently
struct gameData {
    uint8_t lives;
    uint8_t status; // Whether the game is pre-wave, paused, or wave
    int8_t score; // Number of enemies killed
    uint8_t numPathPoints; // Equal to the number of points
    
    uint24_t numEnemies;
    uint24_t livingEnemies;
    uint24_t enemyOffset; // Distance that enemies have proceeded onto the map
};

typedef uint8_t angle_t; // Angle where 256 = 360 degrees or 128 = 180 degrees

// Global vars

extern struct gameData game;
extern pathPoint_t *path;

extern uint16_t pathBufX[255];
extern uint8_t pathBufY[255];
extern char pathBuffErr[32];

#endif
