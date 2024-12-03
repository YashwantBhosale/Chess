
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
	list->moves = (Move**)calloc(list->capacity, sizeof(Move*));

	if (!list->moves) {
		printf("Error: Could not allocate memory for moves array\n");
		exit(1);
	}
	list->move_count = 0;
	return;
}

void add_move(MoveList* list, Move move) {
    if (list->move_count >= list->capacity) {
        wprintf(L"Error: Move list is full. Cannot add move %d -> %d\n", move.src, move.dest);
        return;
    }

    Move* new_move = (Move*)malloc(sizeof(Move));
    if (!new_move) {
        printf("Error: Could not allocate memory for new move\n");
        return;
    }

    *new_move = move;
    list->moves[list->move_count] = new_move;
    list->move_count++;

    // wprintf(L"Added legal move %d -> %d to list. Current count: %d\n", move.src, move.dest, list->move_count);
}

void clear_move_list(MoveList* list) {
    for (int i = 0; i < list->move_count; i++) {
        if (list->moves[i]) {
            free(list->moves[i]);
            list->moves[i] = NULL;
        }
    }
    list->move_count = 0;
}

void remove_move_at_index(MoveList* list, int index) {
    if (index < 0 || index >= list->move_count) {
        wprintf(L"Invalid index %d\n", index);
        return;
    }

    // wprintf(L"Removing move at index %d\n", index);

    if (list->moves[index]) {
        free(list->moves[index]);
        list->moves[index] = NULL;  // Set to NULL to prevent double-free
    }

    // Shift remaining elements left to fill the gap
    for (int i = index; i < list->move_count - 1; i++) {
        list->moves[i] = list->moves[i + 1];
    }

    list->move_count--;
    list->moves[list->move_count] = NULL;  // Set last element to NULL after shifting
}


void remove_move(MoveList* list, Move move) {
	for (int i = 0; i < list->move_count; i++) {
		if (list->moves[i]->src.file == move.src.file &&
		    list->moves[i]->src.rank == move.src.rank &&
		    list->moves[i]->dest.file == move.dest.file &&
		    list->moves[i]->dest.rank == move.dest.rank) {
			remove_move_at_index(list, i);
			return;
		}
	}
}

void free_movelist(MoveList* list) {
	if (!list || !list->moves) return;

	for (int i = 0; i < list->move_count; i++) {
		if (list->moves[i] && list->moves[i]->piece != 0) {
			free(list->moves[i]);
			list->moves[i] = NULL;
		} else {
			wprintf(L"invalid address\n");
		}
	}

	free(list->moves);
	list->moves = NULL;
	list->move_count = 0;
	list->capacity = 0;
	return;
}

void print_movelist(MoveList* list) {
	wprintf(L"Printing move list: total moves %d\n", list->move_count);
	for (int i = 0; i < list->move_count; i++) {
		wprintf(L"(%u, %c%d -> %c%d, %d), ", list->moves[i]->piece, list->moves[i]->src.file + 'A' - 1, list->moves[i]->src.rank, list->moves[i]->dest.file + 'A' - 1, list->moves[i]->dest.rank, list->moves[i]->score);
	}
	wprintf(L"\n");
	return;
}