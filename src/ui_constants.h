#ifndef UI_CONSTANTS_H
#define UI_CONSTANTS_H

// the UI is designed at this resolution and scaled from it, it's also the initial window size
enum {
    DESIGN_WIDTH = 960,
    DESIGN_HEIGHT = 540
};

enum {
    FONT_ID_LARGE = 0,
    FONT_ID_SMALL = 1,
    FONT_COUNT = 2
};

typedef enum {
    Z_LAYER_GHOST_TILE = 1,
    Z_LAYER_LIVE_TILE = 2,
    Z_LAYER_OVERLAY = 10
} ZLayer;

static const float TILE_SLIDE_DURATION = 0.12f;
static const float OVERLAY_FADE_DURATION = 0.4f;

#endif
