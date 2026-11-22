#include "board_view.h"
#include "colors.h"
#include "labels.h"
#include "ui_constants.h"
#include <stdio.h>

enum { PAGE_PADDING = 16 };
enum { SCORE_GRID_GAP = 8 };

static const float GAP_TO_TILE_RATIO = 0.07f;
static const float RADIUS_TO_TILE_RATIO = 0.06f;
static const float SCORE_HEIGHT_RATIO = 0.08f;

const OverlayContent WIN_OVERLAY = {
    .id_name = CLAY_STRING_CONST("WinOverlay"),
    .title = CLAY_STRING_CONST("You win!"),
    .hint = CLAY_STRING_CONST("Click or press Enter to keep going"),
    .background_color = COLOR_WIN_OVERLAY,
    .text_color = COLOR_WIN_TEXT
};

const OverlayContent LOSE_OVERLAY = {
    .id_name = CLAY_STRING_CONST("LoseOverlay"),
    .title = CLAY_STRING_CONST("Game over"),
    .hint = CLAY_STRING_CONST("Click or press Enter to try again"),
    .background_color = COLOR_LOSE_OVERLAY,
    .text_color = COLOR_LOSE_TEXT
};

BoardMetrics ComputeBoardMetrics(size_t sideSize, float screenWidth, float screenHeight)
{
    BoardMetrics metrics;

    float availableWidth = screenWidth - 2.0f * PAGE_PADDING;
    float availableHeight = screenHeight - 2.0f * PAGE_PADDING;
    float maxGridSizeByHeight = (availableHeight - SCORE_GRID_GAP) / (1.0f + SCORE_HEIGHT_RATIO);
    metrics.gridSize = availableWidth < maxGridSizeByHeight ? availableWidth : maxGridSizeByHeight;
    metrics.scoreHeight = metrics.gridSize * SCORE_HEIGHT_RATIO;

    // the gap is rounded to whole pixels, so the tile width is derived from it afterwards
    float side = (float)sideSize;
    float idealTileWidth = metrics.gridSize / (side + (side + 1.0f) * GAP_TO_TILE_RATIO);
    metrics.tileGap = (uint16_t)(idealTileWidth * GAP_TO_TILE_RATIO + 0.5f);
    if (metrics.tileGap < 1)
    {
        metrics.tileGap = 1;
    }

    metrics.tileWidth = (metrics.gridSize - (side + 1.0f) * metrics.tileGap) / side;
    metrics.tileCornerRadius = metrics.tileWidth * RADIUS_TO_TILE_RATIO;
    metrics.gridCornerRadius = metrics.tileCornerRadius + metrics.tileGap;

    return metrics;
}

static uint16_t tile_font_size(float tileWidth, int value)
{
    float ratio = 0.5f; // 1-2 digits
    if (value >= 1000) { ratio = 0.3f; } // 4 digits
    else if (value >= 100) { ratio = 0.38f; } // 3 digits
    return (uint16_t)(tileWidth * ratio);
}

static Clay_TransitionData GrowTileStart(Clay_TransitionData initialState, Clay_TransitionProperty properties)
{
    Clay_TransitionData targetState = initialState;

    if (properties & CLAY_TRANSITION_PROPERTY_WIDTH)
    {
        targetState.boundingBox.x += initialState.boundingBox.width / 2.0f;
        targetState.boundingBox.width = 0.0f;
    }

    if (properties & CLAY_TRANSITION_PROPERTY_HEIGHT)
    {
        targetState.boundingBox.y += initialState.boundingBox.height / 2.0f;
        targetState.boundingBox.height = 0.0f;
    }

    return targetState;
}

static Clay_TransitionData FadeInFromTransparent(Clay_TransitionData targetState, Clay_TransitionProperty properties)
{
    Clay_TransitionData initialState = targetState;

    if (properties & CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR)
    {
        initialState.backgroundColor.a = 0.0f;
    }
    return initialState;
}

// number tiles float on top of the empty tile of their cell, so both need the same id
static Clay_ElementId empty_tile_id(size_t cellIndex)
{
    return CLAY_IDI("EmptyTile", (uint32_t)cellIndex);
}

