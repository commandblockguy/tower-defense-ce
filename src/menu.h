#ifndef H_MENU
#define H_MENU

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enemy.h"
#include "tower.h"

// Menu functions
void mainMenu(void);
void highScores(void);
void towerEdit(tower_t *tower);
archetype_t selectTowerType(void);

void initCursor(void);
void updateCursor(void);

void loseScreen(uint24_t score);

void popup(char* str);

#endif