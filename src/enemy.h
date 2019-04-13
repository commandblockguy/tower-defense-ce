#ifndef H_ENEMY
#define H_ENEMY

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint24_t offset; // distance from the first enemy
    uint8_t health; // 0 = dead
    uint8_t nextPoint; // The next point the enemy will pass
} enemy_t;

bool enemyPos(enemy_t *enemy, uint24_t *x, uint8_t *y);

extern enemy_t *enemies;

#endif