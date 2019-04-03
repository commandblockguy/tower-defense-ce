/*
 *--------------------------------------
 * Program Name: TDEFENSE
 * Author: commandblockguy
 * License:
 * Description: Reverse Tower Defense
 *--------------------------------------
*/

//TODO: remove
#undef NDEBUG

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "debug.h"
#include "keypadc.h"
#include "graphx.h"
#include "gfx/colors.h"
#include "gfx/gfx_group.h"
#include "draw.h"
#include "util.h"

const uint8_t NUM_LIVES     = 25;
const uint8_t NUM_TOWERS    = 16;
const uint8_t CSR_SPEED     = 3;
const uint8_t TOWER_DIST    = 48;

void main(void);
void mainMenu(void);
void highScores(void);
int24_t play(void);
void resetPathBuffer(void);
int8_t updatePath(void);
void initBuffer(void);
void reverseBuffer(void);

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

typedef struct {
    uint24_t position;
    uint8_t health;
} enemy_t;

tower_t towers[NUM_TOWERS];
enemy_t *enemies; // This is dynamically allocated at the beginning of each level
uint16_t pathBufX[255]; // These are used when editing the path
uint8_t pathBufY[255];
char pathBufErr[32]; // Each bit corresponds to whether the segment following that point has an error
uint8_t bufSize = 0; // Number of elements in the path buffer
pathPoint_t *path; // Path rendering is done with this

struct gameData game; // Game global

void main(void) {
    // Set up graphics
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal, 0);
    // Display the menu
    mainMenu();

    // Clean up
    gfx_End();
}

void mainMenu(void) {
    // do menu stuff
    // check if there is a game to resume
    /* Options:
        Resume
        New Game
        High Scores
        Exit
    */
    play(); //temp
}

void highScores(void) {
    // Display high scores
}

// Controls for path editing:
//  Press 2nd over a line to select it
//   Press 2nd over another location to add a point there, "bending" the line
//  Press alpha over a point to move it
//  Press delete over a point to remove it

