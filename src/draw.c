#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>
#include <keypadc.h>

#include <debug.h>

#include "gfx/colors.h"
#include "globals.h"
#include "util.h"
#include "enemy.h"
#include "tower.h"
#include "draw.h"

// Various draw and menu functions
// TODO: rename to graphics.c?

// TODO: add some sort of fancy background

void drawTowers(uint24_t csrX, uint8_t csrY) {
    // Loop through all towers
    int i;
    for(i = 0; i < NUM_TOWERS; i++) {
        tower_t *tower = &towers[i];
        if(csrX == -1) {
            drawTower(tower, tower->posX, tower->posY, 1, false);
            // Skip the distance check if it's not necessary
            continue;
        }
        // Enlarge the tower if the cursor is close enough
        if(distBetween(tower->posX, tower->posY, csrX, csrY) < TOWER_RADIUS + CLICK_RADIUS) {
            drawTower(tower, tower->posX, tower->posY, 2, true);
        } else {
            drawTower(tower, tower->posX, tower->posY, 1, false);
        }
    }
}


// Range: true if the tower range should be displayed
void drawTower(tower_t *tower, uint24_t x, uint8_t y, uint8_t scale, bool range) {
    int i;
    // TODO: rewrite using the actual sprites
    gfx_SetColor(tower->cooldown <= 5 ? RED : BLACK/*temp*/);
    gfx_Circle(x, y, scale * 8);

    // Draw the tower range
    if(range) {
        gfx_SetColor(RED);
        gfx_Circle(x, y, tower->range);
    }

    // Draw the laser
    if((tower->archetype == STANDARD || tower->archetype == SNIPER) && tower->cooldown >= tower->maxCooldown - 5) {
        gfx_SetColor(RED);
        gfx_Line(tower->posX, tower->posY, tower->targetX, tower->targetY);
    }
}

void drawPath(void) {
    // Loop through all path points
    int i;

    gfx_SetColor(PATH_COLOR);
    for(i = 1; i < game.numPathPoints; i++) {
        gfx_Line(path[i].posX, path[i].posY, path[i - 1].posX, path[i - 1].posY);
    }
}

void drawPathBuffer(void) {
    int i;
    gfx_SetColor(PATH_COLOR);
    // Loop through all lines
    for(i = 0; i < bufSize - 1; i++) {
        // Check if there is an error
        if(pathBufErr[i / 8] & (1 << (i % 8))) {
            // Draw in red if there is an error
            gfx_SetColor(RED);
            gfx_Line(pathBufX[i], pathBufY[i], pathBufX[i+1], pathBufY[i+1]);
            gfx_SetColor(PATH_COLOR);
        } else {
            gfx_Line(pathBufX[i], pathBufY[i], pathBufX[i+1], pathBufY[i+1]);
        }
    }
}

void drawUI(void) {
    // Draw the F buttons

    // Maybe I should generalize this so that each button has a function that is called when it
    // is pressed, and an array is read instead of a switch/case and function calls to make the UI 

    gfx_SetColor(BLACK);
    gfx_SetTextFGColor(BLACK);

    gfx_HorizLine(0, LCD_HEIGHT - F_BTN_HEIGHT, LCD_WIDTH);

    switch(game.status) {
        case(PRE_WAVE):
            drawButton(2, 2, false, "Edit Path");
            drawButton(3, 4, false, "Start Wave");
            break;
        case(PAUSED):
            drawButton(4, 4, false, "Resume");
            break;
        case(WAVE):
            drawButton(4, 4, false, "Pause");
            break;
        case(PATH_EDIT):
            drawButton(0, 0, false, "Help");
            drawButton(1, 1, false, "Reverse");
            drawButton(2, 2, false, "Clear");
            drawButton(3, 3, false, "Cancel");
            drawButton(4, 4, false, "Save");
    }
}

// TODO: make look fancy
// posMin-posMax: range 0-4, represents which button
// highlight: whether to color the background the highlight color
void drawButton(uint8_t posMin, uint8_t posMax, bool highlight, char *str) {
    // Calculate edges
    uint24_t leftEdge = posMin * (LCD_WIDTH / 5);
    uint24_t rightEdge = (posMax + 1) * (LCD_WIDTH / 5);
    uint24_t textX;

    gfx_SetColor(BLACK);

    //Draw the lines on either side
    gfx_VertLine( leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);
    gfx_VertLine(rightEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);

    if(highlight) {
        gfx_SetColor(HIGHLIGHT_COLOR);
        gfx_FillRectangle(leftEdge + 1, LCD_HEIGHT - F_BTN_HEIGHT + 1, rightEdge - leftEdge - 1, F_BTN_HEIGHT - 1);
        gfx_SetColor(BLACK);
    }

    // Draw the text
    textX = (leftEdge + rightEdge - gfx_GetStringWidth(str)) / 2 + 1;
    gfx_PrintStringXY(str, textX, LCD_HEIGHT - (F_BTN_HEIGHT + TEXT_HEIGHT) / 2);

}

// Draw all enemies to the screen
void drawEnemies(void) {
    int i;
    for(i = 0; i < game.numEnemies; i++) {
        enemy_t *enemy = &enemies[i];
        // Check if the enemy is on-screen, and get its coords
        uint24_t x;
        uint8_t y;

        // Don't render dead enemies
        if(!enemy->health) continue;

        if(enemyPos(enemy, &x, &y)) {
            // Enemy is on screen and should be displayed
            // TODO: draw sprite
            gfx_SetColor(BLACK);
            gfx_FillCircle(x, y, 5);
            // TODO: healthbar
        }
    }
}

// Print a string, respecting newlines
void printWithNewlines(char *string, uint24_t x, uint8_t y) {
    gfx_SetTextXY(x, y);
    while(*string != 0) {
        if(*string == '\n') {
            gfx_SetTextXY(x, y += 10);
        } else {
            gfx_PrintChar(*string);
        }
        string++;
    }
}

// Copy lines from the screen to the buffer, shifted by offset lines
void blitLinesOffset(uint24_t start, uint24_t width, int24_t offset) {
    // I tried gfx_vbuffer here and it didn't work for some odd reason
    memcpy(gfx_vram + 76800 + LCD_WIDTH * (start + offset), gfx_vram + LCD_WIDTH * start, width * LCD_WIDTH);
}
