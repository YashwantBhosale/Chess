#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits.h>

#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_stack.h"
#include "evaluation.h"

#define FILEMASK_A 0b0000000100000001000000010000000100000001000000010000000100000001ULL

void print_binay_of_uint64(uint64_t num) {
	for (int i = 63; i >= 0; i--) {
		wprintf(L"%d", (num & (1ULL << i)) ? 1 : 0);
		if ((i % 8) == 0) wprintf(L" ");
	}
	wprintf(L"\n");
	return;
}


short num_doubled_blocked_pawns(board* board, short turn) {
	short num = 0, pawn_count = turn == WHITE ? board->white->count.pawns : board->black->count.pawns;
	/* we consider doubled as sub-type of blocked */
	/* hence we directly check if pawn can be blocked by any type of piece */
	uint64_t player_board, opp_board;

	player_board = turn == WHITE ? board->white_board : board->black_board;
	opp_board = turn == WHITE ? board->black_board : board->white_board;

	for (int i = 0; i < pawn_count; i++) {
		switch (turn) {
			case WHITE: {
				num += (board->white->pawns[i] << 8) & (player_board | opp_board) ? 1 : 0;
				break;
			}
			case BLACK: {
				num += (board->black->pawns[i] >> 8) & (player_board | opp_board) ? 1 : 0;
				break;
			}
		}
	}
	return num;
}

short num_isolated_pawns(board* board, short turn) {
	short num = 0, pawn_count = turn == WHITE ? board->white->count.pawns : board->black->count.pawns;
	int rank, file;
	uint64_t combined_board = 0ULL, filemask_combined;
	for (int i = 0; i < pawn_count; i++) {
		combined_board |= (turn == WHITE ? board->white->pawns[i] : board->black->pawns[i]);
	}
	for (int i = 0; i < pawn_count; i++) {
		filemask_combined = 0ULL;
		get_rank_and_file_from_bitboard((turn == WHITE ? board->white->pawns[i] : board->black->pawns[i]), &file, &rank);
		if (file > 1) filemask_combined |= FILEMASK_A << ((file - 1) - 1);
		if (file < 8) filemask_combined |= FILEMASK_A << ((file + 1) - 1);
		num += (combined_board & filemask_combined) ? 0 : 1;
	}
	return num;
}

int get_weight_from_piece_type(uint8_t piece_type) {
	switch (piece_type) {
		case PAWN:
			return 1;
		case KNIGHT:
			return 3;
		case BISHOP:
			return 3;
		case ROOK:
			return 5;
		case QUEEN:
			return 9;
		default:
			return 0;
	}
}

bool static_in_check(short turn, board* board) {
	uint64_t king_position = turn == WHITE ? board->white->king : board->black->king;
	uint64_t opponent_attacks = turn == WHITE ? board->black_lookup_table[0] : board->white_lookup_table[0];
	return (king_position & opponent_attacks) != 0;
}

/* Calculates the evaluation of the Board */
double get_evaluation_of_board(board* board) {
	double eval = 0;

	// pieces captured by white
	for (int i = 0; i < board->captured_pieces_count[WHITE]; i++) {
		eval += get_weight_from_piece_type(piece_type(board->captured_pieces[WHITE][i]));
	}

	// pieces captured by black
	for (int i = 0; i < board->captured_pieces_count[BLACK]; i++) {
		eval -= get_weight_from_piece_type(piece_type(board->captured_pieces[BLACK][i]));
	}

	eval -= static_in_check(WHITE, board);
	eval += static_in_check(BLACK, board);

	eval -= 0.5 * (num_doubled_blocked_pawns(board, WHITE) - num_doubled_blocked_pawns(board, BLACK));
	eval -= 0.5 * (num_isolated_pawns(board, WHITE) - num_isolated_pawns(board, BLACK));
	return eval;
}

void display_evaluation(double eval) {
	double scale = 16.0 / 39.0;
	double eval_scaled = (eval * scale);
	wprintf(L"\n\n\t       [ ");
	for (int i = 0; i < (int)(eval_scaled + 16); i++) wprintf(L"\u2588");
	for (int i = (int)(eval_scaled + 16); i < 32; i++) wprintf(L"-");
	wprintf(L" ]\t%lf\n\n", eval_scaled / 4.0);
	return;
}