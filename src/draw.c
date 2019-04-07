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
#include "util.h"

void drawButton(uint8_t posMin, uint8_t posMax, char *str);
void drawTower(uint8_t index, uint8_t scale);

void drawTowers(uint24_t csrX, uint8_t csrY) {
    // Loop through all towers
    int i;
    for(i = 0; i < NUM_TOWERS; i++) {
    	if(csrX == -1) {
    		drawTower(i, 64);
    		// Skip the distance check if it's not necessary
    		continue;
    	}
    	// Enlarge the tower if the cursor is close enough
    	if(distBetween(towers[i].posX, towers[i].posY, csrX, csrY) < TOWER_RADIUS + CLICK_RADIUS) {
    		drawTower(i, 128);
    	} else {
    		drawTower(i, 64);
    	}
    }
}

// Uses the same scale as gfx_ScaledSprite
void drawTower(uint8_t index, uint8_t scale) {
	// TODO: rewrite
	gfx_SetColor(RED);
	gfx_Circle(towers[index].posX, towers[index].posY, scale / 8);
}

void drawPath(void) {
	//temp
	//circle_t c;
    // Loop through all path points
    int i;

    //c.radius = 25;
    //c.x = LCD_WIDTH / 2;
    //c.y = LCD_HEIGHT / 2;

    gfx_SetColor(PATH_COLOR);
    for(i = 1; i < game.numPathPoints; i++) {
    	//lineSeg_t ls1, ls2;
        gfx_Line(path[i].posX, path[i].posY, path[i - 1].posX, path[i - 1].posY);
        /*ls1.x1 = path[i].posX;
        ls1.y1 = path[i].posY;
        ls1.x2 = path[i-1].posX;
        ls1.y2 = path[i-1].posY;
        if(circCollidesSeg(&c, &ls1, &ls2)) {
        	gfx_SetColor(RED);
        	gfx_Line(ls2.x1, ls2.y1, ls2.x2, ls2.y2);
        	gfx_SetColor(PATH_COLOR);
        }*/
    }
}

void drawPathBuffer(void) {
    static int i;
    gfx_SetColor(PATH_COLOR);
    // Loop through all lines
    for(i = 0; i < bufSize - 1; i++) {
        // Check if there is an error
        if(pathBufErr[i / 8] & (1 << (i % 8))) {
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

    gfx_SetColor(BLACK);
    gfx_SetTextFGColor(BLACK);

    gfx_HorizLine(0, LCD_HEIGHT - F_BTN_HEIGHT, LCD_WIDTH);

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
    //gfx_HorizLine(leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, rightEdge - leftEdge);
    //Draw the lines on either side
    gfx_VertLine( leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);
    gfx_VertLine(rightEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);

    // Draw the text
    textX = (leftEdge + rightEdge - gfx_GetStringWidth(str)) / 2 + 1;
    gfx_PrintStringXY(str, textX, LCD_HEIGHT - (F_BTN_HEIGHT + TEXT_HEIGHT) / 2);

}
