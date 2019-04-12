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

// Various draw and menu functions
// TODO: rename to graphics.c?

void drawButton(uint8_t posMin, uint8_t posMax, char *str);
void drawTower(tower_t *tower, uint8_t scale, bool range);

// from main.c
int24_t play(void);

// TODO: add some sort of fancy background

void drawTowers(uint24_t csrX, uint8_t csrY) {
    // Loop through all towers
    int i;
    for(i = 0; i < NUM_TOWERS; i++) {
    	if(csrX == -1) {
    		drawTower(&towers[i], 64, false);
    		// Skip the distance check if it's not necessary
    		continue;
    	}
    	// Enlarge the tower if the cursor is close enough
    	if(distBetween(towers[i].posX, towers[i].posY, csrX, csrY) < TOWER_RADIUS + CLICK_RADIUS) {
    		drawTower(&towers[i], 128, true);
    	} else {
    		drawTower(&towers[i], 64, false);
    	}
    }
}

// Uses the same scale as gfx_ScaledSprite
// Range: true if the tower range should be displayed
void drawTower(tower_t *tower, uint8_t scale, bool range) {
	// TODO: rewrite using the actual sprites
	gfx_SetColor(RED);
	gfx_Circle(tower->posX, tower->posY, scale / 8);

	// Draw the tower range
	if(range) {
		gfx_SetColor(RED);
		gfx_Circle(tower->posX, tower->posY, tower->range);
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
            drawButton(0, 0, "Help");
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
    // Draw the line on top //TODO: remove
    //gfx_HorizLine(leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, rightEdge - leftEdge);
    //Draw the lines on either side
    gfx_VertLine( leftEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);
    gfx_VertLine(rightEdge, LCD_HEIGHT - F_BTN_HEIGHT, F_BTN_HEIGHT);

    // Draw the text
    textX = (leftEdge + rightEdge - gfx_GetStringWidth(str)) / 2 + 1;
    gfx_PrintStringXY(str, textX, LCD_HEIGHT - (F_BTN_HEIGHT + TEXT_HEIGHT) / 2);

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