static void GridEmptyTileComponent(size_t cellIndex, const BoardMetrics *metrics)
{
    CLAY(empty_tile_id(cellIndex), {
        .layout = {
            .sizing = { .width = CLAY_SIZING_FIXED(metrics->tileWidth), .height = CLAY_SIZING_FIXED(metrics->tileWidth) },
        },
        .backgroundColor = COLOR_EMPTY,
        .cornerRadius = CLAY_CORNER_RADIUS(metrics->tileCornerRadius)
    }) {}
}

static void GridNumberTileComponent(const UiContext *ui, int tileId, int displayValue, size_t cellIndex, ZLayer layer, const BoardMetrics *metrics)
{
    Clay_ElementId elementId = CLAY_IDI("NumberTile", (uint32_t)tileId);
    Clay_ElementData elementData = Clay_GetElementData(elementId);

    // size the text by the tile's current (animated) width, so it grows together with a spawning tile
    float currentTileWidth = elementData.found ? elementData.boundingBox.width : 0.0f;
    uint16_t fontSize = tile_font_size(currentTileWidth, displayValue);
    if (fontSize < 1)
    {
        fontSize = 1;
    }

    Clay_Color textColor = displayValue < 8 ? COLOR_TEXT_BELOW_8 : COLOR_TEXT_ABOVE_8;
    Clay_String text = TILE_LABELS[tile_value_rank(displayValue, TILE_LABEL_COUNT - 1)];

    Clay_Color backgroundColor = COLOR_EMPTY;
    if (displayValue >= 2)
    {
        backgroundColor = TILE_BG_COLORS[tile_value_rank(displayValue, TILE_BG_COLOR_COUNT - 1)];
    }

    CLAY(elementId, {
        .layout = {
            .sizing = { .width = CLAY_SIZING_FIXED(metrics->tileWidth), .height = CLAY_SIZING_FIXED(metrics->tileWidth) },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
        },
        .floating = {
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
            .parentId = empty_tile_id(cellIndex).id,
            .zIndex = (int16_t)layer
        },
        .transition = {
            .handler = Clay_EaseOut,
            .duration = ui->skipAnimations ? 0.0f : TILE_SLIDE_DURATION,
            .properties = CLAY_TRANSITION_PROPERTY_X
                        | CLAY_TRANSITION_PROPERTY_Y
                        | CLAY_TRANSITION_PROPERTY_WIDTH
                        | CLAY_TRANSITION_PROPERTY_HEIGHT
                        | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR,
            .interactionHandling = CLAY_TRANSITION_DISABLE_INTERACTIONS_WHILE_TRANSITIONING_POSITION,
            .enter = { .setInitialState = GrowTileStart }
        },
        .backgroundColor = backgroundColor,
        .cornerRadius = CLAY_CORNER_RADIUS(metrics->tileCornerRadius)
    }) {
        CLAY_TEXT(text, {
            .fontId = FONT_ID_LARGE,
            .fontSize = fontSize,
            .letterSpacing = (uint16_t)(fontSize / 24),
            .textColor = textColor,
        });
    }
}

// the tile that got merged away, kept visible while it slides into its survivor
static void GhostTileComponent(const UiContext *ui, const Board *board, const MergeGhost *ghost, const BoardMetrics *metrics)
{
    int targetCell = FindCellOfTile(board, ghost->survivorId);
    if (targetCell == -1)
    {
        return;
    }

    const Tile *tile = &ghost->consumedTile;
    GridNumberTileComponent(ui, tile->id, tile->value, (size_t)targetCell, Z_LAYER_GHOST_TILE, metrics);
}

static void LiveTileComponent(const UiContext *ui, const Board *board, size_t cellIndex, const BoardMetrics *metrics)
{
    const Tile *tile = &board->tiles[cellIndex];
    if (is_empty_tile(tile))
    {
        return;
    }

    // from the logic POV, the board has already finished the merge, the tile value is then already doubled
    // so we have to search through the ghost array first to keep old value during the visual merge transition timeframe
    // and then fall back onto the real tile value
    int displayValue = FindDisplayValue(board, tile);
    GridNumberTileComponent(ui, tile->id, displayValue, cellIndex, Z_LAYER_LIVE_TILE, metrics);
}

