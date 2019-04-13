#ifndef H_TOWER
#define H_TOWER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_TOWERS 12

enum {
    NONE = 0,
    STANDARD,
    SNIPER,
    BURST,
    MAGE // may not implement this one, idk
};
typedef uint8_t archetype_t;

enum {
    FIRST,
    LAST,
    STRONG,
    WEAK,
    ARBITRARY
};
typedef uint8_t target_t;

typedef struct {
    uint24_t posX;
    uint8_t posY;

    uint8_t numRanges; // Number of elements in ranges
    struct pathRange *ranges;
    uint24_t xp; // This does not get reset with each level
    uint8_t level;
    uint8_t spentLevels; // Number of levels which have been used up already

    archetype_t archetype; // Basic tower type
    target_t targetType;
    uint8_t upgradesA; // Upgrades
    uint8_t upgradesB;
    uint8_t upgradesC;
    uint24_t range;
    uint24_t maxCooldown; // Number that cooldown is set to on firing
} tower_t;

typedef struct {
    char* name;
    char* description;
} upgradeData_t;

//TODO: why is this here?
extern upgradeData_t stdDmg[];

// TODO: upgrade types
// Standard tower upgrade types

void initTowers(void);
void calcTowerStats(tower_t *tower);
void calcTowerRanges(tower_t *tower);

extern tower_t towers[NUM_TOWERS];

#endif