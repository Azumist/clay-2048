#ifndef LABELS_H
#define LABELS_H

#include "clay.h"

static const Clay_String TILE_LABELS[] = {
    CLAY_STRING_CONST("2"),
    CLAY_STRING_CONST("4"),
    CLAY_STRING_CONST("8"),
    CLAY_STRING_CONST("16"),
    CLAY_STRING_CONST("32"),
    CLAY_STRING_CONST("64"),
    CLAY_STRING_CONST("128"),
    CLAY_STRING_CONST("256"),
    CLAY_STRING_CONST("512"),
    CLAY_STRING_CONST("1024"),
    CLAY_STRING_CONST("2048"),
    CLAY_STRING_CONST("4096"),
    CLAY_STRING_CONST("8192"),
    CLAY_STRING_CONST("16384"),
    CLAY_STRING_CONST("32768"),
    CLAY_STRING_CONST("65536"),
    CLAY_STRING_CONST("131072"),
    CLAY_STRING_CONST("262144"),
    CLAY_STRING_CONST("524288"),
    CLAY_STRING_CONST("1048576"),
    CLAY_STRING_CONST("2097152") // technicly it doesn't allow endless play but who will ever reach those values anyway
};

#define TILE_LABEL_COUNT (sizeof(TILE_LABELS) / sizeof(TILE_LABELS[0]))

#endif