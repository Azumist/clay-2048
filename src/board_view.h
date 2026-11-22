#ifndef BOARD_VIEW_H
#define BOARD_VIEW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "board.h"
#include "clay.h"
#include "ui_context.h"

// visual metrics of the board, it does not reflect the game rules e.g. the number of tiles
typedef struct {
    float gridSize;
    float gridCornerRadius;
    float scoreHeight;
    float tileWidth;
    float tileCornerRadius;
    uint16_t tileGap;
} BoardMetrics;

typedef struct {
    Clay_String id_name;
    Clay_String title;
    Clay_String hint;
    Clay_Color background_color;
    Clay_Color text_color;
} OverlayContent;

extern const OverlayContent WIN_OVERLAY;
extern const OverlayContent LOSE_OVERLAY;

BoardMetrics ComputeBoardMetrics(size_t sideSize, float screenWidth, float screenHeight);
void GameScreen(UiContext *ui, const Board *board, bool isBoardAnimating);

#endif
