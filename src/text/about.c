#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

#include "../globals.h"
#include "../util.h"
#include "../reader.h"
#include "../gfx/colors.h"
#include "../gfx/group_about.h"

bool viewQR(gfx_sprite_t *sprite);

const text_t rf_text_about[] = {
    {4, "Made by commandblockguy"},
    {5, "This program was written as an entry in"},
    {6, "Cemetech Contest 22."},
    {10, "Ask on the topic thread:"},
    {11, "     https://ceme.tech/p278115"},
    {13, "Or, join the Cemetech Discord!"},
    {14, "     https://discord.gg/DZbmraw"},
    {16, "Github repository:"},
    {17, "     https://git.io/fjsOO"},
    {21, "Made by commandblockguy"},
    {22, "epsilon5, for the enemy sprite"},
    {23, "_iPhoenix_, for running CC22"},
    {24, "MateoC, for the CE C toolchain"},
    {26, VERSION_STRING}
};

const fancyText_t rf_fancy_about[] = {
    {1, "About", 2, WHITE, 3, 3},
    {8, "Questions?", 2, WHITE, 2, 2},
    {19, "Credits:", 2, WHITE, 2, 2},
};

const image_t rf_images_about[] = {
    {0, LCD_WIDTH - contest_scaled_width, contest_scaled}
};

const link_t rf_links_about[] = {
    {11, 1, HIGHLIGHT_COLOR, &viewQR, qr_topic},
    {14, 1, HIGHLIGHT_COLOR, &viewQR, qr_discord},
    {17, 1, HIGHLIGHT_COLOR, &viewQR, qr_github}
};

const readerFile_t rf_about = {
    30,
    BLACK,
    WHITE,
    &rf_text_about, elems(rf_text_about),
    &rf_fancy_about, elems(rf_fancy_about),
    &rf_images_about, elems(rf_images_about),
    &rf_links_about, elems(rf_links_about)
};