// convpng v7.0
// this file contains all the graphics sources for easy inclusion in a project
#ifndef __gfx_group__
#define __gfx_group__
#include <stdint.h>

#define gfx_group_transparent_color_index 0

#define cursor_width 7
#define cursor_height 11
#define cursor_size 79
extern uint8_t cursor_data[79];
#define cursor ((gfx_sprite_t*)cursor_data)
#define sizeof_gfx_group_pal 50
extern uint16_t gfx_group_pal[25];

#endif
