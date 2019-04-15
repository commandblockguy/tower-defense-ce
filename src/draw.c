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

// Various draw and menu functions
// TODO: rename to graphics.c?

void drawButton(uint8_t posMin, uint8_t posMax, bool highlight, char *str);
void drawTower(tower_t *tower, uint24_t x, uint8_t y, uint8_t scale, bool range);

// from main.c
int24_t play(void);

const char *statNames[] = {
    "Damage",
    "Range",
    "Cooldown"
};

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

// Uses the same scale as gfx_ScaledSprite
// Range: true if the tower range should be displayed
void drawTower(tower_t *tower, uint24_t x, uint8_t y, uint8_t scale, bool range) {
    // TODO: rewrite using the actual sprites
    gfx_SetColor(RED);
    gfx_Circle(x, y, scale * 8);

    // Draw the tower range
    if(range) {
        gfx_SetColor(RED);
        gfx_Circle(x, y, tower->range);
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
        if(enemyPos(enemy, &x, &y)) {
            // Enemy is on screen and should be displayed
            // TODO: draw sprite
            gfx_SetColor(BLACK);
            gfx_FillCircle(x, y, 5);
            // TODO: healthbar
        }
    }
}

void mainMenu(void) {
    // TODO: do menu stuff
    // check if there is a game to resume

    // Menu loop
    // Render fancy menu background
    // Handle keyboard inputs
    // Draw the options

    /* Options:
        Resume
        New Game
        High Scores
        Exit
    */
    play(); //temp
}

void highScores(void) {
    //TODO: Display high scores

    // Open high score appvar
    // Read list of scores
    // Display names
    // Wait for keypress
}

// TODO: finish
void towerEdit(tower_t *tower) {
    uint8_t animTime = 9;
    uint8_t level = xpToLevel(tower->xp);
    // Amount of XP needed for the current level
    uint24_t baseXP = levelToXP(level);
    // Amount of XP needed for the next level
    uint24_t nextXP = levelToXP(level + 1);
    // Width of the xp bar
    uint8_t barWidth = (LCD_WIDTH / 3 - 1) * (tower->xp - baseXP) / (nextXP - baseXP);
    // Which upgrade is being hovered over
    int8_t selection = 0;

    while(true) {
        int i;
        // Loop
        kb_Scan();
        if(kb_IsDown(kb_KeyClear)) {
            // Go back to normal gameplay if clear is pressed
            while(kb_IsDown(kb_KeyClear)) kb_Scan();
            return;
        }

        // Arrow keys
        if(kb_IsDown(kb_KeyUp)) {
            while(kb_IsDown(kb_KeyUp)) kb_Scan();
            if(--selection < 0) selection = 2;
        }
        if(kb_IsDown(kb_KeyDown)) {
            while(kb_IsDown(kb_KeyDown)) kb_Scan();
            if(++selection >= 3) selection = 0;
        }

        // 2nd / enter

        // Mode

        // fKeys
        switch(kb_Data[1]) {
            case(kb_Yequ):
                tower->targetType = FIRST;
                break;
            case(kb_Window):
                tower->targetType = LAST;
                break;
            case(kb_Zoom):
                tower->targetType = STRONG;
                break;
            case(kb_Trace):
                tower->targetType = WEAK;
                break;
            case(kb_Graph):
                tower->targetType = ARBITRARY;
        };

        // Draw the background
        gfx_FillScreen(BACKGROUND_COLOR);

        // Line on top of the screen
        gfx_SetColor(BLACK);
        gfx_HorizLine(0, 0, LCD_WIDTH);

        // Draw the tower icon in top right
        gfx_Rectangle(2 * LCD_WIDTH / 3 + 1, 2, LCD_WIDTH / 3 - 1, 2 * LCD_HEIGHT / 3);
        drawTower(tower, 5 * LCD_WIDTH / 6, LCD_HEIGHT / 3, 4, false);

        // Draw the XP bar and level under it
        gfx_SetColor(BLACK);
        gfx_FillRectangle(2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 6, LCD_WIDTH / 3 - 1, 2);
        gfx_SetColor(XP_COLOR);
        gfx_FillRectangle(2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 6, barWidth, 2);
        gfx_SetColor(BLACK);

        gfx_SetTextFGColor(BLACK);
        gfx_PrintStringXY("Level ", 2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 12);
        gfx_PrintUInt(level, level < 10 ? 1 : 2);

        if(level > tower->spentLevels) {
            gfx_SetTextFGColor(XP_COLOR);
            gfx_SetTextXY(2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 22);
            gfx_PrintUInt(level - tower->spentLevels, level - tower->spentLevels < 10 ? 1 : 2);
            gfx_PrintString(" unspent!");
            gfx_SetTextFGColor(BLACK);
        }

        // Draw text at the top and bottom
        gfx_PrintStringXY("Archetype:  ", 2, 2);
        gfx_PrintString(archNames[tower->archetype]);
        gfx_PrintStringXY("Press [mode] to change tower type", 2, LCD_HEIGHT - F_BTN_HEIGHT - 12);

        // TODO: Show the 3 possible upgrades
        for(i = 0; i < 3; i++) {
            const uint8_t btnHeight = (LCD_HEIGHT - F_BTN_HEIGHT - 12 - 12) / 3;
            const uint24_t width = 2 * LCD_WIDTH / 3 - 2;
            uint8_t baseY = 12 + i * btnHeight;
            upgradeData_t upgrade = upgradeData[tower->archetype-1][i][tower->upgrades[i]];
            uint8_t stat = (&tower->damage)[i];
            uint8_t newStat = i == 2 ? stat / 2 : stat * 2;

            // Draw a rectangle
            gfx_SetColor(BLACK);
            gfx_Rectangle(2, baseY, width, btnHeight - 2);

            // Highlight the inside of the rectangle
            if(i == selection) {
                gfx_SetColor(HIGHLIGHT_COLOR);
                gfx_FillRectangle(3, baseY + 1, width - 2, btnHeight - 4);
            }

            // Print the upgrade name
            gfx_SetTextScale(1, 2);
            gfx_PrintStringXY(upgrade.name, 5, baseY + 2);
            gfx_SetTextScale(1, 1);
            // Print upgrade description
            gfx_PrintStringXY(statNames[i], 5, baseY + 24);
            gfx_PrintString(": ");
            gfx_PrintUInt(stat, 3);
            gfx_PrintString(" -> ");
            gfx_PrintUInt(newStat, 3);

            gfx_PrintStringXY(upgrade.description, 5, baseY + 34);

        }

        gfx_SetColor(BLACK);

        // Show the target switches
        gfx_HorizLine(0, LCD_HEIGHT - F_BTN_HEIGHT, LCD_WIDTH);
        for(i = 0; i < 5; i++) {
            drawButton(i, i, i == tower->targetType, tgtNames[i]);
        }

        if(animTime) {
            uint8_t screenOffset = animTime * (LCD_HEIGHT / 10);
            gfx_ShiftDown(screenOffset);
            gfx_BlitLines(gfx_screen, 0, screenOffset);
            gfx_BlitBuffer();
            animTime--;
        } else {
            // Blit the graphics buffer
            gfx_BlitBuffer();
        }
    }
}
