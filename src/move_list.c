#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "move_list.h"

void init_movelist(MoveList *list) {
	memset(list->moves, 0, sizeof(Move) * MAX_GAME_MOVES);
	list->top = -1;
}

void append_move(MoveList *list, Move move) {
	if (list->top >= MAX_GAME_MOVES) {
		return;
	}

	list->moves[++list->top] = move;
}

static void remove_move_at_index(MoveList *list, int index) {
	if (index < 0 || index > list->top) {
		return;
	}

    if(index == list->top) {
        list->top--;
        return;
    }

    for (int i = index; i <= list->top; i++) {
        list->moves[i] = list->moves[i+1];
    }

    list->top--;
    return;
}

void remove_move(MoveList *list, Move move) {
	for (int i = 0; i <= list->top; i++) {
		if (move.move == list->moves[i].move) {
            remove_move_at_index(list, i);
            return;
		}
	}
}

static inline void file_and_rank_from_index(int index, int *file, int *rank) {
    *file = (index % 8) + 1;
    *rank = (index / 8) + 1;
}

void print_movelist(MoveList *list) {
    wprintf(L"\n");

    for (int i = 0; i <= list->top; i++) {
        int file, rank;
        file_and_rank_from_index(FROMSQ(list->moves[i].move), &file, &rank);
        Square src = (Square){.file = file, .rank = rank};

        file_and_rank_from_index(TOSQ(list->moves[i].move), &file, &rank);
        Square dest = (Square){.file = file, .rank = rank};
        
        wprintf(L"%c%d%c%d,", src.file+'a'-1, src.rank, dest.file+'a'-1, dest.rank);
    }   
    wprintf(L"\n");
}

void clear_movelist(MoveList *list) {
    list->top = -1;
}

Move peek_movelist(MoveList *list) {
    if(list->top == -1) {
        return (Move){0};
    }

    return list->moves[list->top];
}
void append_list(MoveList *list1, const MoveList *list2) {
    if (!list1 || !list2) return;

    if (list1->top + list2->top > MAX_GAME_MOVES - 1) {
        // OVERFLOW!
        return;
    }

    memcpy(&list1->moves[list1->top], list2->moves, list2->top * sizeof(Move));
    list1->top += list2->top;
}