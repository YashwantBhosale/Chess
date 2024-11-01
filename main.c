#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "chessboard.h"
#include "moves.h"
#include "move_stack.h"

move_stack moves;

void clrscr() {
    wprintf(L"\033[H\033[J");
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

void new_game(board *b) {
	short turn, status;
	init_move_stack(&moves);
	init_board(b);
	turn = WHITE;

	while (1) {
		// update_attack_tables(b);
		// clrscr();
		if (b->attack_tables[WHITE] == 0ULL) {
			wprintf(L"%s is in checkmate 1\n", "White");
			break;
		}

		else if (b->attack_tables[BLACK] == 0ULL) {
			wprintf(L"%s is in checkmate\n", "Black");
			break;
		}

		print_board(b, turn);
		wprintf(L"%s's turn\n", turn == WHITE ? "White" : "Black");

		// printf("Enter the source square: ");
		square src = read_square();
		// printf("Enter the destination square: ");
		square dest = read_square();
		status = make_move(src, dest, turn, b);

		switch (status) {
			case NORMAL_MOVE:
				break;
			case CAPTURE_MOVE:
				wprintf(L"Captured piece\n");
				break;
			case CHECK_MOVE:
				wprintf(L"Check\n");
				break;
			case CHECKMATE_MOVE:
				wprintf(L"Checkmate\n");
				break;
			case STALEMATE_MOVE:
				wprintf(L"Stalemate\n");
				break;
			case INVALID_MOVE:
				wprintf(L"Invalid move in main menu\n");
				break;
			default:
				break;
		}
		/* print castling rights */
		wprintf(L"castling rights: %u\n", b->castle_rights);

		update_attack_tables(b, turn);
		if (b->attack_tables[WHITE] == 0ULL) {
			// clrscr();
			print_board(b, turn);
			wprintf(L"%s is in checkmate 2\n", "White");
			break;
		}

		else if (b->attack_tables[BLACK] == 0ULL) {
			// clrscr();
			print_board(b, turn);
			wprintf(L"%s is in checkmate\n", "Black");
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
}

int main() {
	setlocale(LC_CTYPE, "");
	board *b = (board *)malloc(sizeof(board));
	new_game(b);
	free(b);
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
