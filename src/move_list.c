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

#define EXTRACT_FILE(S) ((S & 0b111000) >> 3)
#define EXTRACT_RANK(S) (S & 0b000111)

void print_movelist(MoveList *list) {
    if(!list || list->top == -1) return;

    wprintf(L"\n");

    for (int i = 0; i <= list->top; i++) {
        int file, rank;
        file = EXTRACT_FILE(FROMSQ(list->moves[i].move));
        rank = EXTRACT_RANK(FROMSQ(list->moves[i].move));

        Square src = (Square){.file = file, .rank = rank};

        file = EXTRACT_FILE(TOSQ(list->moves[i].move));
        rank = EXTRACT_RANK(TOSQ(list->moves[i].move));
        Square dest = (Square){.file = file, .rank = rank};
        
        wprintf(L"%c%d%c%d,", src.file+'a', src.rank+1, dest.file+'a', dest.rank+1);
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