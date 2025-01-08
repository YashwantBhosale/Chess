#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>
#include <locale.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_stack.h"
#include "move_array.h"
#include "engine.h"
#include "evaluation.h"
#include "transposition.h"
#include "opening_book.h"

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
// #define STARTING_FEN "7r/8/4K1k1/1Q6/8/3N4/3q4/8 w - - 0 1"
ZobristTable transposition_table;
OpeningBook opening_book;

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

	int system = 1;  // 0 -> linux, 1 -> windows
	#if defined(_WIN32) || defined(_WIN64)
		system = 1;
	#else
		system = 0;
	#endif

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

		print_board(b, turn, system);
		filter_legal_moves(b, turn);

		if (turn == WHITE && b->white_legal_moves->move_count == 0) {
			wprintf(L"Black wins\n");
			return;
		}
		if (turn == BLACK && b->black_legal_moves->move_count == 0) {
			wprintf(L"White wins\n");
			return;
		}

		wprintf(L"%s's Turn: ", turn == WHITE ? "White" : "Black");
		square src = read_square();
		square dest = read_square();

		int status = make_move(src, dest, turn, b, false, 0);
		if (status == INVALID_MOVE)
			continue;

		update_attacks(b);

		turn = !turn;
	}
	return;
}

void single_player(board *b, int level) {
	if (!b) return;

	int system = 1;  // 0 -> linux, 1 -> windows
	#if defined(_WIN32) || defined(_WIN64)
		system = 1;
	#else
		system = 0;
	#endif

	load_fen(b, STARTING_FEN);
	init_zobrist(&transposition_table);
	init_opening_book(&opening_book);

	short turn = WHITE;
	update_attacks(b);
	double evaluation = 0;

	double time_taken = 0;
	load_opening_book(&opening_book, "book.csv");

	while (1) {
		// clrscr();
		display_evaluation(evaluation);

		if (turn == BLACK && b->moves->top) {
			wprintf(L"Last move was: %c%d->%c%d\n", b->moves->top->move.src.file + 'a' - 1, b->moves->top->move.src.rank, b->moves->top->move.dest.file + 'a' - 1, b->moves->top->move.dest.rank);
			wprintf(L"Time taken for search: %.2lfms\n", time_taken);
		}

		print_board(b, BLACK, system);
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

			int status = make_move(src, dest, turn, b, false, 0);
			if (status == INVALID_MOVE) {
				wprintf(L"invalid move\n");
				continue;
			}
		} else {
			wprintf(L"Thinking...\n");
			uint64_t lookup_table_backup[97];
			memcpy(lookup_table_backup, turn == WHITE ? b->white_lookup_table : b->black_lookup_table, sizeof(lookup_table_backup));

			clock_t start = clock();
			evaluated_move eval = minimax(b, 3 + level, turn, INT_MIN, INT_MAX);
			clock_t end = clock();

			time_taken = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
			evaluation = eval.evaluation;

			memcpy(turn == WHITE ? b->white_lookup_table : b->black_lookup_table, lookup_table_backup, sizeof(lookup_table_backup));
			wprintf(L"Best move: ");
			wprintf(L"%c%d -> %c%d\n", eval.best_move.src.file + 'a' - 1, eval.best_move.src.rank, eval.best_move.dest.file + 'a' - 1, eval.best_move.dest.rank);

			int status = make_move(eval.best_move.src, eval.best_move.dest, turn, b, true, eval.best_move.type);
			if (status == INVALID_MOVE) {
				wprintf(L"invalid move\n");
				continue;
			}
		}

		update_attacks(b);
		turn = !turn;
	}
	return;
}

void show_menu() {
	/*

	     ██████╗██╗  ██╗███████╗███████╗███████╗
	    ██╔════╝██║  ██║██╔════╝██╔════╝██╔════╝
	    ██║     ███████║█████╗  ███████╗███████╗
	    ██║     ██╔══██║██╔══╝  ╚════██║╚════██║
	    ╚██████╗██║  ██║███████╗███████║███████║
	     ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝
	*/
	clrscr();
	wprintf(L"\n");
	wprintf(L" ██████╗██╗  ██╗███████╗███████╗███████╗\n");
	wprintf(L"██╔════╝██║  ██║██╔════╝██╔════╝██╔════╝\n");
	wprintf(L"██║     ███████║█████╗  ███████╗███████╗\n");
	wprintf(L"██║     ██╔══██║██╔══╝  ╚════██║╚════██║\n");
	wprintf(L"╚██████╗██║  ██║███████╗███████║███████║\n");
	wprintf(L" ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝\n");
	wprintf(L"\n");
	wprintf(L"\n");

	wprintf(L"Welcome to the chess program CLI\n");
	wprintf(L"Please choose mode\n");
	wprintf(L"press T for two player mode\n");
	wprintf(L"press S for single player mode\n");
	wprintf(L"press H for help\n");
	wprintf(L"press ctrl+c to exit\n");
}

void show_help() {
	wprintf(L"How to play:\n");
	wprintf(L"Enter the source square and destination square in the format <file><rank> <file><rank>\n");
	wprintf(L"Example: a2a4\n");
	wprintf(L"Press enter after entering the move\n");

	wprintf(L"press any key to continue\n");
}

void set_input_mode() {
    struct termios tattr;
    tcgetattr(STDIN_FILENO, &tattr);         
    tattr.c_lflag &= ~(ICANON | ECHO);       
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

void reset_input_mode() {
	struct termios tattr;
	tcgetattr(STDIN_FILENO, &tattr);
	tattr.c_lflag |= (ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

int main() {
	setlocale(LC_ALL, "");
	board b;
	char mode;
	set_input_mode();
	while (1) {
		show_menu();
		mode = getchar();
		if (mode == 'T' || mode == 't') {
			reset_input_mode();
			two_player(&b);
		} else if (mode == 'S' || mode == 's') {
			int level = 1;
			reset_input_mode();
			wprintf(L"Enter level of difficulty (1-3): ");
			scanf("%d", &level);
			single_player(&b, level);
		} else if (mode == 'H' || mode == 'h') {
			show_help();
			getchar();
		} else {
			wprintf(L"Invalid mode\n");
		}
	}

	// testing opening book
	/*
	board b;
	load_fen(&b, STARTING_FEN);
	init_opening_book(&opening_book);
	init_zobrist(&transposition_table);

	load_opening_book(&opening_book, "book.csv");

	//test opening book

	Move m = get_book_move(&opening_book, &b, WHITE);

	wprintf(L"Book move: %c%d -> %c%d\n", m.src.file + 'a' - 1, m.src.rank, m.dest.file + 'a' - 1, m.dest.rank);
	*/

	return 0;
}
