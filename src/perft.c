#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <assert.h>

#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_array.h"
#include "move_stack.h"

#define RED_TEXT "\033[0;31m"
#define GREEN_TEXT "\033[0;32m"
#define RESET "\033[0m"

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
#define TURN WHITE
// #define MAX_DEPTH 6

typedef struct {
	char* fen;
	int depth;
	unsigned long long nodes;
	unsigned long long expected_nodes;
} PerftTest;

#define TEST_1 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define TEST_2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
#define TEST_3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
#define TEST_4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define TEST_5 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 "
#define TEST_6 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 "

const PerftTest perft_test_suite[] = {
    (PerftTest){TEST_1, 6, 0, 119060324},  // {20, 400, 8902, 197281, 4865609, 119060324}
    (PerftTest){TEST_2, 5, 0, 193690690},  // {48, 2039, 97862, 4085603, 193690690}
    (PerftTest){TEST_3, 7, 0, 178633661},   // {14, 191, 2812, 43238, 674624, 11030083}
    (PerftTest){TEST_4, 5, 0, 15833292},   // {6, 264, 9467, 422333, 15833292}
    (PerftTest){TEST_5, 5, 0, 89941194},   // {44, 1486, 62379, 2103487, 89941194}
    (PerftTest){TEST_6, 5, 0, 164075551}   // {46, 2079, 89890, 3894594, 164075551}
};

unsigned long long perfit(int depth, short turn, board* b, const int max_depth) {
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
			unsigned long long child_nodes = perfit(depth - 1, turn == WHITE ? BLACK : WHITE, b, max_depth);
			nodes += child_nodes;

			// Undo the move to restore board state
			unmake_move(b);

			// Print the move and node count at max depth
			if (depth == max_depth) {
				wprintf(L"\"%c%d%c%d\": %llu,\n", m.src.file + 'a' - 1, m.src.rank, m.dest.file + 'a' - 1, m.dest.rank, child_nodes);
			}

		} else {
			// Debugging output for invalid moves
			wprintf(L"%c%d -> %c%d: INVALID MOVE\n", m.src.file + 'a' - 1, m.src.rank, m.dest.file + 'a' - 1, m.dest.rank);

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

void single_perft_test(const char* fen, int depth, int turn) {
	board b;
	load_fen(&b, (char*)fen);
	clear_move_list(b.white_attacks);
	clear_move_list(b.black_attacks);

	if (turn == WHITE) {
		update_attacks_for_color(&b, BLACK);
		update_attacks_for_color(&b, WHITE);

		filter_legal_moves(&b, WHITE);
	} else {
		update_attacks_for_color(&b, WHITE);
		update_attacks_for_color(&b, BLACK);

		filter_legal_moves(&b, BLACK);
	}

	wprintf(L"White board: ");
	print_squares_from_bb(b.white_board);

	wprintf(L"\nBlack board: ");
	print_squares_from_bb(b.black_board);

	wprintf(L"\nblack king attacks: ");
	print_squares_from_bb(generate_king_attacks(BLACK_KING, b.black->king, &b));

	wprintf(L"\n========================================\n");

	clock_t start = clock();
	unsigned long long nodes = perfit(depth, turn, &b, depth);
	clock_t end = clock();

	wprintf(L"Nodes: %llu\n", nodes);
	wprintf(L"time_ms = %.2lf\n", ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC);

	return;
}

void perfit_test() {
	int num_tests = sizeof(perft_test_suite) / sizeof(PerftTest);
	for (int i = 0; i < num_tests; i++) {
		board b;
		init_board(&b);
		load_fen(&b, perft_test_suite[i].fen);
		clear_move_list(b.white_attacks);
		clear_move_list(b.black_attacks);

		update_attacks_for_color(&b, BLACK);
		update_attacks_for_color(&b, WHITE);

		wprintf(L"Running test %d: %s\n", i + 1, perft_test_suite[i].fen);
		clock_t start = clock();
		unsigned long long nodes = perfit(perft_test_suite[i].depth, TURN, &b, perft_test_suite[i].depth);
		clock_t end = clock();
		double time = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
		wprintf(L"Nodes: %llu, time_ms = %.2lf\n", nodes, time);

		if (nodes == perft_test_suite[i].expected_nodes) {
			wprintf(L"Test %d: " GREEN_TEXT "PASSED\n" RESET, i + 1);
		} else {
			wprintf(L"Test %d: " RED_TEXT "FAILED\n" RESET, i + 1);
			wprintf(L"Expected: %llu, Got: %llu\n", perft_test_suite[i].expected_nodes, nodes);
		}
	}
}

int main() {
	setlocale(LC_ALL, "");
	perfit_test();
	return 0;
}
