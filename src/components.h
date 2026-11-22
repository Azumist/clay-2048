#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdint.h>
#include "clay.h"
#include "ui_context.h"

// sizes in design pixels, scaled by the UiContext when drawn
typedef struct {
    float minWidth;
    float minHeight;
    float fontSize;
    Clay_Color textColor;
    Clay_Color textColorHovered;
    Clay_Color backgroundColor;
    Clay_Color backgroundColorHovered;
} ButtonStyle;

// fontSize in design pixels, scaled by the UiContext when drawn
typedef struct {
    float fontSize;
    Clay_Color textColor;
} LabelStyle;

typedef enum {
    STEPPER_NONE, STEPPER_DECREASE, STEPPER_INCREASE
} StepperAction;

void LabelComponent(UiContext *ui, Clay_ElementId id, Clay_String text, const LabelStyle *style);
bool ButtonComponent(UiContext *ui, Clay_ElementId id, Clay_String label, const ButtonStyle *style);

// label above a "- value +" row; the caller decides whether the returned action is allowed
StepperAction StepperComponent(UiContext *ui, Clay_ElementId id, Clay_String label, Clay_String valueText);

#endif
