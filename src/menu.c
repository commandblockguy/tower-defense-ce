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
#include "gfx/group_about.h"
#include "globals.h"
#include "util.h"
#include "enemy.h"
#include "tower.h"
#include "graphics.h"
#include "menu.h"
#include "reader.h"

#define CSR_SPEED (ONE_SECOND * 256 / 64)

// from main.c
int24_t play(bool resume);
bool appvarExists(void);
int8_t addScore(uint24_t score);
void initScores(void);

extern const char *statNames[];

extern const readerFile_t rf_howto;
extern const readerFile_t rf_about;

void mainMenu(void) {
    bool saveExists = appvarExists();
    int8_t selection = !saveExists;

    // TODO: setup graphics

    // Start the timer
    timer_Control &= ~TIMER1_ENABLE;
    timer_1_Counter = 0;
    timer_Control |= TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;

    // Menu loop
    while(true) {
        uint8_t i;
        const char* title = "tower defense CE";

        const uint8_t NUM_OPTIONS = 6;
        // Remember to update this next line too; the compiler is stupid
        const char* options[6] = {
            "Resume",
            "New Game",
            "Scores",
            "How-To",
            "About",
            "Exit"
        };

        kb_Scan();

        // Handle keyboard inputs
        if(kb_IsDown(kb_KeyClear)) {
            // Exit the main menu
            break;
        }

        if(kb_IsDown(kb_KeyUp)) {
            while(kb_IsDown(kb_KeyUp)) kb_Scan();
            if(--selection < !saveExists) selection = NUM_OPTIONS - 1;
        }
        if(kb_IsDown(kb_KeyDown)) {
            while(kb_IsDown(kb_KeyDown)) kb_Scan();
            if(++selection > NUM_OPTIONS - 1) selection = !saveExists;
        }

        if(kb_IsDown(kb_Key2nd)) {
            while(kb_IsDown(kb_Key2nd)) kb_Scan();
            switch(selection) {
                case(0): // Resume
                    if(saveExists) {
                        play(true);
                        saveExists = appvarExists();
                    }
                    break;
                case(1): // New Game
                    play(false);
                    saveExists = appvarExists();
                    break;
                case(2): // High scores
                    highScores();
                    break;
                case(3): // How-to
                    reader(&rf_howto);
                    break;
                case(4): // About
                    gfx_SetPalette(group_about_pal, sizeof_group_about_pal, 0);
                    reader(&rf_about);
                    gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal, 0);
                    break;
                case(5): // Exit
                    return;
            }
        }

        // TODO: Render fancy menu background
        // Render hills at various distances using parallax
        // This may cause minor visual glitches if you leave
        // the main menu open for an inordinate amount of time
        // ticks per pixel = distance from camera / speed 
        // x = (timer / ticks per pixel) % LCD_WIDTH
        gfx_FillScreen(HIGHLIGHT_COLOR);

        // Since hills aren't happening on this timescale let's go for a pleasant green triangle instead
        gfx_SetColor(GRASS);
        gfx_FillRectangle(0, LCD_HEIGHT / 2, LCD_WIDTH, LCD_HEIGHT / 2);
        gfx_FillTriangle(0, LCD_HEIGHT / 3, 0, LCD_HEIGHT / 2, LCD_WIDTH, LCD_HEIGHT / 2);
        gfx_SetColor(BLACK);
        gfx_Line(0, LCD_HEIGHT / 3, LCD_WIDTH, LCD_HEIGHT / 2 + 1);

        gfx_ScaledTransparentSprite_NoClip(castle, 5, LCD_HEIGHT / 5, 3, 3);

        // Path thing
        gfx_SetColor(PATH_COLOR);
        gfx_Line(5 + 3 * castle->width / 2, LCD_HEIGHT / 5 + 3 * castle->height, LCD_WIDTH / 2, LCD_HEIGHT);

        gfx_SetTextScale(2, 2);
        gfx_SetTextFGColor(WHITE);
        gfx_PrintStringXY(title, (LCD_WIDTH - gfx_GetStringWidth(title)) / 2, 8);

        // Draw the options
        gfx_SetTextScale(1, 1);
        gfx_SetTextFGColor(BLACK);
        for(i = 0; i < NUM_OPTIONS; i++) {
            const uint8_t boxSpacing = 5;
            const uint8_t boxHeight = (2 * LCD_HEIGHT / 3 + boxSpacing) / NUM_OPTIONS - boxSpacing;
            const uint8_t boxWidth = LCD_WIDTH / 4;
            uint8_t textWidth;
            // TODO: maybe some gravity / bounciness to this?
            uint8_t boxPosY = LCD_HEIGHT / 6 + (boxHeight + boxSpacing) * i;

            gfx_SetColor(WHITE);
            gfx_FillRectangle((LCD_WIDTH - boxWidth) / 2, boxPosY, boxWidth, boxHeight);
            gfx_SetColor(BLACK);
            gfx_Rectangle((LCD_WIDTH - boxWidth) / 2, boxPosY, boxWidth, boxHeight);

            // TODO: sign colors
            if(i == selection) gfx_SetTextFGColor(HIGHLIGHT_COLOR);
            else if(i == 0 && !saveExists) gfx_SetTextFGColor(LIGHT_GREY);
            else gfx_SetTextFGColor(BLACK);

            textWidth = gfx_GetStringWidth(options[i]);

            gfx_SetTextXY((LCD_WIDTH - textWidth) / 2, boxPosY + (boxHeight - TEXT_HEIGHT) / 2);
            gfx_PrintString(options[i]);
        }
        

        gfx_BlitBuffer();
    }
}

