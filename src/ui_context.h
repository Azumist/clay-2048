#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#include <stdbool.h>
#include <stdint.h>
#include "clay.h"

enum { UI_FRAME_TEXT_CAPACITY = 4096 };

typedef struct {
    char data[UI_FRAME_TEXT_CAPACITY];
    size_t used;
} FrameTextArena;

// per-frame UI state shared by all components
typedef struct {
    float screenWidth;
    float screenHeight;
    float scale;
    int requestedCursor;
    bool skipAnimations;
    FrameTextArena frameText;
} UiContext;

void ui_begin_frame(UiContext *ctx, float screenWidth, float screenHeight, bool windowResized);
float ui_px(float design_pixels, const UiContext *ctx);
uint16_t ui_px_u16(float design_pixels, const UiContext *ctx);
int ui_font_for_size(uint16_t font_size);
Clay_String ui_format(UiContext *ctx, const char *format, ...)
    __attribute__((format(gnu_printf, 2, 3)));

#endif
