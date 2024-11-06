#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits.h>
#include "chessboard.h"
#include "moves.h"
#include "move_stack.h"
#include "evaluation.h"

#define FILEMASK_A 0b0000000100000001000000010000000100000001000000010000000100000001ULL

void print_binay_of_uint64(uint64_t num) {
	for(int i = 63; i >= 0; i--) {
		wprintf(L"%d", (num & (1ULL << i)) ? 1 : 0);
		if((i % 8) == 0) wprintf(L" ");
	}
	wprintf(L"\n");
	return;
}

short num_attacked_pawns(board* board, short turn) {
	short num = 0, pawn_count = turn == WHITE ? board->white->count.pawns : board->black->count.pawns;
	for(int i = 0; i < pawn_count; i++) num += (turn == WHITE ? board->white->pawns[i] : board->black->pawns[i]) ? 0 : 1;
	return num;
}

short num_attacked_knights(board* board, short turn) {
	short num = 0, knight_count = turn == WHITE ? board->white->count.knights : board->black->count.knights;
	for(int i = 0; i < knight_count; i++) num += (turn == WHITE ? board->white->knights[i] : board->black->knights[i]) ? 0 : 1;
	return num;
}

short num_attacked_bishops(board* board, short turn) {
	short num = 0, bishop_count = turn == WHITE ? board->white->count.bishops : board->black->count.bishops;
	for(int i = 0; i < bishop_count; i++) num += (turn == WHITE ? board->white->bishops[i] : board->black->bishops[i]) ? 0 : 1;
	return num;
}

short num_attacked_rooks(board* board, short turn) {
	short num = 0, rook_count = turn == WHITE ? board->white->count.rooks : board->black->count.rooks;
	for(int i = 0; i < rook_count; i++) num += (turn == WHITE ? board->white->rooks[i] : board->black->rooks[i]) ? 0 : 1;
	return num;
}

short num_attacked_queens(board* board, short turn) {
	short num = 0, queen_count = turn == WHITE ? board->white->count.queens : board->black->count.queens;
	for(int i = 0; i < queen_count; i++) num += (turn == WHITE ? board->white->queen[i] : board->black->queen[i]) ? 0 : 1;
	return num;
}

