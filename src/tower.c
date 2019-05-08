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
#define LVL_BASE 1000.0
#define LVL_EXP  1.2

tower_t towers[NUM_TOWERS];

// Archetype names
const char *archNames[] = {
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
        // On second thought, bridson's doesn't seem that much more complicated. Whatever.

        // set up tower upgrades, level, XP, stats, etc.
        tower->xp = 0;
        tower->upgrades[0] = 0;
        tower->upgrades[1] = 0;
        tower->upgrades[2] = 0;
        tower->spentLevels = 0;
        tower->archetype = STANDARD; // TODO: default to standard or random or NONE?
        tower->targetType = FIRST;
        tower->ranges = NULL;
        tower->numRanges = 0;
        tower->cooldown = 0;
        calcTowerStats(&towers[i]);
    }
}

// Calculate the ranges of the path that the tower can hit
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
                ranges[numRanges].dis2 = path[i-1].distance + distBetween(lsOrig.x1, lsOrig.y1, lsInt.x1, lsInt.y1);
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
    if(numRanges) {
        free(tower->ranges);
        tower->ranges = malloc(sizeof(tower->ranges[0]) * numRanges);
        memcpy(tower->ranges, ranges, sizeof(tower->ranges[0]) * numRanges);
    }
    tower->numRanges = numRanges;
}

