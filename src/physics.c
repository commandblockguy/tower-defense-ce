#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

#include "globals.h"
#include "tower.h"
#include "enemy.h"

// Advancement rate per tick, converted to fixed point
#define ADVANCEMENT_RATE (32 * 256 / TPS)

extern uint24_t ticks;

void processPhysics(void) {
    int i;
    // Advance the enemy offset
    game.enemyOffset.combined += ADVANCEMENT_RATE; // TODO: temp

    // Have each tower attempt its shot
    for(i = 0; i < NUM_TOWERS; i++) {
        attemptShot(&towers[i]);
    }

    i = firstEnemyBefore(path[game.numPathPoints - 1].distance);
    while(i >= 0) {
        if(enemies[i].health && (int24_t)(game.enemyOffset.fp.iPart - enemies[i].offset) > (int24_t)path[game.numPathPoints - 1].distance) {
            // Mark enemy as dead, lose a life
            enemies[i].health = 0;
            game.lives--;
            game.livingEnemies--;
            dbg_sprintf(dbgout, "%u lives left.\n", game.lives);
        }
        i--;
    }

    // If all enemies dead
    if(!game.livingEnemies) {

        dbg_sprintf(dbgout, "All enemies dead, ending wave...\n");
        //  Free enemies
        free(enemies);
        enemies = NULL;

        // Set all tower cooldowns to 0
        for(i = 0; i < NUM_TOWERS; i++) {
            towers[i].cooldown = 0;
        }
        
        //  Set status back to PRE_WAVE
        game.status = PRE_WAVE;

        game.waveNumber++;

        // Spawn the next wave's enemies
        spawnEnemies(game.waveNumber);
    }
}

void resetTickTimer(void) {
    timer_Control &= ~TIMER1_ENABLE;
    timer_1_Counter = 0;
    timer_Control |= TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    ticks = 0;
}
