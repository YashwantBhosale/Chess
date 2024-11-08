#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#include "chessboard.h"
#include "move_stack.h"
#include "moves.h"

#define TIME_LOWER_BOUND 0.01
#define TIME_UPPER_BOUND 0.15

move_stack moves;

/*
TODO: Names for player/bot
*/

void clrscr() {
	wprintf(L"\033[H\033[J");
	return;
}

void clear_screen() {
	system("clear");
	return;
}

square read_square() {
	char file;
	int rank;
	square s;
	scanf(" %c%d", &file, &rank);
	s.file = file - 'a' + 1;
	s.rank = rank;
	return s;
}

int randomNumber(int l, int h) {
	// Seed the random number generator with the current time
	srand(time(0));
	// Generate a random number in the range [l, h]
	return l + rand() % (h - l + 1);
}

void single_player(board *b) {
	short turn, status = NORMAL_MOVE, legal_moves_array[MAX_LEGAL_MOVES][4];
	init_move_stack(&moves);
	init_board(b);
	turn = WHITE;
	square src, dest;

	while(1) {
		clrscr();

		switch (status) {
			case NORMAL_MOVE:
				break;
			case CAPTURE_MOVE:
				wprintf(L"\033[1;31mCAPTURED PIECE\033[0m\n");
				break;
			case CHECK_MOVE:
				wprintf(L"\033[1;31mCHECK\033[0m\n");
				break;
			case CHECKMATE_MOVE:
				wprintf(L"\033[1;31mCHECKMATE\033[0m\n");
				break;
			case STALEMATE_MOVE:
				wprintf(L"\033[1;31mSTALEMATE\033[0m\n");
				break;
			case INVALID_MOVE:
				wprintf(L"\033[1;31mINVALID MOVE\033[0m\n");
				break;
			default:
				break;
		}

		if (b->attack_tables[WHITE] == 0ULL) {
			wprintf(L"White is in CHECKMATE\n");
			break;
		} else if (b->attack_tables[BLACK] == 0ULL) {
			wprintf(L"Black is in CHECKMATE\n");
			break;
		}
		print_board(b, turn);
		turn == WHITE ? wprintf(L"White's turn\n") : wprintf(L"Black's turn\n");

		if (turn == WHITE) {
			wprintf(L"Enter src and dest square: ");
			src = read_square();
			dest = read_square();
		}
		else if (turn == BLACK) {
			int upper_bound = get_all_legal_moves(BLACK, b, legal_moves_array) - 1;
			int rand = randomNumber(0, upper_bound);
			src.file = legal_moves_array[rand][0];
			src.rank = legal_moves_array[rand][1];
			dest.file = legal_moves_array[rand][2];
			dest.rank = legal_moves_array[rand][3];
			wprintf(L"Computer's move: %c%d to %c%d\n", src.file + 'a' - 1, src.rank, dest.file + 'a' - 1, dest.rank);
		}

		status = make_move(src, dest, turn, b);

		update_attack_tables(b, turn);
		if (status == INVALID_MOVE) {
			continue;
		}

		if (status == CHECKMATE_MOVE || status == STALEMATE_MOVE) {
			break;
		}

		turn = !turn;
	}
	return;
}

void two_player(board *b) {
	short turn, status = NORMAL_MOVE;
	init_move_stack(&moves);
	init_board(b);
	turn = WHITE;
	square src, dest;

	while (1) {
		clrscr();

		switch (status) {
			case NORMAL_MOVE:
				break;
			case CAPTURE_MOVE:
				wprintf(L"\033[1;31mCAPTURED PIECE\033[0m\n");
				break;
			/* TODO: add check_move in make_move */
			case CHECK_MOVE:
				wprintf(L"\033[1;31mCHECK\033[0m\n");
				break;
			case CHECKMATE_MOVE:
				wprintf(L"\033[1;31mCHECKMATE\033[0m\n");
				break;
			case STALEMATE_MOVE:
				wprintf(L"\033[1;31mSTALEMATE\033[0m\n");
				break;
			case INVALID_MOVE:
				wprintf(L"\033[1;31mINVALID MOVE\033[0m\n");
				break;
			default:
				break;
		}

		if (b->attack_tables[WHITE] == 0ULL) {
			wprintf(L"White is in CHECKMATE\n");
			break;
		}

		else if (b->attack_tables[BLACK] == 0ULL) {
			wprintf(L"Black is in CHECKMATE\n");
			break;
		}

		print_board(b, turn);
		turn == WHITE ? wprintf(L"White's turn\n") : wprintf(L"Black's turn\n");
		wprintf(L"Enter src and dest square: ");
		src = read_square();
		dest = read_square();
		status = make_move(src, dest, turn, b);


		update_attack_tables(b, turn);
		if (b->attack_tables[WHITE] == 0ULL) {
			clrscr();
			print_board(b, turn);
			wprintf(L"White is in CHECKMATE\n");
			break;
		}

		else if (b->attack_tables[BLACK] == 0ULL) {
			clrscr();
			print_board(b, turn);
			wprintf(L"Black is in CHECKMATE\n");
			break;
		}

		if (status == INVALID_MOVE) {
			continue;
		}

		if (status == CHECKMATE_MOVE || status == STALEMATE_MOVE) {
			break;
		}

		turn = !turn;
	}
	return;
}

int main() {
	setlocale(LC_CTYPE, "");
	board *b = (board *)malloc(sizeof(board));
	two_player(b);
	// single_player(b);
	free_board(b);
	return 0;
}

/*
int main() {
    setlocale(LC_CTYPE, "");
    board b, *simulation_board;
    init_board(&b);
    print_board(&b, WHITE);

    square src, dest;
    src.file = E;
    src.rank = 2;

    dest.file = E;
    dest.rank = 4;

    short status = make_move(src, dest, WHITE, &b);
    print_board(&b, BLACK);
    return 0;
}
*/
