#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "renderers/clay_renderer_raylib.c"

#include "board.h"
#include "board_view.h"
#include "font_data.h"
#include "settings_screen.h"
#include "ui_constants.h"
#include "ui_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { FONT_LARGE_SIZE = 128, FONT_SMALL_SIZE = 64 };

typedef enum {
    SCENE_GAME,
    SCENE_SETTINGS
} Scene;

typedef struct {
    Board board;
    SettingsState settings;
    SettingsState settings_snapshot; // restored when the settings scene is left without applying
    Scene scene;
    UiContext ui;
    double anim_ends_at;
    bool has_queued_move;
    Dir queued_dir;
} App;

void HandleClayErrors(Clay_ErrorData errorData)
{
    printf("Clay error: %.*s\n", (int)errorData.errorText.length, errorData.errorText.chars);
}

static bool read_direction_key(Dir* p_dir)
{
    bool moved = false;
    if      (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) { *p_dir = UP;    moved = true; }
    else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) { *p_dir = LEFT;  moved = true; }
    else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) { *p_dir = DOWN;  moved = true; }
    else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) { *p_dir = RIGHT; moved = true; }

    return moved;
}

static bool overlay_confirmed(const OverlayContent *p_overlay)
{
    bool pressed_enter = IsKeyPressed(KEY_ENTER);
    bool clicked_overlay = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && Clay_PointerOver(Clay_GetElementId(p_overlay->id_name));
    return pressed_enter || clicked_overlay;
}

static void start_new_game(App *p_app)
{
    p_app->board = BoardCreate(p_app->settings.selected_side_size);
    for (uint16_t i = 0; i < p_app->settings.tile_spawn_count; i++)
    {
        BoardSpawnRandomTile(&p_app->board);
    }

    p_app->anim_ends_at = 0.0;
    p_app->has_queued_move = false;
}

static void toggle_settings_scene(App *p_app)
{
    if (p_app->scene == SCENE_GAME)
    {
        p_app->settings_snapshot = p_app->settings;
        p_app->scene = SCENE_SETTINGS;
    }
    else
    {
        if (!p_app->settings.new_game_requested)
        {
            p_app->settings = p_app->settings_snapshot;
        }
        p_app->scene = SCENE_GAME;
    }
}

// returns whether tiles are still sliding from the previous move
static bool update_game(App *p_app)
{
    Board *p_board = &p_app->board;

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R))
    {
        start_new_game(p_app);
    }

    Dir pressed_dir = UP;
    bool was_pressed = read_direction_key(&pressed_dir);
    if (was_pressed)
    {
        p_app->queued_dir = pressed_dir;
        p_app->has_queued_move = true;
    }

    bool is_board_animating = GetTime() < p_app->anim_ends_at;
    if (is_board_animating)
    {
        return true;
    }

    if (p_board->state == GAME_STATE_WON && overlay_confirmed(&WIN_OVERLAY))
    {
        p_board->state = GAME_STATE_PLAYING_AFTER_WIN;
        p_app->has_queued_move = false; // ignore input while overlay is present
    }
    else if (p_board->state == GAME_STATE_LOST && overlay_confirmed(&LOSE_OVERLAY))
    {
        start_new_game(p_app);
    }

    p_board->ghostCount = 0;

    if (p_app->has_queued_move)
    {
        p_app->has_queued_move = false;

        bool turn_changed_board = BoardPlayTurn(p_board, p_app->queued_dir);
        if (turn_changed_board)
        {
            p_app->anim_ends_at = GetTime() + TILE_SLIDE_DURATION;
        }
    }

    return false;
}

static Clay_RenderCommandArray BuildLayout(App *p_app, bool is_board_animating)
{
    Clay_BeginLayout();

    if (p_app->scene == SCENE_GAME)
    {
        GameScreen(&p_app->ui, &p_app->board, is_board_animating);
    }
    else if (p_app->scene == SCENE_SETTINGS)
    {
        SettingsScreen(&p_app->ui, &p_app->settings);
    }

    return Clay_EndLayout(GetFrameTime());
}

static Font load_embedded_font(int font_size, const char *font_name)
{
    Font font = LoadFontFromMemory(".ttf", roboto_med_ttf, (int)roboto_med_ttf_len, font_size, NULL, 0);
    if (font.texture.id == 0)
    {
        printf("Failed to load embedded font as %s, falling back to default\n", font_name);
        return GetFontDefault();
    }

    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    return font;
}

int main(void)
{
    srand((unsigned)time(NULL));
    Clay_Raylib_Initialize(DESIGN_WIDTH, DESIGN_HEIGHT, "Clay 2048", FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetExitKey(KEY_NULL);

    uint32_t clay_memory_size = Clay_MinMemorySize();
    void *clay_memory_block = malloc(clay_memory_size);
    if (clay_memory_block == NULL)
    {
        printf("Failed to allocate %u bytes for Clay\n", clay_memory_size);
        Clay_Raylib_Close();
        return 1;
    }

    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, clay_memory_block);
    Clay_Initialize(
        clay_arena,
        (Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() },
        (Clay_ErrorHandler) { .errorHandlerFunction = HandleClayErrors }
    );

    Font fonts[FONT_COUNT];
    fonts[FONT_ID_LARGE] = load_embedded_font(FONT_LARGE_SIZE, "FONT_ID_LARGE");
    fonts[FONT_ID_SMALL] = load_embedded_font(FONT_SMALL_SIZE, "FONT_ID_SMALL");
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    static App app = {
        .settings = {
            .selected_side_size = SET_BOARD_SIZE_DEF,
            .tile_spawn_count = SET_TILE_SPAWN_DEF,
            .new_game_requested = false,
        },
        .scene = SCENE_GAME,
        .queued_dir = UP,
    };
    app.settings_snapshot = app.settings;
    start_new_game(&app);

    while(!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            toggle_settings_scene(&app);
        }

        bool is_board_animating = false;
        if (app.scene == SCENE_GAME)
        {
            is_board_animating = update_game(&app);
        }

        float screen_width = (float)GetScreenWidth();
        float screen_height = (float)GetScreenHeight();
        Clay_SetLayoutDimensions((Clay_Dimensions) { screen_width, screen_height });

        Vector2 mouse_position = GetMousePosition();
        bool is_mouse_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        Clay_SetPointerState(
            (Clay_Vector2) { mouse_position.x, mouse_position.y },
            is_mouse_down
        );

        // TODO: enable when needed (prob. settings scroll container), otherwise remove
        // Vector2 wheel_movement = GetMouseWheelMoveV();
        // Clay_UpdateScrollContainers(
        //     true,
        //     (Clay_Vector2) { wheel_movement.x, wheel_movement.y },
        //     GetFrameTime()
        // );

        ui_begin_frame(&app.ui, screen_width, screen_height, IsWindowResized());
        Clay_RenderCommandArray render_commands = BuildLayout(&app, is_board_animating);
        SetMouseCursor(app.ui.requestedCursor);

        BeginDrawing();
        Clay_Raylib_Render(render_commands, fonts);
        EndDrawing();

        if (app.settings.new_game_requested)
        {
            app.settings.new_game_requested = false;
            app.settings_snapshot = app.settings;
            start_new_game(&app);
            app.scene = SCENE_GAME;
        }
    }

    UnloadFont(fonts[FONT_ID_SMALL]);
    UnloadFont(fonts[FONT_ID_LARGE]);
    Clay_Raylib_Close();
    free(clay_memory_block);
    temp_render_buffer = NULL;

    return 0;
}
