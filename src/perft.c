#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>

#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_array.h"
#include "move_stack.h"

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
#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
// #define STARTING_FEN "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
#define TURN WHITE
#define MAX_DEPTH 6

unsigned long long perfit(int depth, short turn, board* b) {
	if (depth == 0) {
		return 1ULL;
	}

	MoveList* pseudo_legal_moves = turn == WHITE ? b->white_attacks : b->black_attacks;
	MoveList* legal_moves = turn == WHITE ? b->white_legal_moves : b->black_legal_moves;

	clear_move_list(pseudo_legal_moves);
	clear_move_list(legal_moves);

	
	b->black_attacks->move_count = 0;
	update_attacks_for_color(b, !turn);

	b->white_attacks->move_count = 0;
	update_attacks_for_color(b, turn);
	filter_legal_moves(b, turn);  // Filter legal moves into legal_moves

	unsigned long long nodes = 0ULL;
	int move_count = legal_moves->move_count;
	Move legal_moves_bk[move_count];
	memcpy(legal_moves_bk, legal_moves->moves, sizeof(Move) * move_count);

	uint64_t lookup_table_bk[97], *lookup_table_ptr;

	if (turn == WHITE) {
		lookup_table_ptr = b->white_lookup_table;
		memcpy(lookup_table_bk, b->white_lookup_table, sizeof(uint64_t) * 97);
	} else {
		lookup_table_ptr = b->black_lookup_table;
		memcpy(lookup_table_bk, b->black_lookup_table, sizeof(uint64_t) * 97);
	}

	uint64_t white_board_bk = b->white_board;
	uint64_t black_board_bk = b->black_board;

	for (int i = 0; i < move_count; i++) {
		memcpy(legal_moves->moves, legal_moves_bk, sizeof(Move) * move_count);
		memcpy(lookup_table_ptr, lookup_table_bk, sizeof(uint64_t) * 97);
		b->white_board = white_board_bk;
		b->black_board = black_board_bk;

		if (turn == WHITE) {
			b->white_legal_moves->move_count = move_count;
		} else {
			b->black_legal_moves->move_count = move_count;
		}

		if (!legal_moves->moves[i].piece) {
			continue;
		}
		Move m = legal_moves->moves[i];

		// Make the move
		int status = make_move(
		    (square){.file = m.src.file, .rank = m.src.rank},
		    (square){.file = m.dest.file, .rank = m.dest.rank},
		    turn,
		    b,
		    true,
		    m.type);

		if (status != INVALID_MOVE) {
			// Calculate child nodes for this specific move
			unsigned long long child_nodes = perfit(depth - 1, turn == WHITE ? BLACK : WHITE, b);
			nodes += child_nodes;

			// Undo the move to restore board state
			unmake_move(b);

			// Print the move and node count at max depth
			if (depth == MAX_DEPTH) {
				wprintf(L"%c%d%c%d: %llu \n", m.src.file + 'a' - 1, m.src.rank, m.dest.file + 'a' - 1, m.dest.rank, child_nodes);
			}
		} else {
			// Debugging output for invalid moves
			wprintf(L"%c%d -> %c%d: INVALID MOVE\n",
			        m.src.file + 'a' - 1, m.src.rank,
			        m.dest.file + 'a' - 1, m.dest.rank);

			wprintf(L"%d%d->%d%d\n", b->moves->top->move.src.file, b->moves->top->move.src.rank, b->moves->top->move.dest.file, b->moves->top->move.dest.rank);

			// Additional debug information
			print_board(b, turn);
			print_moves(b->black_board);
			wprintf(L"\nwhite_board = ");
			print_moves(b->white_board);
			wprintf(L"\n");
			wprintf(L"black_board = ");
			print_moves(b->black_board);
			wprintf(L"nodes = %llu, turn = %d\n", nodes, turn);
			exit(1);  // Stop execution on invalid move error
		}
	}
	return nodes;
}

void perfit_test(board* b) {
	clock_t start = clock();
	unsigned long long nodes = perfit(MAX_DEPTH, TURN, b);
	clock_t end = clock();

	double time = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
	wprintf(L"Nodes: %llu, time_ms = %.2lf\n", nodes, time);
	wprintf(L"\nwhite attacks: ");
	print_moves(b->white_lookup_table[0]);
	wprintf(L"\n");
}

int main() {
	setlocale(LC_ALL, "");
	board b;
	load_fen(&b, STARTING_FEN);
	wprintf(L"Starting position\n");
	// update_attacks(&b);
	// filter_legal_moves(&b, TURN);

	print_board(&b, TURN);

	update_attacks(&b);
	filter_legal_moves(&b, TURN);
	// print_squares_from_bb(b.white_board);
	// print_squares_from_bb(b.black_board);
	// make_move((square){.file = D, .rank = 7}, (square){.file = C, .rank = 8}, WHITE, &b, true, WHITE_PROMOTES_TO_QUEEN);

	// wprintf(L"=============================================================\n");
	// // wprintf(L"castle rights: %d %llu\n", b.castle_rights, b.castle_rights & BLACK_QUEEN_SIDE_CASTLE_RIGHTS);
	// // make_move((square){.file = A, .rank = 8}, (square){.file = B, .rank = 8}, TURN, &b, false, 0);
	// // wprintf(L"castle rights: %d %llu\n", b.castle_rights, b.castle_rights & BLACK_QUEEN_SIDE_CASTLE_RIGHTS);

	// update_attacks(&b);
	// filter_legal_moves(&b, BLACK);
	// // wprintf(L"queens = %d\n", b.white->count.queens);
	// // wprintf(L"new queen = %llu\n", b.square_table[C - 1][8 - 1]);
	// // print_squares_from_bb(b.white_lookup_table[lookup_index(b.square_table[C - 1][8 - 1])]);

	// print_squares_from_bb(b.white_board);
	// print_squares_from_bb(b.black_board);
	// make_move((square){.file = F, .rank = 2}, (square){.file = H, .rank = 1}, BLACK, &b, true, 0);
	// update_attacks(&b);
	// filter_legal_moves(&b, TURN);

	// print_squares_from_bb(generate_king_attacks(b.square_table[E - 1][1 - 1], b.white->king, &b));
	// print_squares_from_bb(b.white_board);
	// print_squares_from_bb(b.black_board);

	// wprintf(L"%d\n", b.square_table[F - 1][1 - 1]);
	perfit_test(&b);
	return 0;
}
