#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "enemy.h"

#include "debug.h"

enemy_t *enemies = NULL; // This is dynamically allocated at the beginning of each level

// Returns true if the enemy is on the path
// The point is stored in (*x, *y)
bool enemyPos(enemy_t *enemy, uint24_t *x, uint8_t *y) {
    // Calculate the real position of the enemy
    int24_t realPos = game.enemyOffset.fp.iPart - enemy->offset;
    int24_t percentTop, percentBtm;
    pathPoint_t *next, *last;

    //dbg_sprintf(dbgout, "Enemy %u: dist %u, next: %u\n", enemy-enemies, enemy->offset, enemy->nextPoint);

    // Return false if the enemy has not yet entered the screen or has left the screen already
    if(realPos < 0 || realPos > path[game.numPathPoints - 1].distance) return false;

    // If the enemy has passed the point specified by nextPoint, increment nextPoint
    while(realPos > path[enemy->nextPoint].distance) {
        enemy->nextPoint++;
    }

    // Pointers for convenience
    next = &path[enemy->nextPoint];
    last = &path[enemy->nextPoint - 1];

    // Percent of the distance that the enemy has traveled between the previous and next points
    percentTop = (realPos - last->distance);
    percentBtm = (next->distance - last->distance);

    // Because we are dealing with lines, we can calculate the x and y components using that percentage
    *x = last->posX + (int24_t)(next->posX - last->posX) * percentTop / percentBtm;
    *y = last->posY + (int24_t)(next->posY - last->posY) * percentTop / percentBtm;
    return true;
}

// TODO: implement some sort of gradual increase in difficulty
void spawnEnemies(uint24_t round) {
    int i;
    uint8_t enemySpacing = 15; // temp

    game.livingEnemies = game.numEnemies = 50; // temp
    
    // Handle memory
    free(enemies);
    enemies = malloc(game.numEnemies * sizeof(enemies[0]));

    for(i = 0; i < game.numEnemies; i++) {
        enemy_t *enemy = &enemies[i];

        enemy->offset = i * enemySpacing;

        enemy->health = 10; // TODO: temp

        enemy->nextPoint = 0;
    }
}

// Returns the index of the furthest enemy that has not passed distance
uint24_t firstEnemyBefore(uint24_t distance) {
    // Basically just a binary search
    int24_t low = 0, high = game.numEnemies;
    //dbg_sprintf(dbgout, "Last offset before %u:\n", distance);
    while (low != high) {
        uint24_t mid = (low + high) / 2;
        int24_t realPos = game.enemyOffset.fp.iPart - enemies[mid].offset;
        if (realPos >= (int24_t)distance) {
            low = mid + 1;
            //dbg_sprintf(dbgout, "Low: %i\n", low);
        }
        else {
            high = mid;
            //dbg_sprintf(dbgout, "High: %i\n", high);
        }
    }
    return low;
}
