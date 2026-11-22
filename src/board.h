#ifndef BOARD_H
#define BOARD_H

#include "tile.h"
#include <stddef.h>
#include <stdlib.h>

#define MAX_SIDE_SIZE 16
#define MAX_MERGES_PER_MOVE (MAX_SIDE_SIZE * MAX_SIDE_SIZE / 2)

typedef enum {
    RIGHT, UP, LEFT, DOWN
} Dir;

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_WON, // show win screen, lock input
    GAME_STATE_PLAYING_AFTER_WIN, // player continued from win screen, don't check for win condtition again
    GAME_STATE_LOST
} GameState;

typedef struct {
    size_t row;
    size_t col;
} BoardCoord;

typedef struct {
    Tile consumedTile;
    int survivorId;
} MergeGhost;

typedef struct {
    Tile tiles[MAX_SIDE_SIZE * MAX_SIDE_SIZE];
    MergeGhost ghosts[MAX_MERGES_PER_MOVE];
    size_t sideSize;
    size_t finalSize;
    size_t ghostCount;
    int lastSpawnedId;
    unsigned int score;
    GameState state;
} Board;

Board BoardCreate(size_t initSideSize);
bool BoardPlayTurn(Board *board, Dir dir);
bool BoardSpawnRandomTile(Board *board);
size_t BoardCoordToIndex(const Board *board, BoardCoord coord);
BoardCoord BoardCoordFromIndex(const Board *board, size_t index);
int FindCellOfTile(const Board *board, int tileId);
int FindDisplayValue(const Board *board, const Tile *tile);

#endif
