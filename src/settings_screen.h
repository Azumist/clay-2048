#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ui_context.h"

enum {
    SET_BOARD_SIZE_MIN = 3,
    SET_BOARD_SIZE_DEF = 4,
    SET_BOARD_SIZE_MAX = 16
};

enum {
    SET_TILE_SPAWN_MIN = 2,
    SET_TILE_SPAWN_DEF = 2,
    SET_TILE_SPAWN_MAX = 8
};

typedef struct {
    size_t selected_side_size;
    uint16_t tile_spawn_count;
    bool new_game_requested;
} SettingsState;

void SettingsScreen(UiContext *ui, SettingsState *settings);

#endif
