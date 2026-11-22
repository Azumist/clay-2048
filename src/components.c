#include "components.h"
#include "colors.h"
#include "raylib.h"

static const ButtonStyle STEPPER_BUTTON_STYLE = {
    .minWidth = 48.0f,
    .minHeight = 48.0f,
    .fontSize = 32.0f,
    .textColor = COLOR_BUTTON_TEXT,
    .textColorHovered = COLOR_BUTTON_TEXT_HOVER,
    .backgroundColor = COLOR_BUTTON_BG,
    .backgroundColorHovered = COLOR_BUTTON_BG_HOVER
};
static const LabelStyle STEPPER_TITLE_STYLE = { .fontSize = 36.0f, .textColor = COLOR_TEXT_BELOW_8 };
static const LabelStyle STEPPER_VALUE_STYLE = { .fontSize = 36.0f, .textColor = COLOR_BUTTON_TEXT };

void LabelComponent(UiContext *ui, Clay_ElementId id, Clay_String text, const LabelStyle *style)
{
    uint16_t fontSize = ui_px_u16(style->fontSize, ui);

    // text elements can't carry an id in Clay, so the wrapper is what makes the label addressable
    CLAY(id, {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
        }
    }) {
        CLAY_TEXT(text, {
            .fontId = ui_font_for_size(fontSize),
            .fontSize = fontSize,
            .textColor = style->textColor
        });
    }
}

bool ButtonComponent(UiContext *ui, Clay_ElementId id, Clay_String label, const ButtonStyle *style)
{
    bool was_clicked = false;
    uint16_t fontSize = ui_px_u16(style->fontSize, ui);

    CLAY(id, {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIT(ui_px(style->minWidth, ui)),
                .height = CLAY_SIZING_FIT(ui_px(style->minHeight, ui))
            },
            .padding = { 16, 16, 0, 0 },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
        },
        .backgroundColor = Clay_Hovered() ? style->backgroundColorHovered : style->backgroundColor,
        .cornerRadius = CLAY_CORNER_RADIUS(6)
    }) {
        bool is_hovered = Clay_Hovered();
        was_clicked = is_hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        if (is_hovered)
        {
            ui->requestedCursor = MOUSE_CURSOR_POINTING_HAND;
        }

        CLAY_TEXT(label, {
            .fontId = ui_font_for_size(fontSize),
            .fontSize = fontSize,
            .textColor = is_hovered ? style->textColorHovered : style->textColor
        });
    }

    return was_clicked;
}

StepperAction StepperComponent(UiContext *ui, Clay_ElementId id, Clay_String label, Clay_String valueText)
{
    StepperAction action = STEPPER_NONE;

    CLAY(id, {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
            .childGap = ui_px_u16(24, ui)
        }
    }) {
        LabelComponent(ui, CLAY_ID_LOCAL("Title"), label, &STEPPER_TITLE_STYLE);

        CLAY(CLAY_ID_LOCAL("Row"), {
            .layout = {
                .childGap = ui_px_u16(16, ui),
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
            }
        }) {
            // local ids are scoped to this row, so several steppers can share the button names
            if (ButtonComponent(ui, CLAY_ID_LOCAL("Decrease"), CLAY_STRING("-"), &STEPPER_BUTTON_STYLE))
            {
                action = STEPPER_DECREASE;
            }

            LabelComponent(ui, CLAY_ID_LOCAL("Value"), valueText, &STEPPER_VALUE_STYLE);

            if (ButtonComponent(ui, CLAY_ID_LOCAL("Increase"), CLAY_STRING("+"), &STEPPER_BUTTON_STYLE))
            {
                action = STEPPER_INCREASE;
            }
        }
    }

    return action;
}