// Returns the score, or -1 if exiting due to clear being pressed
int24_t play(void) {
    // Actual game stuff
    uint24_t csrX = LCD_WIDTH  / 2;
    uint8_t  csrY = LCD_HEIGHT / 2;
    static int i;
    static uint8_t carryMode;
    static uint8_t selectedIndex;

    bool updatedPath = true; // True if the player has changed the path

    // Initialize towers
    for(i = 0; i < NUM_TOWERS; i++) {
        // Do something
        // TODO: ensure that towers are not to close to each other
        // Maybe divide the screen into regions and then put one tower in each region?
        towers[i].posX = randInt(0, 320);
        towers[i].posY = randInt(0, 240);
    }

    // Reset game variables
    game.lives = NUM_LIVES;
    game.status = PRE_WAVE; // Whether the game is pre-wave, paused, or wave
    game.score = 0;

    resetPathBuffer();
    updatePath();

    // Repeats with each new wave
    while(game.lives) {
        // Game loop

        kb_key_t fKey = 0;

        kb_Scan(); // Update keys

        // Exit if clear is pressed
        if(kb_IsDown(kb_KeyClear)) {
            // If editing path:
            if(game.status == PATH_EDIT) {
                // Discard path without saving
                game.status = PRE_WAVE;
            } else {
                // Save game struct to an appvar
                // Save path
                // Save enemies

                // Free path
                // Free enemies
            }
            return -1;
        }

        if(game.status == WAVE) {
            // Handle physics stuff

            // If all enemies dead
            //  Free enemies
        }

        // Draw background
        gfx_FillScreen(BACKGROUND_COLOR);
        // Draw towers
        drawTowers(csrX, csrY);
        // Draw path
        if(game.status == PATH_EDIT) {
            // Draw the path buffer
        } else {
            drawPath();
        }
        // Draw UI
        drawUI();
        // Draw cursor
        gfx_TransparentSprite(cursor, csrX, csrY);

        // Blit the buffer
        gfx_BlitBuffer();

        // Check if any F keys are pressed
        fKey = kb_Data[1];

        // Move the cursor around
        // TODO: acceleration?
        //  (I always say this and then never do it)
        if(kb_IsDown(kb_KeyLeft )) csrX -= CSR_SPEED;
        if(kb_IsDown(kb_KeyRight)) csrX += CSR_SPEED;
        if(kb_IsDown(kb_KeyUp   )) csrY -= CSR_SPEED;
        if(kb_IsDown(kb_KeyDown )) csrY += CSR_SPEED;

        // Check if the cursor went offscreen
        if(csrX > 500)        csrX += LCD_WIDTH ; // hacky check for underflow
        if(csrX > LCD_WIDTH ) csrX -= LCD_WIDTH ;
        if(csrY > 250)        csrY += LCD_HEIGHT; // hacky check for  overflow
        if(csrY > LCD_HEIGHT) csrY -= LCD_HEIGHT;

        if(kb_IsDown(kb_Key2nd) || kb_IsDown(kb_KeyEnter)) {
            // Click detected

            // Check if we are over a tower

            // Check if we are over a Fn button
            if(csrY > LCD_HEIGHT - F_BTN_HEIGHT) {
                // Which column
                uint8_t btn = csrX / (LCD_WIDTH / 5);
                // Update the fKey pressed
                fKey = 1 << (4 - btn);
            }
        }

        switch(game.status) {
            case(PRE_WAVE):
                if(fKey == kb_Trace || fKey == kb_Graph) {
                    // Start the wave
                    if(updatedPath) {
                        updatedPath = false;
                        // Check if the path is valid
                        //  If not, inform the user of this
                        // Recalculate tower ranges
                        // Compute XP amount
                    }

                    // Spawn enemies

                    game.status = WAVE;
                }
                if(fKey == kb_Zoom) {
                    // Edit path
                    // Initialize buffer
                    initBuffer();

                    game.status = PATH_EDIT;
                }
                break;
            case(PAUSED):
                if(fKey == kb_Graph) game.status = WAVE;
                break;
            case(WAVE):
                if(fKey == kb_Graph) game.status = PAUSED;
                break;
            case(PATH_EDIT):
                switch(fKey) {
                    default:
                        break;
                    case(kb_Window):
                        // Reverse the buffer
                        reverseBuffer();
                        break;
                    case(kb_Zoom):
                        // Clear the buffer
                        resetPathBuffer();
                        break;
                    case(kb_Trace):
                        // Revert changes
                        game.status = PRE_WAVE;
                        break;
                    case(kb_Graph):
                        // Save changes
                        // TODO: support errors
                        if(!updatePath())
                            game.status = PRE_WAVE;
                        break;
                }
                break;
        }
        

        if(fKey) {
            do {
                kb_Scan();
            } while(kb_Data[1] || kb_IsDown(kb_Key2nd));
        }
    }
    return game.score;
}

void resetPathBuffer(void) {
    // Set path length to 2
    bufSize = 2;
    // Set points
    pathBufX[0] = LCD_WIDTH / 2;
    pathBufY[0] = LCD_HEIGHT - F_BTN_HEIGHT;

    pathBufX[1] = LCD_WIDTH / 2;
    pathBufY[1] = 0;
}

void initBuffer(void) {
    // Copy the contents of the path into the buffer
    static int i;
    for(i = 0; i < game.numPathPoints; i++) {
        pathBufX[i] = path[i].posX;
        pathBufY[i] = path[i].posY;
    }
}

void reverseBuffer(void) {
    // Reverse the direction of the path buffer
    static int i;
    for(i = 0; i < bufSize / 2; i++) {
        static uint24_t tempX;
        static uint8_t tempY;

        tempX = pathBufX[i];
        tempY = pathBufY[i];
    
        pathBufX[i] = pathBufX[bufSize - i];
        pathBufY[i] = pathBufY[bufSize - i];

        pathBufX[bufSize - i] = tempX;
        pathBufY[bufSize - i] = tempY;
    }
}

// Move the contents of the path buffer to the actual path
int8_t updatePath(void) {
    uint24_t distance = 0;
    uint24_t lastX = pathBufX[0];
    uint8_t lastY = pathBufY[0];
    static int i;

    // Verify that the path is valid, e.g.
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
