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

// Advancement rate per tick, converted to fixed point
#define ADVANCEMENT_RATE (32 * 256 / TPS)

// TODO: finish pseudocoding
void processPhysics(void) {
	int i;
	// TODO: Advance the enemy offset
	game.enemyOffset.combined += ADVANCEMENT_RATE; // TODO: temp

	// Have each tower attempt its shot
	for(i = 0; i < NUM_TOWERS; i++) {
		attemptShot(&towers[i]);
	}

	// TODO: Check if any enemy has passed the end of the line

	// TODO: If all enemies dead
        //  Free enemies
        //  Set status back to PRE_WAVE
}