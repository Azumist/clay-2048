#ifndef COLORS_H
#define COLORS_H

#include "clay.h"

static const Clay_Color COLOR_WIN_OVERLAY = (Clay_Color) { 237, 194, 46, 128 };
static const Clay_Color COLOR_WIN_TEXT    = (Clay_Color) { 249, 246, 242, 255 };

static const Clay_Color COLOR_LOSE_OVERLAY = (Clay_Color) { 238, 228, 218, 186 }; 
static const Clay_Color COLOR_LOSE_TEXT    = (Clay_Color) { 119, 110, 101, 255 };

static const Clay_Color COLOR_PAGE_BG = (Clay_Color) { 250, 248, 239, 255 };
static const Clay_Color COLOR_BOARD_BG = (Clay_Color) { 187, 173, 160, 255 };

static const Clay_Color COLOR_EMPTY = (Clay_Color) { 220, 209, 198, 255 };
static const Clay_Color TILE_BG_COLORS[] = {
    (Clay_Color) { 238, 228, 218, 255 },    // 2
    (Clay_Color) { 237, 224, 200, 255 },    // 4
    (Clay_Color) { 242, 177, 121, 255 },    // 8
    (Clay_Color) { 245, 149, 99, 255 },     // 16
    (Clay_Color) { 246, 124, 95, 255 },     // 32
    (Clay_Color) { 246, 94, 59, 255 },      // 64
    (Clay_Color) { 237, 207, 114, 255 },    // 128
    (Clay_Color) { 237, 204, 97, 255 },     // 256
    (Clay_Color) { 237, 200, 80, 255 },     // 512
    (Clay_Color) { 237, 197, 63, 255 },     // 1024
    (Clay_Color) { 237, 197, 46, 255 },    // 2048
    (Clay_Color) { 60, 58, 50, 255 },      // above
};

#define TILE_BG_COLOR_COUNT (sizeof(TILE_BG_COLORS) / sizeof(TILE_BG_COLORS[0]))

static const Clay_Color COLOR_TEXT_BELOW_8 = (Clay_Color) { 119, 110, 101, 255 };
static const Clay_Color COLOR_TEXT_ABOVE_8 = (Clay_Color) { 249, 246, 242, 255 };

static const Clay_Color COLOR_BUTTON_BG = (Clay_Color) { 220, 209, 198, 255 };
static const Clay_Color COLOR_BUTTON_BG_HOVER = (Clay_Color) { 212, 199, 185, 255 };
static const Clay_Color COLOR_BUTTON_TEXT = (Clay_Color) { 119, 110, 101, 255 };
static const Clay_Color COLOR_BUTTON_TEXT_HOVER = (Clay_Color) { 119, 110, 101, 255 };

#endif