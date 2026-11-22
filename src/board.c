#include "board.h"
#include "tile.h"
#include <assert.h>
#include <stdio.h>

enum { WINNING_VALUE = 2048 }; // TODO: move to settings to allow for more rules customization

// one row or column, walked starting from the edge the tiles slide toward
typedef struct {
    size_t startIndex;
    ptrdiff_t step;
} BoardLine;

static void BoardMove(Board *board, Dir dir);
static BoardLine BoardLineFor(const Board *board, Dir dir, size_t lineNumber);
static size_t BoardLineCell(BoardLine line, size_t position);
static void BoardReadLine(const Board *board, BoardLine line, Tile *lineTiles);
static void BoardWriteLine(Board *board, BoardLine line, const Tile *lineTiles);
static void BoardSquashAndMerge(Board *board, Tile *lineTiles);
static bool BoardContainsValue(const Board *board, int value);
static bool BoardTilesEqual(const Board *first, const Board *second);
static const Tile *BoardTileAt(const Board *board, BoardCoord coord);
static bool CanMergeWith(const Tile *tile, const Tile *neighbour);
static bool BoardHasAnyMove(const Board *board);
static bool BoardCoordOutOfBounds(const Board *board, const BoardCoord *coord);
static void TilesLeftSquashInPlace(Tile *tileArr, size_t rowSize);
static void TilesLeftMergeInPlace(Tile *tileArr, size_t rowSize, unsigned int *score, MergeGhost *ghostArr, size_t *ghostCount);

#ifdef DEBUG_MODE
static void DebugPrintBoardCoord(const BoardCoord *boardCoord);
static void DebugPrintTileArr(const Tile *tileArr, size_t size);
static void DebugPrintBoard(const Board *board);
#endif

Board BoardCreate(size_t initSideSize)
{
    assert(initSideSize <= MAX_SIDE_SIZE);

    Board board = (Board) {
        .sideSize = initSideSize,
        .finalSize = initSideSize * initSideSize,
        .ghostCount = 0,
        .lastSpawnedId = -1,
        .score = 0,
        .state = GAME_STATE_PLAYING,
    };

    for (size_t i = 0; i < board.finalSize; ++i)
    {
        board.tiles[i] = tile_create_empty();
    }

    return board;
}

bool BoardPlayTurn(Board *board, Dir dir)
{
    bool isPlayable = board->state == GAME_STATE_PLAYING || board->state == GAME_STATE_PLAYING_AFTER_WIN;
    if (!isPlayable)
    {
        return false;
    }

    Board before = *board;
    BoardMove(board, dir);

    bool boardChanged = !BoardTilesEqual(&before, board);
    if (!boardChanged)
    {
        return false; // move into a wall
    }

    BoardSpawnRandomTile(board);

    bool canStillWin = board->state == GAME_STATE_PLAYING;
    if (canStillWin && BoardContainsValue(board, WINNING_VALUE))
    {
        board->state = GAME_STATE_WON;
    }
    else if (!BoardHasAnyMove(board))
    {
        board->state = GAME_STATE_LOST;
    }

#ifdef DEBUG_MODE
    DebugPrintBoard(board);
#endif

    return true;
}

static void BoardMove(Board *board, Dir dir)
{
    board->ghostCount = 0;

    for (size_t lineNumber = 0; lineNumber < board->sideSize; lineNumber++)
    {
        BoardLine line = BoardLineFor(board, dir, lineNumber);
        Tile lineTiles[MAX_SIDE_SIZE];

        BoardReadLine(board, line, lineTiles);
        BoardSquashAndMerge(board, lineTiles);
        BoardWriteLine(board, line, lineTiles);
    }
}

static BoardLine BoardLineFor(const Board *board, Dir dir, size_t lineNumber)
{
    size_t side = board->sideSize;
    size_t lastPosition = side - 1;

    switch (dir)
    {
        case LEFT:  return (BoardLine) { .startIndex = lineNumber * side,                .step = 1 };
        case RIGHT: return (BoardLine) { .startIndex = lineNumber * side + lastPosition, .step = -1 };
        case UP:    return (BoardLine) { .startIndex = lineNumber,                       .step = (ptrdiff_t)side };
        case DOWN:  return (BoardLine) { .startIndex = lastPosition * side + lineNumber, .step = -(ptrdiff_t)side };
    }

    assert(false && "unknown direction");
    return (BoardLine) { .startIndex = lineNumber * side, .step = 1 };
}