void highScores(void) {
    ti_var_t slot;
    uint8_t i;
    uint24_t scores[NUMSCORES];
    const char* text_scores = "high scores";
    uint8_t width;
    // Display high scores

    gfx_FillScreen(GRASS);

    // Graphics stuff
    gfx_SetTextFGColor(WHITE);
    gfx_SetColor(WHITE);
    gfx_SetTextScale(3, 3);
    width = gfx_GetStringWidth(text_scores);
    gfx_PrintStringXY(text_scores, (LCD_WIDTH - width) / 2, 16);
    gfx_HorizLine((LCD_WIDTH - width) / 2, 16 + 3 * 8 - 2, width);
    gfx_SetTextScale(1, 1);

    // Open high score appvar
    initScores();
    slot = ti_Open(SCOREVAR, "r");
    if(!slot) return;

    // Read list of scores
    ti_Read(scores, sizeof(scores[0]), NUMSCORES, slot);
    ti_Close(slot);

    // Display names
    for(i = 0; i < NUMSCORES; i++) {
        gfx_PrintStringXY("#", LCD_WIDTH / 6, LCD_HEIGHT / 3 + 12 * i);
        gfx_PrintUInt(i + 1, 1);
        gfx_PrintString(": ");
        gfx_PrintUInt(scores[i], 4);
    }

    gfx_BlitBuffer();

    // Wait for keypress
    while(os_GetCSC());
    while(!os_GetCSC());
}

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
        gfx_FillScreen(WHITE);

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
    const uint8_t border = 25;
    // QR codes are square, so we can just use the width
    uint8_t size = sprite->width;
    // Maximum size we can scale the image without making things non-square
    // or going off the top of the screen
    uint8_t scale = (LCD_HEIGHT - border * 2) / size;

    uint24_t x = (LCD_WIDTH - scale * size) / 2;
    uint8_t y = (LCD_HEIGHT - scale * size) / 2;

    gfx_FillScreen(WHITE);
    gfx_ScaledSprite_NoClip(sprite, x, y, scale, scale);

    gfx_BlitBuffer();

    // Wait for a keypress
    while(!os_GetCSC());

    // Wait for key to be released
    while(os_GetCSC());
    // Don't exit the reader
    return false;
}

