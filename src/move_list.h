#ifndef MOVE_LIST_H
#define MOVE_LIST_H

#include "move_types.h"

#define MAX_GAME_MOVES 256

typedef struct moveList {
    Move moves[MAX_GAME_MOVES];
    int top;
} MoveList;

void init_movelist(MoveList *list);
void append_move(MoveList *list, Move move);
void remove_move(MoveList *list, Move move);
void clear_movelist(MoveList *list);
void print_movelist(MoveList *list);
Move peek_movelist(MoveList *list);
void append_list(MoveList *list1, MoveList *list2);

#endif