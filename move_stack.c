#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>
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
	if(s.top == NULL) {
		Move m = {0};
		return m;
	}

	return (s.top)->move;
}

Move pop(move_stack *s) {
	Move m = s->top->move;
	node *temp = s->top;
	s->top = temp->next;
	s->size--;
	free(temp);
	return m;
}

bool validate_square(square s) {
	if(s.file < A || s.file > H || s.rank < 1 || s.rank > 8)
		return false;
	else
		return true;
}