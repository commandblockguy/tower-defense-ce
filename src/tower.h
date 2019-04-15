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
    FIRST = 0,
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
    uint24_t xp; // Used to calculate the level of the tower
    uint8_t spentLevels; // Number of levels which have been used up already

    archetype_t archetype; // Basic tower type
    target_t targetType;
    uint8_t upgrades[3]; // Upgrades
    uint8_t damage;
    uint8_t range;
    uint8_t maxCooldown; // Number that cooldown is set to on firing

    uint8_t cooldown; // Amount of time between shots
    uint8_t animTime; // Reset to 0 each time an animation is started, increments by 1 each tick
} tower_t;

typedef struct {
    char* name;
    char* description;
} upgradeData_t;

void initTowers(void);
void calcTowerStats(tower_t *tower);
void calcTowerRanges(tower_t *tower);
void attemptShot(tower_t *tower);

uint24_t levelToXP(uint8_t level);
uint8_t xpToLevel(uint24_t xp);

extern tower_t towers[NUM_TOWERS];
extern char *archNames[];
extern char *tgtNames[];

extern const upgradeData_t upgradeData[3][3][4];

#endif