#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

#include "../util.h"
#include "../reader.h"
#include "../gfx/colors.h"
#include "../gfx/gfx_group.h"

bool viewQR(gfx_sprite_t *sprite);

const text_t rf_text_test[] = {
    {0, "Line 1"},
    {1, "Line 2"},
    {2, "Line 3"},
    {3, "Line 4"},
    {4, "Line 5"},
    {5, "Line 6"},
    {6, "Line 7"},
    {7, "Line 8"}
};

const fancyText_t rf_fancy_test[] = {
    {8, "Red 9", 10, RED, 1, 1},
    {9, "BIG 10", 2, WHITE, 2, 2},
    {11, "HUGE 12", 2, WHITE, 3, 3}
};

const image_t rf_images_test[] = {
    {14, 2, cursor}
};

const link_t rf_links_test[] = {
    {5, 3, HIGHLIGHT_COLOR, &viewQR, cursor}
};

const readerFile_t rf_test = {
    30,
    BLACK,
    WHITE,
    &rf_text_test, elems(rf_text_test),
    &rf_fancy_test, elems(rf_fancy_test),
    &rf_images_test, elems(rf_images_test),
    &rf_links_test, elems(rf_links_test)
};
