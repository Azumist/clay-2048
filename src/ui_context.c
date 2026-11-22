#include "ui_context.h"
#include "ui_constants.h"
#include "raylib.h"
#include <stdarg.h>
#include <stdio.h>

static const float MIN_UI_SCALE = 0.6f;

void ui_begin_frame(UiContext *ctx, float screenWidth, float screenHeight, bool windowResized)
{
    ctx->screenWidth = screenWidth;
    ctx->screenHeight = screenHeight;

    float scale_x = screenWidth / (float)DESIGN_WIDTH;
    float scale_y = screenHeight / (float)DESIGN_HEIGHT;
    ctx->scale = scale_x < scale_y ? scale_x : scale_y;

    if (ctx->scale < MIN_UI_SCALE)
    {
        ctx->scale = MIN_UI_SCALE;
    }

    ctx->requestedCursor = MOUSE_CURSOR_DEFAULT;
    // tiles would otherwise slide to their new positions while the window is being resized
    ctx->skipAnimations = windowResized;

    // last frame's text has been rendered by now, so its memory can be reused
    ctx->frameText.used = 0;
}

float ui_px(float design_pixels, const UiContext *ctx)
{
    return design_pixels * ctx->scale;
}

uint16_t ui_px_u16(float design_pixels, const UiContext *ctx)
{
    float scaled = design_pixels * ctx->scale + 0.5f;
    return scaled < 1.0f ? 1 : (uint16_t)scaled;
}

int ui_font_for_size(uint16_t font_size)
{
    return font_size > 40 ? FONT_ID_LARGE : FONT_ID_SMALL;
}

Clay_String ui_format(UiContext *ctx, const char *format, ...)
{
    FrameTextArena *arena = &ctx->frameText;
    size_t remaining = UI_FRAME_TEXT_CAPACITY - arena->used;
    char *dest = arena->data + arena->used;

    bool is_arena_full = remaining == 0;
    if (is_arena_full)
    {
        return CLAY_STRING("");
    }

    va_list args;
    va_start(args, format);
    int written = vsnprintf(dest, remaining, format, args);
    va_end(args);

    if (written < 0)
    {
        return CLAY_STRING("");
    }

    // vsnprintf returns the length it wanted to write
    size_t length = (size_t)written;
    bool was_truncated = length >= remaining;
    if (was_truncated)
    {
        printf("ui_format: frame text arena full, truncating \"%s\"\n", format);
        length = remaining - 1;
    }

    arena->used += length + 1; // +1 to keep the terminator

    return (Clay_String) { .length = (int32_t)length, .chars = dest };
}