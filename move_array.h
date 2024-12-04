#ifndef MOVE_LIST_H
#define MOVE_LIST_H

#include <stdbool.h>
#include <stdint.h>
#include "move_types.h"

/* for reference:
typedef struct {
	square src;
	square dest;
	uint8_t piece;

	uint8_t captured_piece;
	uint8_t promoted_piece;
	uint64_t en_passant_square;

	uint8_t castle_rights;
	uint8_t type;

	bool is_check;
	unsigned int score;
} Move;
*/

#define MAX_MOVES 256  // Maximum number of moves possible in a chess position
#define NULL_MOVE (Move){0}

typedef struct MoveList {
	Move moves[MAX_MOVES];
    int capacity;      // Total capacity of the array
    int move_count;    // Current number of moves
} MoveList;

void init_movelist(MoveList* list);
void add_move(MoveList* list, Move move);
void remove_move(MoveList* list, Move move);
void remove_move_at_index(MoveList* list, int index);
void print_movelist(MoveList* list);
void clear_move_list(MoveList* list);
#endif