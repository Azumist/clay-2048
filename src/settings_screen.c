#include "settings_screen.h"
#include "colors.h"
#include "components.h"
#include <stdio.h>

static const ButtonStyle APPLY_BUTTON_STYLE = {
    .minWidth = 64.0f,
    .minHeight = 48.0f,
    .fontSize = 32.0f,
    .textColor = COLOR_BUTTON_TEXT,
    .textColorHovered = COLOR_BUTTON_TEXT_HOVER,
    .backgroundColor = COLOR_BUTTON_BG,
    .backgroundColorHovered = COLOR_BUTTON_BG_HOVER
};
static const LabelStyle SETTINGS_TITLE_STYLE = { .fontSize = 48.0f, .textColor = COLOR_TEXT_BELOW_8 };
static const LabelStyle SETTINGS_HINT_STYLE = { .fontSize = 24.0f, .textColor = COLOR_TEXT_BELOW_8 };

static Clay_String clay_string_from_buffer(const char *buffer, int snprintf_result)
{
    Clay_String result = {
        .length = snprintf_result > 0 ? snprintf_result : 0,
        .chars = buffer
    };
    return result;
}

static void TileSpawnCountStepper(UiContext *ui, SettingsState *settings)
{
    unsigned int spawnCount = (unsigned)settings->tile_spawn_count;
    StepperAction action = StepperComponent(
        ui,
        CLAY_ID("TileCountStepper"),
        CLAY_STRING("Initial tile spawn count"),
        ui_format(ui, "%u", spawnCount)
    );

    bool can_decrease = settings->tile_spawn_count > SET_TILE_SPAWN_MIN;
    bool fits_on_board = settings->tile_spawn_count < settings->selected_side_size;
    bool can_increase = settings->tile_spawn_count < SET_TILE_SPAWN_MAX && fits_on_board;

    if (action == STEPPER_DECREASE && can_decrease)
    {
        settings->tile_spawn_count--;
    }
    else if (action == STEPPER_INCREASE && can_increase)
    {
        settings->tile_spawn_count++;
    }
}

static void BoardSizeStepper(UiContext *ui, SettingsState *settings)
{
    size_t side = settings->selected_side_size;
    StepperAction action = StepperComponent(
        ui,
        CLAY_ID("BoardSizeStepper"),
        CLAY_STRING("Board size"),
        ui_format(ui, "%zu x %zu", side, side)
    );

    bool can_decrease = settings->selected_side_size > SET_BOARD_SIZE_MIN;
    bool can_increase = settings->selected_side_size < SET_BOARD_SIZE_MAX;

    if (action == STEPPER_DECREASE && can_decrease)
    {
        settings->selected_side_size--;
        if (settings->tile_spawn_count > settings->selected_side_size)
        {
            settings->tile_spawn_count = (uint16_t)settings->selected_side_size;
        }
    }
    else if (action == STEPPER_INCREASE && can_increase)
    {
        settings->selected_side_size++;
    }
}

void SettingsScreen(UiContext *ui, SettingsState *settings)
{
    CLAY(CLAY_ID("SettingsContainer"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .childGap = ui_px_u16(24, ui)
        },
        .backgroundColor = COLOR_PAGE_BG
    }) {
        LabelComponent(ui, CLAY_ID("SettingsTitle"), CLAY_STRING("Settings"), &SETTINGS_TITLE_STYLE);
        LabelComponent(ui, CLAY_ID("SettingsHint"), CLAY_STRING("To restart the game from the board directly, press Ctrl + R"), &SETTINGS_HINT_STYLE);

        TileSpawnCountStepper(ui, settings);
        BoardSizeStepper(ui, settings);

        if (ButtonComponent(ui, CLAY_ID("ApplySettings"), CLAY_STRING("Apply and restart"), &APPLY_BUTTON_STYLE))
        {
            settings->new_game_requested = true;
        }
    }
}
