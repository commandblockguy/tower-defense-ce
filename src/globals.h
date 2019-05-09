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

#include "util.h"
#include "tower.h"
#include "path.h"

#define F_BTN_HEIGHT 16
#define TEXT_HEIGHT  8
#define CLICK_RADIUS 8

#define TPS 30
#define ONE_SECOND 128
#define TPC TPS / ONE_SECOND

#define APPVAR "TDefSave"
#define SCOREVAR "TDefHigh"

#define NUMSCORES 5

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define STRINGIFY(arg) #arg
#define STR(arg) STRINGIFY(arg)

#ifdef DEBUG
#define VERSION_STRING (STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "-debug")
#else
#define VERSION_STRING (STR(VERSION_MAJOR) "." STR(VERSION_MINOR))
#endif

enum status {
    PRE_WAVE,
    PAUSED,
    WAVE,
    PATH_EDIT
};

// Data that will be included in the save
// Lines and enemies are also included but handled differently
struct gameData {
    uint8_t lives;
    uint8_t status; // Whether the game is pre-wave, paused, or wave
    int24_t score; // Number of enemies killed
    uint24_t waveNumber;
    
    uint8_t numPathPoints; // Equal to the number of points
    
    // Wave specific variables
    uint24_t numEnemies;
    uint24_t livingEnemies;
    ufix_t enemyOffset; // Distance that enemies have proceeded onto the map
    uint8_t xpAmt; // XP amount, dependant on the length of the path
};

// Global vars

extern struct gameData game;

// yeah these are globals but I am lazy
extern uint24_t csrX;
extern uint8_t csrY;

// Upper 24 bits of the timer
// 128 cycles / second
#define timer_1_Upper *(uint24_t*)(((uint8_t*)&timer_1_Counter)+1)

#endif