void initCursor(void) {
    timer_Control &= ~TIMER2_ENABLE;
    timer_2_Counter = 0;
    timer_Control |= TIMER2_ENABLE | TIMER2_32K | TIMER2_UP;
}

void updateCursor(void) {
    // Move the cursor around
    // TODO: acceleration?
    //  (I always say this and then never do it)
    kb_key_t keys;
    uint24_t dist;

    // Arrow keys
    keys = kb_Data[7];

    // Enable the timer
    timer_Control &= ~TIMER2_ENABLE;

    //dbg_sprintf(dbgout, "Updating cursor pos\n");
    //dbg_sprintf(dbgout, "Speed is %u, timer is %u\n", CSR_SPEED, timer_2_Counter);

    dist = timer_2_Counter / CSR_SPEED;

    //dbg_sprintf(dbgout, "Dist: %u\n", dist);

    if(keys && dist && timer_2_Counter < ONE_SECOND * 256) {
        //dbg_sprintf(dbgout, "Key is pressed or whatever\n");

        // Process individual keys
        if(keys & kb_Left ) csrX -= dist;
        if(keys & kb_Right) csrX += dist;
        if(keys & kb_Up   ) csrY -= dist;
        if(keys & kb_Down ) csrY += dist;

        // Check if the cursor went offscreen
        if(csrX > 500)        csrX += LCD_WIDTH ; // hacky check for underflow
        if(csrX > LCD_WIDTH ) csrX -= LCD_WIDTH ;
        if(csrY > 250)        csrY += LCD_HEIGHT; // hacky check for  overflow
        if(csrY > LCD_HEIGHT) csrY -= LCD_HEIGHT;

    }

    timer_2_Counter -= CSR_SPEED * dist;
    timer_Control |= TIMER2_ENABLE;
}

void loseScreen(uint24_t score) {
    const char *text_lose = "You Lose!";
    const char *text_score = "Score: ";
    int8_t rank = addScore(score) + 1;

    gfx_FillScreen(BLACK);

    gfx_SetTextScale(3, 3);
    gfx_SetTextFGColor(RED);
    gfx_PrintStringXY(text_lose, (LCD_WIDTH - gfx_GetStringWidth(text_lose)) / 2, LCD_HEIGHT / 3 - 8 * 3 / 2);

    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(WHITE);
    gfx_PrintStringXY(text_score, (LCD_WIDTH - gfx_GetStringWidth(text_score) - gfx_GetStringWidth("000")) / 2, 2 * LCD_HEIGHT / 3);
    gfx_PrintUInt(score, 3);

    if(rank) {
        const char *text_high = "New high score!";
        const char *text_rank = "You are now ranked #";
        gfx_PrintStringXY(text_high, (LCD_WIDTH - gfx_GetStringWidth(text_high)) / 2, 2 * LCD_HEIGHT / 3 + 10);
        gfx_PrintStringXY(text_rank, (LCD_WIDTH - gfx_GetStringWidth(text_rank)) / 2, 2 * LCD_HEIGHT / 3 + 20);
        gfx_PrintUInt(rank, 1);
    }

    gfx_BlitBuffer();

    while(os_GetCSC());
    while(!os_GetCSC());
}

void popup(char* str) {
    uint8_t width;

    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(BLACK);

    width = gfx_GetStringWidth(str);

    gfx_SetColor(WHITE);
    gfx_FillRectangle((LCD_WIDTH - width - 10) / 2, (LCD_HEIGHT - 12) / 2, width + 10, 12);
    gfx_SetColor(BLACK);
    gfx_Rectangle((LCD_WIDTH - width - 10) / 2, (LCD_HEIGHT - 12) / 2, width + 10, 12);

    gfx_PrintStringXY(str, (LCD_WIDTH - width) / 2, (LCD_HEIGHT - 8) / 2);

    gfx_BlitRectangle(gfx_buffer, (LCD_WIDTH - width - 10) / 2, (LCD_HEIGHT - 12) / 2, width + 10, 12);

    // Wait for key to be pressed
    while(os_GetCSC());
    while(!os_GetCSC());
}
