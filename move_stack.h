#include <stdint.h>
#include <stdbool.h>
#include "move_types.h"

typedef struct node{
	Move move;
	struct node *next;
} node;


typedef struct move_stack {
	node *top;
	int size;
} move_stack;

bool validate_square(square s);
void init_move_stack(move_stack *s);
void push(move_stack *s, Move move);
Move peek(move_stack s);
Move pop(move_stack *s);