// TODO: implement first, highest, lowest, etc.
// TODO: make more general / DRY
void attemptShot(tower_t *tower) {
    if(tower->cooldown) {
        // Tower is still on cooldown
        tower->cooldown--;
    } else {
        int i;
        // We can actually fire

        int24_t firstEnemyDist = game.enemyOffset.fp.iPart - enemies[0].offset;
        int24_t lastEnemyDist = game.enemyOffset.fp.iPart - enemies[game.numEnemies - 1].offset;
                        

        // Debug stuff //temp
        /*dbg_sprintf(dbgout, "Enemies: [");
        for(i = 0; i < game.numEnemies; i++) {
            dbg_sprintf(dbgout, "(%u, %i, %u),", enemies[i].offset, game.enemyOffset.fp.iPart - enemies[i].offset, enemies[i].health);
        }
        dbg_sprintf(dbgout, "]\n");*/

        // Handle different tower types differently
        switch(tower->archetype) {
            default:
            case(NONE):
                // We shouldn't ever get here
                dbg_sprintf(dbgerr, "Archetypeless tower in play\n");
            case(STANDARD):
            case(SNIPER): {
                // Scan stuff somehow

                if(tower->targetType == LAST) {
                    int24_t index;
                    // Iterate through ranges backwards
                    for(i = tower->numRanges - 1; i >= 0; i--) {
                        struct pathRange *range = &tower->ranges[i];
                        // Check if all enemies have already passed this range
                        if(firstEnemyDist < (int24_t)range->dis1) {
                            // Skip the rest of the check
                            //dbg_sprintf(dbgout, "First enemy has not entered this range\n");
                            return;
                        }
                        if(lastEnemyDist > (int24_t)range->dis2) {
                            // Skip the rest of the check
                            //dbg_sprintf(dbgout, "Last enemy has left this range\n");
                            continue;
                        }
                        //dbg_sprintf(dbgout, "Checking range %u - (%u, %u)\n", i, range->dis1, range->dis2);
                        //dbg_sprintf(dbgout, "Last enemy distance: %i\n", lastEnemyDist);
                        index = firstEnemyBefore(range->dis1);
                        //dbg_sprintf(dbgout, "Index: %u - (%i, %u)\n", index, game.enemyOffset.fp.iPart - enemies[index].offset, enemies[index].health);
                        // Get the first after or on the range start point
                        if(game.enemyOffset.fp.iPart - enemies[index].offset != range->dis1) index--;
                        // Find a living enemy
                        while(index >= 0 && !enemies[index].health) {
                            //dbg_sprintf(dbgout, "Index %u is dead\n", index);
                            if(game.enemyOffset.fp.iPart - enemies[index].offset > range->dis2)
                                index = -1;
                            index--;
                        }
                        // If we exit the range again, skip to the next one
                        if(index < 0) continue;
                        //dbg_sprintf(dbgout, "Tower: %u\n", tower-towers);
                        //dbg_sprintf(dbgout, "Found enemy %u\n", index);
                        towerDamageEnemy(tower, &enemies[index]);
                        return;
                    }
                } else {
                    // Tower target is something else
                    int24_t index;
                    // Iterate through ranges forwards
                    for(i = 0; i < tower->numRanges; i++) {
                        struct pathRange *range = &tower->ranges[i];
                        
                        // Check if all enemies have already passed this range
                        /*if(firstEnemyDist < (int24_t)range->dis1) {
                            // Skip the rest of the check
                            //dbg_sprintf(dbgout, "First enemy has not entered this range\n");
                            continue;
                        }
                        if(lastEnemyDist > (int24_t)range->dis2) {
                            // Skip the rest of the check
                            //dbg_sprintf(dbgout, "Last enemy has left this range\n");
                            return;
                        }*/
                        //dbg_sprintf(dbgout, "Checking range %u - (%u, %u)\n", i, range->dis1, range->dis2);
                        //dbg_sprintf(dbgout, "Last enemy distance: %i\n", lastEnemyDist);

                        //dbg_sprintf(dbgout, "Tower: %u\n", tower-towers);

                        index = firstEnemyBefore(range->dis2);
                        //dbg_sprintf(dbgout, "Index: %u - (%i, %u)\n", index, game.enemyOffset.fp.iPart - enemies[index].offset, enemies[index].health);
                        
                        if((int24_t)(game.enemyOffset.fp.iPart - enemies[index].offset) < (int24_t)range->dis1)
                            continue;

                        // Find a living enemy
                        while(!enemies[index].health && index < game.numEnemies) {
                            //dbg_sprintf(dbgout, "Index %u is dead\n", index);
                            if((int24_t)(game.enemyOffset.fp.iPart - enemies[index].offset) < (int24_t)range->dis1)
                                index = game.numEnemies;
                            index++;
                        }
                        // If we exit the range again, skip to the next one
                        if(index >= game.numEnemies) continue;
                        //dbg_sprintf(dbgout, "Tower: %u\n", tower-towers);
                        //dbg_sprintf(dbgout, "Found enemy %u\n", index);
                        towerDamageEnemy(tower, &enemies[index]);
                        return;
                    }
                }
                break;
            }

            case(BURST): {
                // Endpoints on which enemies to damage
                uint24_t i;
                // Damage all enemies inside the range
                for(i = 0; i < tower->numRanges; i++) {
                    struct pathRange *range = &tower->ranges[i];
                    int24_t first;
                    int24_t last;
                    int24_t index;

                    // Check if all enemies have already passed this range
                    if(firstEnemyDist < (int24_t)range->dis1) {
                        // Skip the rest of the check
                        //dbg_sprintf(dbgout, "First enemy has not entered this range\n");
                        continue;
                    }
                    if(lastEnemyDist > (int24_t)range->dis2) {
                        // Skip the rest of the check
                        //dbg_sprintf(dbgout, "Last enemy has left this range\n");
                        return;
                    }
                    //dbg_sprintf(dbgout, "Checking range %u - (%u, %u)\n", i, range->dis1, range->dis2);
                    //dbg_sprintf(dbgout, "Last enemy distance: %i\n", lastEnemyDist);

                    //dbg_sprintf(dbgout, "Tower: %u\n", tower-towers);

                    first = firstEnemyBefore(range->dis2);
                    last = firstEnemyBefore(range->dis1) - 1;

                    //dbg_sprintf(dbgout, "First: %i, Last: %i\n", first, last);

                    // Damage all enemies in the range
                    for(index = first; index <= last; index++) {
                        //dbg_sprintf(dbgout, "Found enemy %u\n", index);
                        towerDamageEnemy(tower, &enemies[index]);
                    }

                    if(tower->cooldown) return;
                }
                break;
            }
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
    return pow(xp / LVL_BASE, 1 / LVL_EXP);
}

void calcTowerStats(tower_t *tower) {
    // compute damage, range, reload time based on upgrades
    tower->damage      = baseStats[tower->archetype][0] + statMultipliers[tower->archetype][0] * (tower->upgrades[0] + 1);
    tower->range       = baseStats[tower->archetype][1] + statMultipliers[tower->archetype][1] * (tower->upgrades[1] + 1);
    tower->maxCooldown = baseStats[tower->archetype][2] / (tower->upgrades[2] + 1);
}

void towerDamageEnemy(tower_t *tower, enemy_t *enemy) {
        // If enemy is dead, skip it
        if(!enemy->health) return;

        // Reset the tower cooldown
        tower->cooldown = tower->maxCooldown;

        if(enemy->health <= tower->damage) {
            // Shot will kill enemy
            tower->xp += game.xpAmt * enemy->health;
            enemy->health = 0;
            game.livingEnemies--;
            game.score++;
           //dbg_sprintf(dbgout, "%u left.\n", game.livingEnemies);
        } else {
            // Shot will not kill enemy
            //dbg_sprintf(dbgout, "Damaging enemy %u\n", index);
            enemy->health -= tower->damage;
            tower->xp += game.xpAmt * tower->damage;
        }

        // Set the coordinates for lasers
        enemyPos(enemy, &tower->targetX, &tower->targetY);
}
