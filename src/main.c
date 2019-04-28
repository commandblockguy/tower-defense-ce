/*
 *--------------------------------------
 * Program Name: TDEFENSE
 * Author: commandblockguy
 * License:
 * Description: Reverse Tower Defense
 *--------------------------------------
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <debug.h>
#include <keypadc.h>

#include "globals.h"
#include "gfx/colors.h"
#include "gfx/gfx_group.h"
#include "graphics.h"
#include "util.h"
#include "tower.h"
#include "enemy.h"
#include "physics.h"
#include "menu.h"
#include "reader.h"

extern const readerFile_t rf_test; // TODO: temp

const uint8_t NUM_LIVES     = 25;
const uint8_t CSR_SPEED     = 3;
const uint8_t TOWER_DIST    = 48;
const uint24_t XP_DIST      = LCD_HEIGHT * 10;
const uint8_t BASE_XP       = 3;

void main(void);
int24_t play(bool resume);
bool saveAppvar(void);
bool loadAppvar(void);

extern uint16_t pathBufX[255]; // These are used when editing the path
extern uint8_t pathBufY[255];
extern char pathBufErr[32]; // Each bit corresponds to whether the segment following that point has an error
extern uint8_t bufSize; // Number of elements in the path buffer
extern pathPoint_t *path; // Path rendering is done with this

struct gameData game; // Game global

// Cursor position
uint24_t csrX = LCD_WIDTH  / 2;
uint8_t  csrY = LCD_HEIGHT / 2;

uint24_t ticks;

void main(void) {
    ti_var_t slot;
    char temp[5] = {0, 0, 0, 0, 0};
    dbg_sprintf(dbgout, "\n\nProgram Started\n");
    // Seed the RNG
    srand(rtc_Time());

    ti_CloseAll();

    //temp
    slot = ti_Open("test", "w");
    ti_Write(temp, 1, 5, slot);

    // Set up graphics
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal, 0);

    // Display the menu
    mainMenu();

    // Clean up
    gfx_End();
}

// Controls for path editing:
//  Press 2nd over a line to select it
//   Press 2nd over another location to add a point there, "bending" the line
//  Press alpha over a point to move it
//  Press delete over a point to remove it

// Returns the score, or -1 if exiting due to clear being pressed
int24_t play(bool resume) {
    // Actual game stuff
    uint8_t i;
    bool carry = false;
    uint8_t selectedIndex = 0;
    uint24_t carryOrigX;
    uint8_t carryOrigY;
    // Used for sliding popups down
    uint8_t animTime = 0;

    bool updatedPath = true; // True if the player has changed the path

    if(resume) {
        uint8_t i;
        // Resume game

        // Load the appvar
        if(!loadAppvar()) {
            dbg_sprintf(dbgout, "Failed to load appvar\n");
            return -1;
        }

        // Re-calculate tower ranges
        for(i = 0; i < NUM_TOWERS; i++) {
            dbg_sprintf(dbgout, "calc range %u...\n", i);
            calcTowerRanges(&towers[i]);
        }

        // Reset ticks passed
        resetTickTimer();
    } else {
        // Start new game
        initTowers();

        // Reset game variables
        game.lives = NUM_LIVES;
        game.status = PRE_WAVE; // Whether the game is pre-wave, paused, or wave
        game.score = 0;
        game.waveNumber = 0;

        resetPathBuffer();
        updatePath();

        spawnEnemies(0);
    }

    // Repeats with each new wave
    while(game.lives) {
        // Game loop

        kb_key_t fKey = 0;

        kb_Scan(); // Update keys

        // Exit if clear is pressed
        if(kb_IsDown(kb_KeyClear)) {
            // Wait for key to be released
            while(kb_IsDown(kb_KeyClear)) kb_Scan();
            // If editing path:
            if(game.status == PATH_EDIT) {
                // Discard path without saving
                game.status = PRE_WAVE;
            } else {
                uint8_t i;

                // Save the game so it can be resumed later
                saveAppvar();

                dbg_sprintf(dbgout, "saved.\n");

                dbg_sprintf(dbgout, "freeing e: %p, p: %p\n", enemies, path);

                // Handle memory stuff
                free(enemies);
                enemies = NULL;
                free(path);
                path = NULL;

                dbg_sprintf(dbgout, "freed p+e.\n");

                for(i = 0; i < NUM_TOWERS; i++) {
                    if(towers[i].numRanges)
                        free(towers[i].ranges);
                    towers[i].ranges = NULL;
                }

                dbg_sprintf(dbgout, "freed tower ranges.\n");

                return -1;
            }
        }

        if(game.status == WAVE) {
            uint8_t tries;
            // Run physics stuff until we are caught up
            // This will overflow after 73 minutes in a round, if my calculations are correct
            for(tries = 0; ONE_SECOND * ticks < TPS * timer_1_Upper && tries < 10; tries++) {
                processPhysics();
                // If the wave is over, quit running physics
                if(game.status != WAVE) break;
                // Increment the number of ticks that have elapsed
                ticks++;
            }
            if(tries >= 10) {
                // If this takes more than 10 tries we will assume that we are running behind
                // TODO: handle this somehow
                dbg_sprintf(dbgerr, "Game is running behind!\n");
            }
        }

        // Draw background
        // TODO: fancy sprites for a background?
        gfx_FillScreen(BACKGROUND_COLOR);
        // Draw path
        if(game.status == PATH_EDIT) {
            // Don't enlarge towers if we are editing paths
            drawTowers(-1, -1);
            // Draw the path buffer
            drawPathBuffer();
        } else {
            // Draw the regular path
            drawPath();
            drawTowers(csrX, csrY);
        }
        // Draw enemies
        if(game.status == WAVE || game.status == PAUSED) {
            drawEnemies();
        }
        // Draw UI
        drawUI();
        // Draw cursor
        gfx_TransparentSprite(cursor, csrX, csrY);

        if(animTime) {
            const uint8_t diff = LCD_HEIGHT / 10;
            // Slide the screen thing down
            blitLinesOffset(LCD_HEIGHT - diff * animTime, diff * animTime, diff);
            animTime--;
        }

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

        // Handle alpha presses
        if(kb_IsDown(kb_KeyAlpha) && game.status == PATH_EDIT && !carry) {
            int index;
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

        // Handle del presses
        if(kb_IsDown(kb_KeyDel) && game.status == PATH_EDIT && !carry) {
            int index;
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
                    //TODO: shift all the error checks over by a bit
                    bufSize--;
                    break;
                }
            }
            //TODO: Check if the newly created line and its adjacent points are valid
            // If not, set error bits
        }

        // Handle 2nd presses
        if(kb_IsDown(kb_Key2nd) || kb_IsDown(kb_KeyEnter)) {
            // Click detected

            circle_t circ;
            int i, j;

            // Check mode
            if(game.status == PATH_EDIT) {
                if(carry) {
                    // Try to drop the point
                    //TODO: If position is valid, reset carry
                    if(selectedIndex == 0 || selectedIndex == bufSize + 1) {
                        // If the carried point is the start or end point, it should be on an edge
                    } else {
                        // Otherwise, it should be within the area of the screen
                    }
                    // Reset carry
                    carry = false;
                    //TODO: Check if the point, the two adjacent lines, and the two adjacent points are still valid
                    // If not, set the error bits
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
                        // TODO: shift all error bits over by one

                        // Increment the buffer size
                        bufSize++;
                        // Set the selected point and selection mode
                        selectedIndex = i;
                        carry = true;
                        break;
                    }
                }
            } else {
                uint8_t i;
                // Check if we are over a tower
                // Scan through all towers
                for(i = 0; i < NUM_TOWERS; i++) {
                    tower_t *tower = &towers[i];
                    // Check if tower is near cursor
                    if(distBetween(tower->posX, tower->posY, csrX, csrY) < TOWER_RADIUS + CLICK_RADIUS) {
                        towerEdit(tower);
                        // Reset ticks passed
                        resetTickTimer();
                        // Setup animation
                        animTime = 10;
                        break;
                    }
                }
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
            } while(kb_IsDown(kb_Key2nd) || kb_IsDown(kb_KeyEnter));

        }

        if(game.status == PATH_EDIT && carry) {
            // If carrying a point, move the point to the cursor position
            pathBufX[selectedIndex] = csrX;
            // If the point is over the buttons, clip it to inside the play area
            pathBufY[selectedIndex] = csrY > LCD_HEIGHT - F_BTN_HEIGHT ? LCD_HEIGHT - F_BTN_HEIGHT : csrY;
            if(selectedIndex == 0 || selectedIndex == bufSize - 1) {
                // TODO: replace with casting a point out of the middle?
                //  Would be less confusing, but slower
                // If the carried point is the start or end point, snap to the nearest edge
                // Check top
                uint24_t lowDist = csrY;
                pathBufX[selectedIndex] = csrX;
                pathBufY[selectedIndex] = 0;
                // Check left
                if(csrX < lowDist) {
                    lowDist = csrX;
                    pathBufX[selectedIndex] = 0;
                    pathBufY[selectedIndex] = csrY;
                }
                // Check bottom
                if(LCD_HEIGHT - F_BTN_HEIGHT - csrY < lowDist) {
                    lowDist = LCD_HEIGHT - F_BTN_HEIGHT - csrY;
                    pathBufX[selectedIndex] = csrX;
                    pathBufY[selectedIndex] = LCD_HEIGHT - F_BTN_HEIGHT;
                }
                // Check right
                if(LCD_WIDTH - csrX < lowDist) {
                    lowDist = LCD_WIDTH - csrX;
                    pathBufX[selectedIndex] = LCD_WIDTH;
                    pathBufY[selectedIndex] = csrY;
                }
            }

            // Check point and segments
            // Check returns true if OK, but error bits are true if *not* OK
            setBit(pathBufPtErr, selectedIndex, !checkBufPoint(selectedIndex));
            
            setBit(pathBufSegErr, selectedIndex, !checkBufSegment(selectedIndex));

            // Check the preceding segment if it exists
            if(selectedIndex)
                setBit(pathBufSegErr, selectedIndex - 1, !checkBufSegment(selectedIndex - 1));
            
        }

        // Handle fKey clicks
        switch(game.status) {
            case(PRE_WAVE):
                if(fKey == kb_Trace || fKey == kb_Graph) {
                    uint8_t i;
                    // Start the wave
                    if(updatedPath) {
                        int i;
                        updatedPath = false;

                        // Recalculate tower ranges
                        for(i = 0; i < NUM_TOWERS; i++) {
                            calcTowerRanges(&towers[i]);
                        }

                        // Compute XP amount
                        game.xpAmt = BASE_XP + XP_DIST / path[game.numPathPoints - 1].distance;

                        dbg_sprintf(dbgout, "XP amount is %u\n", game.xpAmt);
                    }

                    // Reset enemy progression
                    game.enemyOffset.combined = 0;

                    // Reset all towers' cooldowns
                    for(i = 0; i < NUM_TOWERS; i++) {
                        towers[i].cooldown = 0;
                    }

                    resetTickTimer();

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
                if(fKey == kb_Graph) {
                    resetTickTimer();

                    game.status = WAVE;
                }
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
                        carry = false;
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
        

        // Wait for the held fKey to be released
        if(fKey) {
            do {
                kb_Scan();
            } while(kb_Data[1]);
        }
    }

    // Handle memory stuff
    free(enemies);
    enemies = NULL;
    free(path);
    path = NULL;

    for(i = 0; i < NUM_TOWERS; i++) {
        free(towers[i].ranges);
        towers[i].ranges = NULL;
    }

    // Delete save appvar
    ti_Delete(APPVAR);

    return game.score;
}

// Return true if success
bool saveAppvar(void) {
    ti_var_t slot;

    // Open the file
    slot = ti_Open(APPVAR, "w");

    if(!slot) return false;

    // Save game struct to an appvar
    ti_Write(&game, sizeof(game), 1, slot);

    // Save path
    ti_Write(path, sizeof(path[0]), game.numPathPoints, slot);

    // Save enemies
    ti_Write(enemies, sizeof(enemies[0]), game.numEnemies, slot);

    // Save towers
    ti_Write(towers, sizeof(towers[0]), NUM_TOWERS, slot);

    ti_Close(slot);

    return true;
}

bool appvarExists(void) {
    ti_var_t slot;

    // Open the file
    slot = ti_Open(APPVAR, "r");

    ti_Close(slot);

    dbg_sprintf(dbgout, "slot %u\n", slot);

    if(!slot) return false;

    return true;
}

bool loadAppvar(void) {
    ti_var_t slot;

    // Open the file
    slot = ti_Open(APPVAR, "r");

    if(!slot) return false;

    // Read game struct
    ti_Read(&game, sizeof(game), 1, slot);

    // Allocate memory
    free(path);
    free(enemies);
    path = NULL;
    enemies = NULL;
    dbg_sprintf(dbgout, "freed p+e on load\n");
    path = malloc(sizeof(path[0]) * game.numPathPoints);
    enemies = malloc(sizeof(enemies[0]) * game.numEnemies);

    // Read path
    ti_Read(path, sizeof(path[0]), game.numPathPoints, slot);

    // Read enemies
    ti_Read(enemies, sizeof(enemies[0]), game.numEnemies, slot);

    // Read towers
    ti_Read(towers, sizeof(towers[0]), NUM_TOWERS, slot);

    ti_Close(slot);

    dbg_sprintf(dbgout, "save loaded\n");
    dbg_sprintf(dbgout, "nE: %u, nP: %u\n", game.numEnemies, game.numPathPoints);
    dbg_sprintf(dbgout, "e: %p, p: %p\n", enemies, path);

    return true;
}

#undef malloc
void *malloc2(size_t size) {
    void *ptr;
    dbg_sprintf(dbgout, "Allocating %u bytes at ", size);
    ptr = malloc(size);
    dbg_sprintf(dbgout, "%p\n", ptr);
    return ptr;
}
#define malloc malloc2

#undef free
void free2(void* ptr) {
    dbg_sprintf(dbgout, "Freeing %p\n", ptr);
    free(ptr);
}
#define free free2
