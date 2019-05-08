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

const text_t rf_text_pathedit[] = {
    {3, "The path editor allows you to"},
    {4, "reshape the path that enemies take."},
    {5, "Shorter paths give you more XP, while"},
    {6, "longer paths can go by more towers."},
    {7, "The path cannot cross itself, nor can"},
    {8, "it get too close to a tower. If a"},
    {9, "segment of a line is invalid, it will"},
    {10, "turn red, indicating that it is preventing"},
    {11, "you from starting a wave."},
    {14, "2nd: Split a line into two smaller ones,"},
    {15, "  picking up the newly created point"},
    {16, "2nd: Drop the carried point"},
    {17, "Alpha: Pick up and move a point"},
    {18, "del: Delete a point"},
    {19, "clear: Exit without changing the path"},
    {21, "The first and last points on the line will"},
    {22, "snap to the edges of the screen."},
    {26, "Help: Open this menu"},
    {27, "Reverse: Enemies will switch direction"},
    {28, "Clear: Reset the path to the original"},
    {29, "Cancel: Exit without changing the path"},
    {30, "Save: Save the path so it can be used"}
};

const fancyText_t rf_fancy_pathedit[] = {
    {1, "Help: Path Edit", 2, WHITE, 2, 2},
    {12, "Controls:", 2, WHITE, 2, 2},
    {24, "Buttons:", 2, WHITE, 2, 2}
};

const readerFile_t rf_pathedit = {
    31,
    BLACK,
    WHITE,
    &rf_text_pathedit, elems(rf_text_pathedit),
    &rf_fancy_pathedit, elems(rf_fancy_pathedit),
    NULL, 0,
    NULL, 0
};