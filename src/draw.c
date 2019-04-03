#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

#include "gfx/colors.h"
#include "globals.h"

void drawButton(uint8_t posMin, uint8_t posMax, char *str);

void drawTowers(uint24_t csrX, uint8_t csrY) {
    // Loop through all towers
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
    static int i;
    gfx_SetColor(PATH_COLOR);
    // Loop through all lines
    for(i = 0; i < bufSize - 1; i++) {
    	// Check if there is an error
    	if()
    }
}

void drawUI(void) {
    // Draw the F buttons

    gfx_SetColor(BLACK);
    gfx_SetTextFGColor(BLACK);

    switch(game.status) {
        case(PRE_WAVE):
            drawButton(2, 2, "Edit Path");
            drawButton(3, 4, "Start Wave");
            break;
        case(PAUSED):
            drawButton(4, 4, "Resume");
            break;
        case(WAVE):
            drawButton(4, 4, "Pause");
            break;
        case(PATH_EDIT):
            drawButton(1, 1, "Reverse");
            drawButton(2, 2, "Clear");
            drawButton(3, 3, "Cancel");
            drawButton(4, 4, "Save");
    }
}

// posMin-posMax: range 0-4, represents which button
void drawButton(uint8_t posMin, uint8_t posMax, char *str) {
    // Calculate edges
    uint24_t leftEdge = posMin * (LCD_WIDTH / 5);
    uint24_t rightEdge = (posMax + 1) * (LCD_WIDTH / 5);
    uint24_t textX;
    // Draw the line on top
    gfx_HorizLine(leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, rightEdge - leftEdge);
    //Draw the lines on either side
    gfx_VertLine( leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);
    gfx_VertLine(rightEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);

    // Draw the text
    textX = (leftEdge + rightEdge - gfx_GetStringWidth(str)) / 2 + 1;
    gfx_PrintStringXY(str, textX, LCD_HEIGHT - (F_BTN_HEIGHT + TEXT_HEIGHT) / 2);

}
