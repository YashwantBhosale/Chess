#include <stdint.h>
#include <stdbool.h>
typedef struct {
	square src;
	square dest;
	uint8_t piece;
	uint8_t captured_piece;
	uint8_t flags;
} Move;

typedef struct node{
	Move move;
	struct node *next;
} node;


typedef struct {
	node *top;
	int size;
} move_stack;

extern move_stack moves;

bool validate_square(square s);
void init_move_stack(move_stack *s);
void push(move_stack *s, Move move);
Move peek(move_stack s);
Move pop(move_stack *s);