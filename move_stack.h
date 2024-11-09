#include <stdint.h>
#include <stdbool.h>
typedef struct Move {
	square src;
	square dest;
	uint8_t piece;

	uint8_t captured_piece;
	uint8_t promoted_piece;

	uint8_t castle_rights;
	uint64_t attack_tables[2]; // We may remove this as we can just call update attack tables function instead but ok for now
	uint8_t flags;
} Move;

typedef struct node{
	Move move;
	struct node *next;
} node;


typedef struct move_stack {
	node *top;
	int size;
} move_stack;

// extern move_stack moves;

bool validate_square(square s);
void init_move_stack(move_stack *s);
void push(move_stack *s, Move move);
Move peek(move_stack s);
Move pop(move_stack *s);