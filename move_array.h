#ifndef MOVE_LIST_H
#define MOVE_LIST_H

#include <stdbool.h>
#include <stdint.h>
#include "move_types.h"

#define MAX_MOVES 256  // Maximum number of moves possible in a chess position

typedef struct MoveList {
    Move** moves;      // Array of pointers to Move objects
    int capacity;      // Total capacity of the array
    int move_count;    // Current number of moves
} MoveList;

void init_movelist(MoveList* list);
void add_move(MoveList* list, Move move);
void remove_move(MoveList* list, Move move);
void remove_move_at_index(MoveList* list, int index);
void free_movelist(MoveList* list);
void print_movelist(MoveList* list);
void clear_move_list(MoveList* list);
#endif