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
    // Seed the RNG
    srand(rtc_Time());

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
    bool carry = false;
    static uint8_t selectedIndex;
    static uint24_t carryOrigX;
    static uint8_t carryOrigY;

    bool updatedPath = true; // True if the player has changed the path

    // Initialize towers
    // There are more efficient ways to make a Poisson-disc distribution, but this saves on code size.
    // I think.
    for(i = 0; i < NUM_TOWERS; i++) {
        // Do something
        uint8_t candidate;
        uint24_t max = 0;
        uint24_t maxX = 0;
        uint8_t maxY = 0;
        // Pick the candidate furthest from the closest other tower
        for(candidate = 0; candidate < 5; candidate++) {
            uint8_t j;
            uint24_t minDist = -1;
            uint24_t newX = randInt(TOWER_RADIUS, LCD_WIDTH - TOWER_RADIUS);
            uint8_t newY = randInt(TOWER_RADIUS, LCD_HEIGHT - F_BTN_HEIGHT - TOWER_RADIUS);
            // Find the distance to the closest tower
            for(j = 0; j < i; j++) {
                uint24_t dist = distBetween(towers[j].posX, towers[j].posY, newX, newY);
                if(dist < minDist) {
                    minDist = dist;
                }
            }
            // If this candidate is better than the previous best, update accordingly
            if(minDist > max) {
                max = minDist;
                maxX = newX;
                maxY = newY;
            }
        }
        towers[i].posX = maxX;
        towers[i].posY = maxY;
    }
    // On second thought bridson's doesn't seem that much more complicated. Whatever.

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
                return -1;
            }
            // Wait for key to be released
            while(kb_IsDown(kb_KeyClear)) kb_Scan();
        }

        if(game.status == WAVE) {
            // Handle physics stuff

            // If all enemies dead
            //  Free enemies
            //  Set status back to PRE_WAVE
        }

        // Draw background
        gfx_FillScreen(BACKGROUND_COLOR);
        // Draw path
        if(game.status == PATH_EDIT) {
            // Don't enlarge towers if we are editing paths
            drawTowers(-1, -1);
            // Draw the path buffer
            drawPathBuffer();
        } else {
            drawTowers(csrX, csrY);
            // Draw the regular path
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

        if(kb_IsDown(kb_KeyAlpha) && game.status == PATH_EDIT && !carry) {
            static int index;
            circle_t c;

            // Make a circle around the cursor
            c.x = csrX;
            c.y = csrY;
            c.radius = CLICK_RADIUS;

            // Check which point we are over
            for(index = 0; index < bufSize; index++) {
                // Loop through all points until one is in the circle
                if(ptInsideCirc(&c, pathBufX[index], pathBufY[index])) {
                    // Select that index
                    carryOrigX = pathBufX[index];
                    carryOrigY = pathBufY[index];
                    selectedIndex = index;
                    carry = true;
                    break;
                }
            }
        }

        if(kb_IsDown(kb_KeyDel) && game.status == PATH_EDIT && !carry) {
            static int index;
            circle_t c;

            // Make a circle around the cursor
            c.x = csrX;
            c.y = csrY;
            c.radius = CLICK_RADIUS;

            // Check which point we are over
            for(index = 0; index < bufSize; index++) {
                // Loop through all points until one is in the circle
                if(ptInsideCirc(&c, pathBufX[index], pathBufY[index])) {
                    // Select that index
                    memcpy(&pathBufX[index], &pathBufX[index+1], sizeof(pathBufX[0]) * (bufSize - index - 1));
                    memcpy(&pathBufY[index], &pathBufY[index+1], sizeof(pathBufY[0]) * (bufSize - index - 1));
                    bufSize--;
                    break;
                }
            }
        }

        if(kb_IsDown(kb_Key2nd) || kb_IsDown(kb_KeyEnter)) {
            // Click detected

            circle_t circ;
            int i, j;

            // Check mode
            if(game.status == PATH_EDIT) {
                if(carry) {
                    // Try to drop the point
                    // If position is valid, reset carry
                    if(selectedIndex == 0 || selectedIndex == bufSize + 1) {
                        // If the carried point is the start or end point, it should be on an edge
                    } else {
                        // Otherwise, it should be within the area of the screen
                    }
                    // Reset carry
                    carry = false;
                    // Skip the rest of the click check
                    // I know, bad style or whatever
                    goto clickSkip;
                }

                // Check if buffer would overflow
                if(bufSize >= 250) {
                    // TODO: display some message to the player
                } else {
                    // Make a circle around the cursor
                    circ.x = csrX;
                    circ.y = csrY;
                    circ.radius = CLICK_RADIUS;

                    // Check if we are over a line by iterating through all
                    // Not really a better way to do this AFAIK
                    for(i = 0; i < bufSize - 1; i++) {
                        // Get the lineSeg
                        lineSeg_t ls;

                        ls.x1 = pathBufX[i];
                        ls.y1 = pathBufY[i];
                        ls.x2 = pathBufX[i+1];
                        ls.y2 = pathBufY[i+1];

                        // Check if that circle overlaps the line
                        if(!circCollidesSeg(&circ, &ls, NULL)) continue;

                        // Select the index of the new point we are about to make
                        i++;

                        // Move all points down by one
                        for(j = bufSize; j > i; j--) {
                            pathBufX[j] = pathBufX[j - 1];
                            pathBufY[j] = pathBufY[j - 1];
                        }

                        // Increment the buffer size
                        bufSize++;
                        // Set the selected point and selection mode
                        selectedIndex = i;
                        carry = true;
                        break;
                    }
                }
            } else {
                // Check if we are over a tower
            }

            // Check if we are over a Fn button
            if(csrY > LCD_HEIGHT - F_BTN_HEIGHT) {
                // Which column
                uint8_t btn = csrX / (LCD_WIDTH / 5);
                // Update the fKey pressed
                fKey = 1 << (4 - btn);
            }

            clickSkip:
            // Wait to release 2nd
            do {
                kb_Scan();
            } while(kb_IsDown(kb_Key2nd));

        }

        if(game.status == PATH_EDIT && carry) {
            // If carrying a point, move the point to the cursor position
            pathBufX[selectedIndex] = csrX;
            pathBufY[selectedIndex] = csrY;
            if(selectedIndex == 0 || selectedIndex == bufSize + 1) {
                // If the carried point is the start or end point, snap to the nearest edge
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
            } while(kb_Data[1]);
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
    static int i;

    // If an endpoint is not on the edge of the playing area, extend that line out until it is

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