static void EndOverlayComponent(const OverlayContent *content, const BoardMetrics *metrics)
{
    float gridSize = metrics->gridSize;

    CLAY(Clay_GetElementId(content->id_name), {
        .layout = {
            .sizing = { .width = CLAY_SIZING_FIXED(gridSize), .height = CLAY_SIZING_FIXED(gridSize) },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .childGap = 16
        },
        .floating = {
            .attachTo = CLAY_ATTACH_TO_PARENT,
            .zIndex = Z_LAYER_OVERLAY
        },
        .transition = {
            .handler = Clay_EaseOut,
            .duration = OVERLAY_FADE_DURATION,
            .properties = CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR,
            .enter = { .setInitialState = FadeInFromTransparent }
        },
        .backgroundColor = content->background_color,
        .cornerRadius = CLAY_CORNER_RADIUS(metrics->gridCornerRadius)
    }) {
        CLAY_TEXT(content->title, {
            .fontId = FONT_ID_LARGE,
            .fontSize = (uint16_t)(gridSize * 0.12f),
            .letterSpacing = 2,
            .textColor = content->text_color
        });
        CLAY_TEXT(content->hint, {
            .fontId = FONT_ID_SMALL,
            .fontSize = (uint16_t)(gridSize * 0.045f),
            .letterSpacing = 1,
            .textColor = content->text_color
        });
    }
}

static void ScoreComponent(UiContext *ui, unsigned int score, const BoardMetrics *metrics)
{    
    Clay_String scoreText = ui_format(ui, "Score: %u", score);

    CLAY(CLAY_ID("Score"), {
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .sizing = { .width = CLAY_SIZING_FIXED(metrics->gridSize), .height = CLAY_SIZING_FIXED(metrics->scoreHeight) },
            .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_CENTER }
        }
    }) {
        CLAY_TEXT(scoreText, {
            .fontId = FONT_ID_SMALL,
            .fontSize = (uint16_t)(metrics->scoreHeight * 0.8f),
            .letterSpacing = 1,
            .textColor = COLOR_TEXT_BELOW_8,
        });
    }
}

static void GridComponent(const UiContext *ui, const Board *board, bool isBoardAnimating, const BoardMetrics *metrics)
{
    uint16_t tileGap = metrics->tileGap;

    CLAY(CLAY_ID("Grid"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = { .width = CLAY_SIZING_FIXED(metrics->gridSize), .height = CLAY_SIZING_FIXED(metrics->gridSize) },
            .padding = { tileGap, tileGap, tileGap, tileGap },
            .childGap = tileGap,
        },
        .backgroundColor = COLOR_BOARD_BG,
        .cornerRadius = { metrics->gridCornerRadius }
    }) {
        for (size_t row = 0; row < board->sideSize; row++)
        {
            CLAY_AUTO_ID({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = tileGap } }) {
                for (size_t col = 0; col < board->sideSize; col++)
                {
                    size_t cellIndex = BoardCoordToIndex(board, (BoardCoord) { .row = row, .col = col });
                    GridEmptyTileComponent(cellIndex, metrics);
                }
            }
        }

        for (size_t ghostIndex = 0; ghostIndex < board->ghostCount; ghostIndex++)
        {
            GhostTileComponent(ui, board, &board->ghosts[ghostIndex], metrics);
        }

        for (size_t cellIndex = 0; cellIndex < board->finalSize; cellIndex++)
        {
            LiveTileComponent(ui, board, cellIndex, metrics);
        }

        if (!isBoardAnimating)
        {
            if (board->state == GAME_STATE_WON)
            {
                EndOverlayComponent(&WIN_OVERLAY, metrics);
            }
            else if (board->state == GAME_STATE_LOST)
            {
                EndOverlayComponent(&LOSE_OVERLAY, metrics);
            }
        }
    }
}

void GameScreen(UiContext *ui, const Board *board, bool isBoardAnimating)
{
    BoardMetrics metrics = ComputeBoardMetrics(board->sideSize, ui->screenWidth, ui->screenHeight);

    CLAY(CLAY_ID("AppContainer"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .padding = CLAY_PADDING_ALL(PAGE_PADDING),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .childGap = SCORE_GRID_GAP
        },
        .backgroundColor = COLOR_PAGE_BG
    }) {
        ScoreComponent(ui, board->score, &metrics);
        GridComponent(ui, board, isBoardAnimating, &metrics);
    }
}
