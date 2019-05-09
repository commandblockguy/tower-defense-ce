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
#include "gfx/gfx_group.h"
#include "globals.h"
#include "util.h"
#include "enemy.h"
#include "tower.h"
#include "graphics.h"

// Various draw and menu functions

// TODO: add some sort of fancy background

const gfx_sprite_t *tower_sprites[] = {
    tower_standard, // STANDARD
    tower_sniper, // SNIPER
    tower_burst // BURST
};

const gfx_sprite_t *enemy_sprites[ENEMY_TYPES] = {
    alien0,
    alien1,
    alien2,
    alien3,
    alien4,
    alien5,
    alien6,
    alien7,
    alien8,
    alien9
};

// TODO: show XP / level up
void drawTowers(uint24_t csrX, uint8_t csrY, uint8_t selectedIndex) {
    // Loop through all towers
    int i;
    for(i = 0; i < NUM_TOWERS; i++) {
        tower_t *tower = &towers[i];
        if(csrX == -1) {
            // We are in edit mode
            circle_t c;
            lineSeg_t ls1, ls2;
            bool showRange = false;

            if(selectedIndex != (uint8_t)-1) {

                c.x = tower->posX;
                c.y = tower->posY;
                c.radius = tower->range;

                if(selectedIndex + 1 != bufSize) {

                    ls1.x1 = pathBufX[selectedIndex];
                    ls1.x2 = pathBufX[selectedIndex+1];
                    ls1.y1 = pathBufY[selectedIndex];
                    ls1.y2 = pathBufY[selectedIndex+1];

                    showRange = circCollidesSeg(&c, &ls1, NULL);
                }

                if(selectedIndex) {
                    ls2.x1 = pathBufX[selectedIndex-1];
                    ls2.x2 = pathBufX[selectedIndex];
                    ls2.y1 = pathBufY[selectedIndex-1];
                    ls2.y2 = pathBufY[selectedIndex];

                    showRange |= circCollidesSeg(&c, &ls2, NULL);
                }
            }

            drawTower(tower, tower->posX, tower->posY, 1, showRange);
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
    gfx_sprite_t *sprite = tower_sprites[tower->archetype];
    gfx_ScaledTransparentSprite_NoClip(sprite, x - sprite->width * scale / 2, y - sprite->height * scale / 2, scale, scale);

    // Draw the tower range
    if(range) {
        gfx_SetColor(HIGHLIGHT_COLOR);
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
    // TODO: thicc-er lines
    int i;

    gfx_SetColor(PATH_COLOR);

    for(i = 1; i < game.numPathPoints; i++) {
        int points[8];
        int24_t x1 = path[i].posX;
        int24_t y1 = path[i].posY;
        int24_t x2 = path[i-1].posX;
        int24_t y2 = path[i-1].posY;


        // Get the length of the line segment
        /*int24_t length = distBetween(x1, y1, x2, y2);

        // Find the offset from the actual path to the edge of the path
        int24_t offsetX = (PATH_WIDTH / 2) * (y2 - y1) / length;
        int24_t offsetY = (PATH_WIDTH / 2) * (x2 - x1) / length;

        //dbg_sprintf(dbgout, "(%i, %i)\n", offsetX, offsetY);

        // y u so slow???
        points[0] = x1 - offsetX; points[1] = y1 - offsetY;
        points[2] = x2 - offsetX; points[3] = y2 - offsetY;
        points[4] = x2 + offsetX; points[5] = y2 + offsetY;
        points[6] = x1 + offsetX; points[7] = y1 + offsetY;
        gfx_Polygon(points, 4);*/
        //gfx_FillTriangle_NoClip(, , , );
        //gfx_FillTriangle_NoClip(, , );

        gfx_Line(x1, y1, x2, y2);
    }
}

void drawPathBuffer(void) {
    int i;
    gfx_SetColor(PATH_COLOR);

    // Loop through all lines
    // TODO: thicc-er lines
    for(i = 0; i < bufSize - 1; i++) {
        // Check if there is an error
        if(getBit(pathBufPtErr, i)) {
            // Draw a red dot to indicate the invalid point
            gfx_SetColor(RED);
            gfx_Circle(pathBufX[i], pathBufY[i], 5);
            gfx_SetColor(PATH_COLOR);
        }
        if(getBit(pathBufSegErr, i)) {
            // Draw in red if there is an error
            gfx_SetColor(RED);
            gfx_Line(pathBufX[i], pathBufY[i], pathBufX[i+1], pathBufY[i+1]);
            gfx_SetColor(PATH_COLOR);
        } else {
            gfx_Line(pathBufX[i], pathBufY[i], pathBufX[i+1], pathBufY[i+1]);
        }
    }

    // draw the circle at the last point
    if(getBit(pathBufPtErr, i)) {
        gfx_SetColor(RED);
        gfx_Circle(pathBufX[i], pathBufY[i], 5);
        gfx_SetColor(PATH_COLOR);
    }
}

void drawUI(void) {
    // Draw the F buttons

    // Maybe I should generalize this so that each button has a function that is called when it
    // is pressed, and an array is read instead of a switch/case and function calls to make the UI 

    gfx_SetColor(WHITE);
    gfx_FillRectangle(0, LCD_HEIGHT - F_BTN_HEIGHT + 1, LCD_WIDTH, F_BTN_HEIGHT - 1);

    gfx_SetColor(BLACK);
    gfx_SetTextFGColor(BLACK);
    gfx_SetTextScale(1, 1);

    gfx_HorizLine(0, LCD_HEIGHT - F_BTN_HEIGHT, LCD_WIDTH);

    if(game.status != PATH_EDIT) {
        static uint8_t state;

        gfx_SetTextXY(1, LCD_HEIGHT - (F_BTN_HEIGHT + TEXT_HEIGHT) / 2);

        if(state < 255 / 4) {
            gfx_PrintString("Wave ");
            gfx_PrintUInt(game.waveNumber + 1, 3);
        } else if(state < 255 / 4 * 2) {
            gfx_PrintUInt(game.numEnemies, 3);
            gfx_PrintString(" enemies");
        } else if(state < 255 / 4 * 3) {
            gfx_PrintString("Spacing: ");
            gfx_PrintUInt(enemies[game.numEnemies - 1].offset / (game.numEnemies - 1), 2);
        } else {
            gfx_PrintString("Lives: ");
            gfx_PrintUInt(game.lives, 2);
        }

        state++;
    }

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
            // Not entirely sure how sprites work memory-wise
            // I think this is correct?
            gfx_sprite_t *sprite = enemy_sprites[enemy->type];
            // Enemy is on screen and should be displayed
            // draw sprite
            //gfx_SetColor(BLACK);
            //gfx_FillCircle(x, y, 5);
            gfx_TransparentSprite(sprite, x - sprite->width / 2, y - sprite->height / 2);
            // TODO: health indication
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
