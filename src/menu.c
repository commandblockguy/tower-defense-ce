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
#include "menu.h"

// from main.c
int24_t play(void);

extern const char *statNames[];

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
    uint8_t barWidth = (LCD_WIDTH / 3 - 1) * (uint24_t)(tower->xp - baseXP) / (nextXP - baseXP);
    // Which upgrade is being hovered over
    int8_t selection = 0;

    while(kb_IsDown(kb_Key2nd)) kb_Scan();

    while(true) {
        int i;
        // Whether there are any unspent levels
        bool hasUnspentLevel = (int8_t)(level - tower->spentLevels) > 0;
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
        if(kb_IsDown(kb_Key2nd)) {
            // Hopefully this is the worst line in the program
            bool hasNextUpgrade = tower->upgrades[selection] < 4 && ((char*)&upgradeData[tower->archetype][selection][tower->upgrades[selection]])[0] != 0;
            // Wait for key to be released
            while(kb_IsDown(kb_Key2nd)) kb_Scan();

            // Do nothing if there is no next upgrade
            if(hasNextUpgrade) {
                if(hasUnspentLevel) {
                    bool response;
                    // TODO: confirmation menu
                    response = true;
                    if(response) {
                        // Add the new upgrade
                        tower->upgrades[selection]++;
                        tower->spentLevels++;
                        calcTowerStats(tower);
                    }
                }
            }
        }

        // Mode
        if(kb_IsDown(kb_KeyMode)) {
            if(true) {
                tower->upgrades[0] = 0;
                tower->upgrades[1] = 0;
                tower->upgrades[2] = 0;

                tower->spentLevels = 0;

                // Archetype selection
                tower->archetype = selectTowerType();

                calcTowerStats(tower);
            }
            while(kb_IsDown(kb_KeyMode)) kb_Scan();
        }

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
            //case(kb_Graph):
            //    tower->targetType = ARBITRARY;
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

        // Level text
        gfx_SetTextFGColor(BLACK);
        gfx_PrintStringXY("Level ", 2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 12);
        gfx_PrintUInt(level, level < 10 ? 1 : 2);

        // Number of unspent levels
        if(level > tower->spentLevels) {
            gfx_SetTextFGColor(XP_COLOR);
            gfx_SetTextXY(2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 22);
            gfx_PrintUInt(level - tower->spentLevels, level - tower->spentLevels < 10 ? 1 : 2);
            gfx_PrintString(" unspent!");
            gfx_SetTextFGColor(BLACK);
        } else {
            gfx_PrintStringXY("None to spend", 2 * LCD_WIDTH / 3 + 1, 2 * LCD_HEIGHT / 3 + 22);
        }

        // Draw text at the top and bottom
        gfx_PrintStringXY("Archetype:  ", 2, 2);
        gfx_PrintString(archNames[tower->archetype]);
        gfx_PrintStringXY("Press [mode] to change tower type", 2, LCD_HEIGHT - F_BTN_HEIGHT - 12);
        
        for(i = 0; i < 3; i++) {
            const uint8_t btnHeight = (LCD_HEIGHT - F_BTN_HEIGHT - 12 - 12) / 3;
            const uint24_t width = 2 * LCD_WIDTH / 3 - 2;
            uint8_t baseY = 12 + i * btnHeight; // Coordinate for the button

            // Draw a rectangle
            gfx_SetColor(BLACK);
            gfx_Rectangle(2, baseY, width, btnHeight - 2);

            // Highlight the inside of the rectangle
            if(hasUnspentLevel && i == selection) {
                gfx_SetColor(HIGHLIGHT_COLOR);
                gfx_FillRectangle(3, baseY + 1, width - 2, btnHeight - 4);
            }

            // Don't draw text if we are sliding up
            if(!animTime) {
                uint8_t nextLevel = tower->upgrades[i] + 2; // The level the upgrade will be after the next upgrade
                uint24_t stat = (&tower->damage)[i]; // The current stat
                // The new value of the stat if the upgrade is bought
                uint24_t newStat = i == 2 ? baseStats[tower->archetype][i] / nextLevel : stat + statMultipliers[tower->archetype][i];
                // Contains data about upgrade name, etc.
                upgradeData_t upgrade = upgradeData[tower->archetype][i][tower->upgrades[i]];

                if(tower->upgrades[i] < 4 && ((char*)&upgrade)[0] != 0) {
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
        
                    printWithNewlines(upgrade.description, 5, baseY + 42);
                } else {
                    gfx_PrintStringXY("All ", 5, baseY + 24);
                    gfx_PrintString(statNames[i]);
                    gfx_PrintString("upgrades unlocked");
                }
            }
        }

        gfx_SetColor(BLACK);

        // Show the target switches if tower type is not burst
        if(tower->archetype != BURST) {
            gfx_HorizLine(0, LCD_HEIGHT - F_BTN_HEIGHT, LCD_WIDTH);
            for(i = 0; i < 4; i++) {
                drawButton(i, i, i == tower->targetType, tgtNames[i]);
            }
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

// TODO: light grey background
archetype_t selectTowerType(void) {
    // Corners of the dialog box
    const uint8_t width = LCD_WIDTH / 3;
    const uint8_t left = (LCD_WIDTH - width) / 2;
    const uint8_t height = 60;
    const uint8_t top = (LCD_HEIGHT - height) / 2;
    // Which type is selected
    int8_t selection = 0;

    while(true) {
        uint8_t i;

        kb_Scan();

        // Handle keypresses
        if(kb_IsDown(kb_Key2nd) || kb_IsDown(kb_KeyClear)) {
            while(kb_IsDown(kb_Key2nd) || kb_IsDown(kb_KeyClear)) kb_Scan();
            return selection;
        }
        if(kb_IsDown(kb_KeyUp)) {
            while(kb_IsDown(kb_KeyUp)) kb_Scan();
            if(--selection < 0)  selection = 2;
        }
        if(kb_IsDown(kb_KeyDown)) {
            while(kb_IsDown(kb_KeyDown)) kb_Scan();
            if(++selection >= 3) selection = 0;
        }

        // Fill background
        gfx_SetColor(WHITE);
        gfx_FillRectangle(left, top, width, height);
        // Draw border
        gfx_SetColor(BLACK);
        gfx_Rectangle(left, top, width, height);

        gfx_SetTextFGColor(BLACK);
        gfx_PrintStringXY("Select Type:", left + 2, top + 2);

        // Draw the names of things
        for(i = 0; i < 3; i++) {
            gfx_SetTextFGColor(i == selection ? HIGHLIGHT_COLOR : BLACK);
            gfx_PrintStringXY(archNames[i], left + 2, top + 20 + i * 10);
        }

        gfx_BlitBuffer();
    }
}

// Used as a callback for the reader
bool viewQR(gfx_sprite_t *sprite) {
    // QR codes are square, so we can just use the width
    uint8_t size = sprite->width;
    // Maximum size we can scale the image without making things non-square
    // or going off the top of the screen
    uint8_t scale = LCD_HEIGHT / size;

    uint24_t x = (LCD_WIDTH - scale * size) / 2;
    uint8_t y = (LCD_HEIGHT - scale * size) / 2;

    gfx_FillScreen(WHITE);
    gfx_ScaledSprite_NoClip(sprite, x, y, scale, scale);

    // Wait for a keypress
    while(!os_GetCSC());
    // Don't exit the reader
    return false;
}
