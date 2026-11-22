#include "tile.h"

int _next_tile_id = 1;

Tile tile_create_new(int p_value)
{
    return (Tile) {
        .id = _next_tile_id++,
        .value = p_value
    };
}

Tile tile_create_empty()
{
    return (Tile) {
        .id = 0,
        .value = 0
    };
}

bool tile_eq_val(const Tile *a, const Tile *b)
{
    return a->value == b->value;
}

bool tile_eq(const Tile *a, const Tile *b)
{
    return a->id == b->id && a->value == b->value;
}

bool is_empty_tile(const Tile *p_tile)
{
    return p_tile->id == 0 && p_tile->value == 0;
}

// 2 -> 0, 4 -> 1, 8 -> 2, ... clamped to max_rank; used to index per-value lookup tables
size_t tile_value_rank(int tile_value, size_t max_rank)
{
    size_t rank = 0;
    int remaining_value = tile_value;
    while (remaining_value > 2 && rank < max_rank)
    {
        remaining_value /= 2;
        rank++;
    }
    return rank;
}