short num_doubled_blocked_pawns(board* board, short turn) {
	short num = 0, pawn_count = turn == WHITE ? board->white->count.pawns : board->black->count.pawns;
	/* we consider doubled as sub-type of blocked */
	/* hence we directly check if pawn can be blocked by any type of piece */
	uint64_t player_board, opp_board;
	get_player_board_and_opp_board(turn, board, &player_board, &opp_board);
	for(int i = 0; i < pawn_count; i++) {
		switch(turn) {
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
	wprintf(L"num: %d\n", num);
	return num;
}

short num_isolated_pawns(board* board, short turn) {
	short num = 0, pawn_count = turn == WHITE ? board->white->count.pawns : board->black->count.pawns;
	int rank, file;
	uint64_t combined_board = 0ULL, filemask_combined;
	for(int i = 0; i < pawn_count; i++) {
		combined_board |= (turn == WHITE ? board->white->pawns[i] : board->black->pawns[i]);
	}
	for(int i = 0; i < pawn_count; i++) {
		filemask_combined = 0ULL;
		get_rank_and_file_from_bitboard((turn == WHITE ? board->white->pawns[i] : board->black->pawns[i]), &file, &rank);
		if(file > 1) filemask_combined |=  FILEMASK_A<< ((file - 1) - 1);
		if(file < 8) filemask_combined |= FILEMASK_A << ((file + 1) - 1);
		num += (combined_board & filemask_combined) ? 0 : 1;
	}
	wprintf(L"num: %d\n", num);
}


/* Calculates the evaluation of the Board */
double get_evaluation_of_board(board* board) {
	double eval = 0;
	short int legal_moves_array[MAX_LEGAL_MOVES][4];
	eval += 1 * (num_attacked_pawns(board, WHITE) - num_attacked_pawns(board, BLACK));
	eval += 3 * (num_attacked_knights(board, WHITE) - num_attacked_knights(board, BLACK));
	eval += 3 * (num_attacked_bishops(board, WHITE) - num_attacked_bishops(board, BLACK));
	eval += 5 * (num_attacked_rooks(board, WHITE) - num_attacked_rooks(board, BLACK));
	eval += 9 * (num_attacked_queens(board, WHITE) - num_attacked_queens(board, BLACK));
	eval += 0.1 * (get_all_legal_moves(WHITE, board, legal_moves_array) - get_all_legal_moves(BLACK, board, legal_moves_array));
	eval -= 0.5 * (num_doubled_blocked_pawns(board, WHITE) - num_doubled_blocked_pawns(board, BLACK));
	eval -= 0.5 * (num_isolated_pawns(board, WHITE) - num_isolated_pawns(board, BLACK));
	/* TODO: center */
	eval -= in_check(WHITE, board);
	eval += in_check(BLACK, board);
	wprintf(L"eval: %lf\n", eval);
	return eval;
}

void display_evaluation(double eval) {
	double scale = 16.0/39.0;
	double eval_scaled = eval * scale;
	wprintf(L"\n\n\t       [ ");
	for(int i = 0; i < (int)(eval_scaled + 16); i++) wprintf(L"\u2588");
	for(int i = (int)(eval_scaled + 16); i < 32; i++) wprintf(L"-");
	wprintf(L" ]\t%lf\n\n", eval_scaled/4.0);
	return;
}


// double max(double num_1, double num_2) {
// 	return (num_1 > num_2) ? num_1 : num_2;
// }
// double min(double num_1, double num_2) {
// 	return (num_1 < num_2) ? num_1 : num_2;
// }


// double minimax(short int num_legal_moves, double alpha, double beta, short int depth, short int turn) {
// 	if(depth == 0 || num_legal_moves == 0) {
// 		return get_evaluation_of_child_position();
// 	}
// 	/* since white is maximizing player */
// 	num_legal_moves = num_legal_moves_in_position();
	
// 	if(!turn) {
// 		double max_eval = INT_MIN;
// 		for(int i = 0; i < num_legal_moves; i++) {
// 			double eval = minimax(num_legal_moves, alpha, beta, depth - 1, 1);
// 			max_eval = max(max_eval, eval);
// 			alpha = max(alpha, eval);
// 			if(beta <= alpha) {
// 				break;
// 			}
// 		}
// 		return max_eval;
// 	}
// 	else {
// 		double min_eval = INT_MAX;
// 		for(int i = 0; i < num_legal_moves; i++) {
// 			double eval = minimax(num_legal_moves,alpha, beta, depth - 1, 0);
// 			min_eval = min(min_eval, eval);
// 			beta = min(beta, eval);
// 			if(beta <= alpha) {
// 				break;
// 			}
// 		}
// 		return min_eval;
// 	}
// }

// int go_in_depth_and_get_optimal_soln(board* b, uint8_t turn) {
// 	// board* temp_board = copy_board(b);
// 	/* assuming depth = 4 */
// 	short int legal_moves_array_1[MAX_LEGAL_MOVES][4];
// 	short int legal_moves_array_2[MAX_LEGAL_MOVES][4];
// 	short int legal_moves_array_3[MAX_LEGAL_MOVES][4];
// 	short int legal_moves_array_4[MAX_LEGAL_MOVES][4];
// 	int legal_moves_array_1_count = get_all_legal_moves(turn, b, legal_moves_array_1);
// 	int legal_moves_array_2_count = 0;
// 	int legal_moves_array_3_count = 0;
// 	int legal_moves_array_4_count = 0;
// 	// minimax(1, INT_MIN, INT_MAX, 4, turn);
// 	// wprintf(L"num: %d\n", legal_moves_array_1_count);
// 	return 0;
// }