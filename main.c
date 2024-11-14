#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>

#include <locale.h>
#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_stack.h"
#include "move_array.h"
#include "engine.h"
#include "evaluation.h"

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_FEN "rnbqk2r/pppp1ppp/5n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1"
// #define STARTING_FEN "r1bqkb1r/ppp1ppPp/2n2n2/3p4/5p2/8/PPPPP2P/RNBQKBNR w KQkq - 0 6"

square read_square() {
	char file;
	int rank;
	square s;
	scanf(" %c%d", &file, &rank);
	s.file = file - 'a' + 1;
	s.rank = rank;
	if (!validate_square(s)) {
		return read_square();
	}
	return s;
}

void print_squares_from_bb(uint64_t bb) {
	for (int i = 0; i < 64; i++) {
		if (bb & (1ULL << i)) {
			int file = i % 8;
			int rank = i / 8;
			wprintf(L"%c%d ", file + 'a', rank + 1);
		}
	}
	wprintf(L"\n");
}

void clrscr() {
	system("clear");
}

void two_player(board *b) {
	if (!b) return;

	load_fen(b, STARTING_FEN);
	short turn = WHITE;
	update_type_board(b, turn);
	wprintf(L"White Board: \n");
	print_squares_from_bb(b->white_board);
	wprintf(L"Black Board: \n");
	print_squares_from_bb(b->black_board);

	wprintf(L"rooks = %d\n", b->white->count.rooks);
	update_attacks(b);

	while (1) {
		clrscr();

		print_board(b, turn);
		filter_legal_moves(b, turn);

		if (turn == WHITE && b->white_legal_moves->move_count == 0) {
			wprintf(L"Black wins\n");
			return;
		}
		if (turn == BLACK && b->black_legal_moves->move_count == 0) {
			wprintf(L"White wins\n");
			return;
		}

		/*
			wprintf(L"White Attacks: \n");
			print_movelist(b->white_attacks);
			wprintf(L"\n");
			wprintf(L"Black Attacks: \n");
			print_movelist(b->black_attacks);
		*/

		wprintf(L"%s's Turn: ", turn == WHITE ? "White" : "Black");
		square src = read_square();
		square dest = read_square();

		int status = make_move(src, dest, turn, b, false);
		if (status == INVALID_MOVE)
			continue;

		update_attacks(b);

		turn = !turn;
	}
	return;
}

void single_player(board *b) {
	if (!b) return;

	load_fen(b, STARTING_FEN);
	short turn = WHITE;
	update_attacks(b);

	while (1) {
		clrscr();
    	print_board(b, BLACK);
		filter_legal_moves(b, turn);
		if (turn == WHITE && b->white_legal_moves->move_count == 0) {
			wprintf(L"Black wins\n");
			return;
		}
		if (turn == BLACK && b->black_legal_moves->move_count == 0) {
			wprintf(L"White wins\n");
			return;
		}

		wprintf(L"%s's Turn: \n", turn == WHITE ? "Engine" : "Black");

		if (turn == BLACK) {
			square src = read_square();
			square dest = read_square();

			int status = make_move(src, dest, turn, b, false);
			if (status == INVALID_MOVE) {
				wprintf(L"invalid move\n");
				continue;
			}
		} else {
			wprintf(L"Thinking...\n");
            uint64_t lookup_table_backup[97];
            memcpy(lookup_table_backup, turn == WHITE ? b->white_lookup_table : b->black_lookup_table, sizeof(lookup_table_backup));
			evaluated_move eval = minimax(b, 3, turn, INT_MIN, INT_MAX);

            memcpy(turn == WHITE ? b->white_lookup_table : b->black_lookup_table, lookup_table_backup, sizeof(lookup_table_backup));
			wprintf(L"Best move: ");
			wprintf(L"%c%d -> %c%d\n", eval.best_move.src.file + 'a' - 1, eval.best_move.src.rank, eval.best_move.dest.file + 'a' - 1, eval.best_move.dest.rank);


			make_move(eval.best_move.src, eval.best_move.dest, turn, b, true);
		}

		update_attacks(b);
		turn = !turn;
	}
}

int main() {
	setlocale(LC_ALL, "");
	board b;
	// two_player(&b);
	single_player(&b);
	return 0;
}
