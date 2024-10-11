#include <stdio.h>
#include <stdlib.h>
#include "chessboard.h"
#include "move_stack.h"


void init_move_stack(move_stack *s) {
	s->top = NULL;
	s->size = 0;
	return;
}

void push(move_stack *s, Move move) {
	node *nn = (node *) malloc (sizeof(node));
	nn -> move = move;
	nn->next = s->top;
	s->top = nn;
	s->size++;
	return;
}

Move peek(move_stack s) {
	return s.top->move;
}

Move pop(move_stack *s) {
	Move m = s->top->move;
	node *temp = s->top;
	s->top = temp->next;
	s->size--;
	free(temp);
	return m;
}


