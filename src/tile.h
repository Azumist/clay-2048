#ifndef TILE_H
#define TILE_H

#include <stdbool.h>
#include <stddef.h>

extern int _next_tile_id;

typedef struct {
    int id;
    int value;
} Tile;

Tile tile_create_new(int p_value);
Tile tile_create_empty();
bool tile_eq_val(const Tile *a, const Tile *b);
bool tile_eq(const Tile *a, const Tile *b);
bool is_empty_tile(const Tile *p_tile);
size_t tile_value_rank(int tile_value, size_t max_rank);

#endif