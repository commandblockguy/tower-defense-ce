#ifndef H_READER
#define H_READER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

typedef struct Text {
    uint24_t line;
    char *text;
} text_t;

typedef struct FancyText {
    uint24_t line;
    char *text;
    uint24_t x;
    uint8_t color;
    uint8_t sizeX;
    uint8_t sizeY;
} fancyText_t;

typedef struct Image {
    uint24_t line;
    uint24_t x;
    gfx_sprite_t *sprite;
} image_t;

// Links should never overlap
// Callback should return true to exit the reader
// or false to continue reading
typedef struct Link {
    uint24_t line;
    uint8_t height;
    uint8_t color;
    bool (*callback)(void *data);
    void *data;
} link_t;

// Pointers are to lists of elements sorted by line
typedef struct ReaderFile {
    uint24_t lines;
    uint8_t bgColor;
    uint8_t textColor;
    text_t *texts;
    uint24_t numTexts;
    fancyText_t *fancyTexts;
    uint24_t numFancy;
    image_t *images;
    uint24_t numImages;
    link_t *links;
    uint24_t numLinks;
} readerFile_t;

// It is the caller's responsibility to set up the palettes and such
void reader(readerFile_t *file);

#endif
