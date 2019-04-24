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

// TODO: improve

const text_t rf_text_howto[] = {
    {2, "In Tower Defense CE, you control the path that"},
    {3, "enemies take. Towers are in fixed locations,"},
    {4, "and cannot be bought, sold, or moved."},
    {5, "There is no money system in TD-CE."},
    {6, "Instead, towers gain experience from dealing"},
    {7, "damage to enemies and level up."},
    {8, "You can spend these levels to upgrade towers"},
    {9, "from the tower's menu, which can be opened by"},
    {10, "clicking on the tower."},
    {11, "Towers will gain more experience the shorter"},
    {12, "the path is. You can edit the path between"},
    {13, "waves by pressing Edit Path."},
    {14, "The path editor has its own help page."},
    {18, "Arrow keys: Move cursor or scroll"},
    {19, "2nd: Click"},
    {20, "y= through graph: F-key shortcuts"},
    {21, "Clear: Save (where applicable) and exit"}
};

const fancyText_t rf_fancy_howto[] = {
    {0, "How-To", 2, WHITE, 2, 2},
    {16, "Controls", 2, WHITE, 2, 2}
};

const readerFile_t rf_howto = {
    30,
    BLACK,
    WHITE,
    &rf_text_howto, elems(rf_text_howto),
    &rf_fancy_howto, elems(rf_fancy_howto),
    NULL, 0,
    NULL, 0
};