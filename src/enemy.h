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

#define ENEMY_TYPES 10

typedef struct {
    uint24_t offset; // distance from the first enemy
    uint8_t maxHealth;
    uint8_t health; // 0 = dead
    uint8_t nextPoint; // The next point the enemy will pass
    uint8_t type;
} enemy_t;

bool enemyPos(enemy_t *enemy, uint24_t *x, uint8_t *y);
void spawnEnemies(uint24_t round);
uint24_t firstEnemyBefore(uint24_t distance);

extern enemy_t *enemies;

#endif