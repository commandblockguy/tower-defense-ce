#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "util.h"
#include "tower.h"
#include "enemy.h"

#include "debug.h"

// Factors determining XP curve
#define LVL_BASE 50
#define LVL_EXP  1.2

tower_t towers[NUM_TOWERS];

// Archetype names
const char *archNames[] = {
    "NONE",
    "Standard",
    "Sniper",
    "Burst"
};

// Target type names
const char *tgtNames[] = {
    "First",
    "Last",
    "Strong",
    "Weak",
    "Random"
};

void initTowers(void) {
    int i;
    // Initialize towers
    // There are more efficient ways to make a Poisson-disc distribution, but this saves on code size.
    // I think.
    for(i = 0; i < NUM_TOWERS; i++) {
        tower_t *tower = &towers[i];
        uint8_t candidate; // Candidate index
        uint24_t max = 0; // The distance to the nearest tower of the candidate that maximizes this
        uint24_t maxX = 0; // The position of that candidate
        uint8_t maxY = 0;
        // Pick the candidate furthest from the closest other tower
        for(candidate = 0; candidate < 5; candidate++) {
            uint8_t j; // Scan index
            uint24_t minDist = -1; // Distance to the closest tower
            uint24_t newX = randInt(2 * TOWER_RADIUS, LCD_WIDTH - 2 * TOWER_RADIUS);
            uint8_t newY = randInt(2 * TOWER_RADIUS, LCD_HEIGHT - F_BTN_HEIGHT - 2 * TOWER_RADIUS);
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
        // On second thought bridson's doesn't seem that much more complicated. Whatever.

        // set up tower upgrades, level, XP, stats, etc.
        tower->xp = 20; // TODO: temp
        tower->upgrades[0] = 0;
        tower->upgrades[1] = 0;
        tower->upgrades[2] = 0;
        tower->spentLevels = 0;
        tower->archetype = STANDARD; // TODO: default to standard?
        tower->targetType = FIRST;
        tower->ranges = NULL;
        calcTowerStats(&towers[i]);
    }
}

// TODO: Calculate the ranges of the path that the tower can hit
void calcTowerRanges(tower_t *tower) {
    int i;
    // Temporary memory to stick ranges in
    // Probably way bigger than it needs to be
    // Maybe allocate so that it is equal to game.numPathPoints?
    struct pathRange ranges[255];
    // Set to -1 so that the first increment sets it to 0
    uint8_t numRanges = -1;
    // Whether to append to the previous range or not
    bool continueRange = false;

    // Temp linesegs for collisions
    lineSeg_t lsOrig, lsInt;

    // Make a circle for the tower's range
    circle_t circ;
    circ.x = tower->posX;
    circ.y = tower->posY;
    circ.radius = tower->range;

    //dbg_sprintf(dbgout, "Calculating ranges for tower at %u, %u\n", tower->posX, tower->posY);

    // Iterate through all path segments
    for(i = 1; i < game.numPathPoints; i++) {
        // Get the lineSeg from the points
        lsOrig.x1 = path[i-1].posX;
        lsOrig.y1 = path[i-1].posY;
        lsOrig.x2 = path[ i ].posX;
        lsOrig.y2 = path[ i ].posY;

        //dbg_sprintf(dbgout, "Points %u (%u, %u) - %u (%u, %u):\n", i-1, lsOrig.x1, lsOrig.y1, i, lsOrig.x2, lsOrig.y2);

        // Check if it collides with circ
        // This also stores the intersection into lsInt
        if(circCollidesSeg(&circ, &lsOrig, &lsInt)) {
            //dbg_sprintf(dbgout, "Intersection: (%u, %u), (%u, %u)\n", lsInt.x1, lsInt.y1, lsInt.x2, lsInt.y2);
            if(lsOrig.x1 == lsInt.x1 && lsOrig.y1 == lsInt.y1) {
                // If the intersection has the same start point as the original
                if(!continueRange) {
                    // If we cannot add on to the last range, make a new one
                    //dbg_sprintf(dbgout, "Range %u: %u to %u\n", numRanges, ranges[numRanges].dis1, ranges[numRanges].dis2);
                    numRanges++;
                    ranges[numRanges].dis1 = ranges[numRanges].dis2 = path[i-1].distance;
                }
            } else {
                // Make a new range, as the intersection is not connected to the previous range
                if(numRanges != 255) {
                    // Ignore the bogus range that is a result of my debugging method
                    //dbg_sprintf(dbgout, "Range %u: %u to %u\n", numRanges, ranges[numRanges].dis1, ranges[numRanges].dis2);
                }
                numRanges++;
                ranges[numRanges].dis1 = // (how do I style this?)
                ranges[numRanges].dis2 = distBetween(lsOrig.x1, lsOrig.y1, lsInt.x1, lsInt.y1);
            }

            // Add the length of the line to the high distance
            ranges[numRanges].dis2 += length(&lsInt);

            // If the end points are the same, continue the range next loop
            if(lsOrig.x2 == lsInt.x2 && lsOrig.y2 == lsInt.y2) {
                continueRange = true;
            } else {
                continueRange = false;
            }
        } else {
            //dbg_sprintf(dbgout, "Collison check returned false\n");
            // If not, set continueRange to false and continue
            continueRange = false;
        }
    }

    //dbg_sprintf(dbgout, "Range %u: %u to %u\n", numRanges, ranges[numRanges].dis1, ranges[numRanges].dis2);

    // Change index to size
    numRanges++;

    // Allocate memory for the path range and copy the data into it
    // Set the tower's ranges and numRanges
    //dbg_sprintf(dbgout, "Total ranges: %u\n", numRanges);
    free(tower->ranges);
    tower->ranges = malloc(sizeof(tower->ranges[0]) * numRanges);
    memcpy(tower->ranges, ranges, sizeof(tower->ranges[0]) * numRanges);
    tower->numRanges = numRanges;
}

// TODO: implement
void attemptShot(tower_t *tower) {
    if(tower->cooldown) {
        // Tower is still on cooldown
        tower->cooldown--;
    } else {
        // We can actually fire

        // Handle different tower types differently
        switch(tower->archetype) {
            default:
            case(NONE):
                // We shouldn't ever get here
                dbg_sprintf(dbgerr, "Archetypeless tower in play\n");
            case(STANDARD):
            case(SNIPER):
                // Scan stuff somehow

                if(tower->targetType == LAST) {
                    int8_t i;
                    // Iterate through ranges backwards
                    for(i = tower->numRanges - 1; i >= 0; i--) {
                        // Check if all enemies have already passed this range
                        if((int24_t)(game.enemyOffset.fp.iPart - enemies[game.numEnemies - 1].offset) > tower->ranges[i].dis2) {
                            // Skip the rest of the check
                            continue;
                        }
                    }
                }
                break;
            case(BURST):
                // Damage all enemies inside the range
                break;
        }
    }
}

// TODO: if shot results in level up, handle accordingly

// Minimum amount of XP a tower of that level will have
uint24_t levelToXP(uint8_t level) {
    return LVL_BASE * pow(level, LVL_EXP);
}

// The level a tower with that amount of XP would have
uint8_t xpToLevel(uint24_t xp) {
    return log(xp / LVL_BASE) / log(LVL_EXP);
}
