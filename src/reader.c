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

#include "reader.h"

#define MAX_VELOCITY 10
#define LINE_HEIGHT 10
#define SCREEN_LINES (LCD_HEIGHT / LINE_HEIGHT)

// Compatibility for toolchain versions
#ifndef kb_IsDown
#define kb_IsDown(lkey) \
(kb_Data[(lkey) >> 8] & (lkey))
#endif

uint24_t firstAfter(uint24_t target, void *ptr, uint24_t n, size_t size);
void processLinkClick(link_t *link);

const kb_lkey_t numKeys[10] = {
    kb_Key0,
    kb_Key1,
    kb_Key2,
    kb_Key3,
    kb_Key4,
    kb_Key5,
    kb_Key6,
    kb_Key7,
    kb_Key8,
    kb_Key9,
};

void reader(readerFile_t *file) {
    // The first line visible on the screen
    int24_t currentLine = 0;
    int8_t velocity = 0;

    do {
        int24_t i;

        // Scan the keypad
        kb_Scan();

        // Handle scrolling
        if(file->lines > SCREEN_LINES) {
            if(kb_IsDown(kb_KeyDown)) {
                velocity++;
            } else if(kb_IsDown(kb_KeyUp)) {
                velocity--;
            } else {
                // Stop motion if no key is held
                velocity = 0;
            }
            currentLine += velocity;

            if(velocity >  MAX_VELOCITY) velocity =  MAX_VELOCITY;
            if(velocity < -MAX_VELOCITY) velocity = -MAX_VELOCITY;

            if(currentLine < 0) currentLine = 0;
            if(currentLine + SCREEN_LINES > file->lines)
                currentLine = file->lines - SCREEN_LINES;

        } else {
            currentLine = 0;
        }

        // Graphics stuff

        gfx_FillScreen(file->bgColor);

        gfx_SetTextScale(1, 1);
        gfx_SetTextFGColor(file->textColor);
        if(file->numTexts) {
            for(i = firstAfter(currentLine, file->texts, file->numTexts, sizeof(file->texts[0]));
                i < file->numTexts && file->texts[i].line < currentLine + SCREEN_LINES; i++) {

                gfx_PrintStringXY(file->texts[i].text, 2, (file->texts[i].line - currentLine) * LINE_HEIGHT);
            }
        }

        if(file->numFancy) {
            for(i = firstAfter(currentLine, file->fancyTexts, file->numFancy, sizeof(file->fancyTexts[0]));
                i < file->numFancy && file->fancyTexts[i].line < currentLine + SCREEN_LINES; i++) {

                fancyText_t *elem = &file->fancyTexts[i];

                gfx_SetTextFGColor(elem->color);
                gfx_SetTextScale(elem->sizeX, elem->sizeY);
                gfx_PrintStringXY(elem->text, elem->x, (elem->line - currentLine) * LINE_HEIGHT);
            }
        }

        if(file->numImages) {
            for(i = firstAfter(currentLine, file->images, file->numImages, sizeof(file->images[0]));
                i < file->numImages && file->images[i].line < currentLine + SCREEN_LINES; i++) {

                image_t *elem = &file->images[i];

                gfx_Sprite(elem->sprite, elem->x, (elem->line - currentLine) * LINE_HEIGHT);
            }
        }

        // TODO: make it easier to click multiple links on one screen
        if(file->numLinks) {
            // The button the user needs to press to click the link
            uint8_t number = 1;

            for(i = firstAfter(currentLine, file->links, file->numLinks, sizeof(file->links[0]));
                i < file->numLinks && file->links[i].line < currentLine + SCREEN_LINES; i++) {
                link_t *link = &file->links[i];

                gfx_SetTextXY(1, (file->links[i].line - currentLine) * LINE_HEIGHT);
                gfx_SetTextScale(1, 1);
                gfx_SetTextFGColor(file->links[i].color);
                gfx_PrintUInt(number, 1);
                gfx_PrintChar('>');

                // Check if the key is pressed
                if(kb_IsDown(numKeys[number])) {
                    // Wait for key to be released
                    while(kb_IsDown(numKeys[number])) kb_Scan();
                    // Call the callback if it exists
                    if(link->callback)
                        (link->callback)(link->data);
                }

                number++;
            }
        }

        gfx_BlitBuffer();


    } while(!kb_IsDown(kb_KeyClear));
}

// Returns the index of the first element of the array at ptr, which has n elements of size size,
// whose first three bytes are greater than or equal to target
// Should operate in O(log(n)) time (TODO: confirm)
uint24_t firstAfter(uint24_t target, void *ptr, uint24_t n, size_t size) {
    int24_t low = 0, high = n;
    //dbg_sprintf(dbgout, "First element after %u in %p:\n", target, ptr);
    while (low != high) {
        uint24_t mid = (low + high) / 2;
        // You know that bit where I said "I sincerely hope this is the worst part of the code?"
        // Well, this next line here is giving it a run for its money.
        uint24_t val = *(uint24_t*)((uint24_t)ptr + mid * size);
        //dbg_sprintf(dbgout, "Element %u (%p): %u\n", mid, (uint24_t)ptr + mid * size, val);
        if(val < target) {
            low = mid + 1;
            //dbg_sprintf(dbgout, "Low: %i\n", low);
        }
        else {
            high = mid;
            //dbg_sprintf(dbgout, "High: %i\n", high);
        }
    }
    //dbg_sprintf(dbgout, "Returning: %i\n", low);
    return low;
}
