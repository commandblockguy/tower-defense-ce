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

// TODO: finish pseudocoding
void processPhysics(void) {
    int i;
    // Advance the enemy offset
    game.enemyOffset.combined += ADVANCEMENT_RATE; // TODO: temp

    // Have each tower attempt its shot
    for(i = 0; i < NUM_TOWERS; i++) {
        attemptShot(&towers[i]);
    }

    // TODO: Check if any enemy has passed the end of the line

    // If all enemies dead
    if(!game.livingEnemies) {
        //  Free enemies
        free(enemies);
        enemies = NULL;

        // Set all tower cooldowns to 0
        for(i = 0; i < NUM_TOWERS; i++) {
            tower->cooldown = 0;
        }
        
        //  Set status back to PRE_WAVE
        game.status = PRE_WAVE;
    }
}

void resetTickTimer(void) {
    timer_Control = TIMER1_DISABLE;
    timer_1_Counter = 0;
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    ticks = 0;
}