static size_t BoardLineCell(BoardLine line, size_t position)
{
    ptrdiff_t offset = (ptrdiff_t)position * line.step;
    return (size_t)((ptrdiff_t)line.startIndex + offset);
}

static void BoardReadLine(const Board *board, BoardLine line, Tile *lineTiles)
{
    for (size_t position = 0; position < board->sideSize; position++)
    {
        lineTiles[position] = board->tiles[BoardLineCell(line, position)];
    }
}

static void BoardWriteLine(Board *board, BoardLine line, const Tile *lineTiles)
{
    for (size_t position = 0; position < board->sideSize; position++)
    {
        board->tiles[BoardLineCell(line, position)] = lineTiles[position];
    }
}

static void BoardSquashAndMerge(Board *board, Tile *lineTiles)
{
    TilesLeftSquashInPlace(lineTiles, board->sideSize);
    TilesLeftMergeInPlace(lineTiles, board->sideSize, &board->score, board->ghosts, &board->ghostCount);
    TilesLeftSquashInPlace(lineTiles, board->sideSize);
}

static bool BoardContainsValue(const Board *board, int value)
{
    for (size_t cellIndex = 0; cellIndex < board->finalSize; cellIndex++)
    {
        if (board->tiles[cellIndex].value == value)
        {
            return true;
        }
    }

    return false;
}

static bool BoardTilesEqual(const Board *first, const Board *second)
{
    for (size_t cellIndex = 0; cellIndex < first->finalSize; cellIndex++)
    {
        const Tile *tileA = &first->tiles[cellIndex];
        const Tile *tileB = &second->tiles[cellIndex];
        if (!tile_eq(tileA, tileB))
        {
            return false;
        }
    }

    return true;
}

static const Tile *BoardTileAt(const Board *board, BoardCoord coord)
{
    if (BoardCoordOutOfBounds(board, &coord))
    {
        return NULL;
    }

    return &board->tiles[BoardCoordToIndex(board, coord)];
}

static bool CanMergeWith(const Tile *tile, const Tile *neighbour)
{
    if (neighbour == NULL)
    {
        return false;
    }

    return tile_eq_val(tile, neighbour);
}

static bool BoardHasAnyMove(const Board *board)
{
    for (size_t row = 0; row < board->sideSize; row++)
    {
        for (size_t col = 0; col < board->sideSize; col++)
        {
            const Tile *tile = BoardTileAt(board, (BoardCoord) { .row = row, .col = col });
            if (is_empty_tile(tile))
            {
                return true;
            }

            const Tile *rightTile = BoardTileAt(board, (BoardCoord) { .row = row, .col = col + 1 });
            const Tile *belowTile = BoardTileAt(board, (BoardCoord) { .row = row + 1, .col = col });

            bool canMergeRight = CanMergeWith(tile, rightTile);
            bool canMergeDown = CanMergeWith(tile, belowTile);

            if (canMergeRight || canMergeDown)
            {
                return true;
            }
        }
    }

    return false;
}

bool BoardSpawnRandomTile(Board *board)
{
    size_t emptyCells[MAX_SIDE_SIZE * MAX_SIDE_SIZE];
    size_t emptyCount = 0;

    for (size_t cellIndex = 0; cellIndex < board->finalSize; cellIndex++)
    {
        if (is_empty_tile(&board->tiles[cellIndex]))
        {
            emptyCells[emptyCount] = cellIndex;
            emptyCount++;
        }
    }

    if (emptyCount == 0)
    {
        return false;
    }

    size_t chosenCell = emptyCells[(size_t)rand() % emptyCount];
    int spawnValue = (rand() % 10 == 0) ? 4 : 2; // TODO: change back after testning

    Tile spawnedTile = tile_create_new(spawnValue);
    board->tiles[chosenCell] = spawnedTile;

#ifdef DEBUG_MODE
    BoardCoord spawnCoord = BoardCoordFromIndex(board, chosenCell);
    printf("spawned %d at ", spawnValue);
    DebugPrintBoardCoord(&spawnCoord);
#endif

    board->lastSpawnedId = spawnedTile.id;
    return true;
}


