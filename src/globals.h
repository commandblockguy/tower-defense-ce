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
#define CLICK_RADIUS 8
#define NUM_TOWERS   16
#define TOWER_RADIUS 8

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

enum {
    STANDARD,
    SNIPER,
    BURST,
    MAGE // may not implement this one, idk
};
typedef uint8_t archetype_t;

struct pathRange {
    uint24_t dis1;
    uint24_t dis2;
};

typedef struct {
    uint24_t posX;
    uint8_t posY;

    struct pathRange *ranges;
    uint24_t xp; // This does not get reset with each level
    uint8_t level;
    uint8_t spentLevels; // Number of levels which have been used up already

    archetype_t archetype; // Basic tower type
    uint24_t upgrades; // Upgrades bitmap
    uint24_t range;
    uint24_t maxCooldown; // Number that cooldown is set to on firing
} tower_t;

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

// Global vars

extern struct gameData game;
extern pathPoint_t *path;

extern uint16_t pathBufX[255];
extern uint8_t pathBufY[255];
extern char pathBufErr[32];
extern uint8_t bufSize;

extern tower_t towers[NUM_TOWERS];

#endif
