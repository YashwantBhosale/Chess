
// move_list.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include "move_array.h"
#include "chessboard.h"

void init_movelist(MoveList* list) {
	list->capacity = MAX_MOVES;
	list->move_count = 0;
	memset(list->moves, 0, sizeof(Move) * MAX_MOVES);
	return;
}

void add_move(MoveList* list, Move move) {
    if (list->move_count >= list->capacity) {
        wprintf(L"Error: Move list is full. Cannot add move %d -> %d\n", move.src, move.dest);
        return;
    }
    list->moves[list->move_count++] = move;
}

void clear_move_list(MoveList* list) {
    list->move_count = 0;
}

void remove_move_at_index(MoveList* list, int index) {
    if (index < 0 || index >= list->move_count) {
        wprintf(L"Invalid index %d\n", index);
        return;
    }
	for (int i = index; i < list->move_count - 1; i++) {
		list->moves[i] = list->moves[i + 1];
	}
	list->move_count--;
}


void remove_move(MoveList* list, Move move) {
	for (int i = 0; i < list->move_count; i++) {
		if (list->moves[i].src.file == move.src.file &&
		    list->moves[i].src.rank == move.src.rank &&
		    list->moves[i].dest.file == move.dest.file &&
		    list->moves[i].dest.rank == move.dest.rank) {
			remove_move_at_index(list, i);
			return;
		}
	}
}


void print_movelist(MoveList* list) {
	wprintf(L"Printing move list: total moves %d\n", list->move_count);
	for (int i = 0; i < list->move_count; i++) {
		wprintf(L"(%c%d%c%d, %d), ", list->moves[i].src.file + 'A' - 1, list->moves[i].src.rank, list->moves[i].dest.file + 'A' - 1, list->moves[i].dest.rank, list->moves[i].type);
	}
	wprintf(L"\n");
	return;
}