BoardCoord BoardCoordFromIndex(const Board *board, size_t index)
{
    return (BoardCoord) {
        .row = index / board->sideSize,
        .col = index % board->sideSize
    };
}

size_t BoardCoordToIndex(const Board *board, BoardCoord coord)
{
    return (size_t)(coord.row * board->sideSize + coord.col);
}

static bool BoardCoordOutOfBounds(const Board *board, const BoardCoord *coord)
{
    return coord->col < 0
        || coord->row < 0
        || coord->col >= board->sideSize
        || coord->row >= board->sideSize;
}

static void TilesLeftSquashInPlace(Tile *tileArr, size_t rowSize)
{
    size_t keptCount = 0;
    for (size_t i = 0; i < rowSize; i++)
    {
        if (is_empty_tile(&tileArr[i]))
        {
            continue;
        }
        tileArr[keptCount] = tileArr[i];
        keptCount++;
    }

    for (size_t i = keptCount; i < rowSize; i++)
    {
        tileArr[i] = tile_create_empty();
    }
}

static void TilesLeftMergeInPlace(Tile *tileArr, size_t rowSize, unsigned int *score, MergeGhost *ghostArr, size_t *ghostCount)
{
    // rowSize - 1 below would wrap around for an empty row
    assert(rowSize > 0);

    for (size_t i = 0; i < rowSize - 1; i++)
    {
        Tile survivor = tileArr[i];
        Tile consumed = tileArr[i + 1];

        bool bothExist = !is_empty_tile(&survivor) && !is_empty_tile(&consumed);
        bool shouldMerge = bothExist && survivor.value == consumed.value;

        if (!shouldMerge)
        {
            continue;
        }

        int mergedValue = survivor.value * 2;
        tileArr[i].value = mergedValue;
        tileArr[i + 1] = tile_create_empty();
        *score += mergedValue;

        assert(*ghostCount < MAX_MERGES_PER_MOVE);
        MergeGhost ghost = { .consumedTile = consumed, .survivorId = survivor.id };
        ghostArr[*ghostCount] = ghost;
        (*ghostCount)++;

        i++;
    }
}

#ifdef DEBUG_MODE
static void DebugPrintBoardCoord(const BoardCoord *boardCoord)
{
    printf("[row: %zu, col: %zu]\n", boardCoord->row, boardCoord->col);
}

// one row per line, so the output reads like the board on screen
static void DebugPrintBoard(const Board *board)
{
    if (board == NULL)
    {
        printf("Board not found.\n");
        return;
    }

    printf("score: %u, state: %d\n", board->score, (int)board->state);
    for (size_t row = 0; row < board->sideSize; row++)
    {
        size_t rowStart = BoardCoordToIndex(board, (BoardCoord) { .row = row, .col = 0 });
        DebugPrintTileArr(&board->tiles[rowStart], board->sideSize);
    }
}

static void DebugPrintTileArr(const Tile *tileArr, size_t size)
{
    printf("{");
    for (size_t i = 0; i < size; i++)
    {
        printf("[t_id: %d, t_v: %d]", tileArr[i].id, tileArr[i].value);
        if (i < size - 1)
        {
            printf(", ");
        }
    }
    printf("}\n");
}
#endif

int FindCellOfTile(const Board *board, int tileId)
{
    for (size_t cellIndex = 0; cellIndex < board->finalSize; cellIndex++)
    {
        if (board->tiles[cellIndex].id == tileId)
        {
            return (int)cellIndex;
        }
    }

    return -1;
}

int FindDisplayValue(const Board *board, const Tile *tile)
{
    for (size_t ghostIndex = 0; ghostIndex < board->ghostCount; ghostIndex++)
    {
        const MergeGhost *ghost = &board->ghosts[ghostIndex];
        if (ghost->survivorId == tile->id)
        {
            return ghost->consumedTile.value;
        }
    }

    return tile->value;
}
