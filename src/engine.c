#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "chessboard.h"
#include "move_stack.h"
#include "moves.h"
#include "evaluation.h"
#include "engine.h"
#include "move_array.h"
#include "transposition.h"
#include "opening_book.h"

void swap(Move* a, Move* b) {
	Move temp = *a;
	*a = *b;
	*b = temp;
}

int partition(MoveList* list, int low, int high) {
	Move pivot = list->moves[high];
	int i = low - 1;

	for (int j = low; j <= high - 1; j++) {
		if (list->moves[j].score > pivot.score) {
			i++;
			swap(&list->moves[i], &list->moves[j]);
		}
	}

	swap(&list->moves[i + 1], &list->moves[high]);
	return i + 1;
}

void quick_sort(MoveList* list, int low, int high) {
	if (low < high) {
		int pi = partition(list, low, high);

		quick_sort(list, low, pi - 1);
		quick_sort(list, pi + 1, high);
	}
}

evaluated_move minimax(board* b, int depth, short maximizing_player, double alpha, double beta) {
	evaluated_move _move;
	if (depth == 0) {
		_move.evaluation = get_evaluation_of_board(b);
		_move.best_move = PLACEHOLDER_MOVE;
		return _move;
	}

	// check opening book
	Move book_move = get_book_move(&opening_book, b, maximizing_player);
	if (book_move.src.file != 0) {
		_move.evaluation = 0;
		_move.best_move = book_move;
		return _move;
	}

	// check the transposition table
	uint64_t key = get_zobrist_key(b, &transposition_table, maximizing_player);
	Entry* entry = get_entry(&transposition_table, key);

	if (entry && entry->key && entry->depth >= depth) {
		_move.evaluation = entry->evaluation;
		_move.best_move = entry->best_move;
		return _move;
	}


	// Step 1: Select the correct move list based on the turn
	MoveList* pseudo_legal_moves = maximizing_player == WHITE ? b->white_attacks : b->black_attacks;
	MoveList* legal_moves = maximizing_player == WHITE ? b->white_legal_moves : b->black_legal_moves;

	// Step 2: Clear the pseudo-legal and legal move lists before generating moves
	pseudo_legal_moves->move_count = 0;
	legal_moves->move_count = 0;

	clear_move_list(pseudo_legal_moves);
	clear_move_list(legal_moves);

	// update_attacks(b);

	b->black_attacks->move_count = 0;
	update_attacks_for_color(b, !maximizing_player);
	b->white_attacks->move_count = 0;
	update_attacks_for_color(b, maximizing_player);

	filter_legal_moves(b, maximizing_player);

	int num_legal_moves = legal_moves->move_count;

	// Sort the legal moves based on their scores
	quick_sort(legal_moves, 0, num_legal_moves - 1);

	// Backup essential elements in board state
	Move legal_moves_bk[num_legal_moves];
	memcpy(legal_moves_bk, legal_moves->moves, sizeof(Move) * num_legal_moves);

	uint64_t lookup_table_bk[97], *lookup_table_ptr;

	if (maximizing_player == WHITE) {
		lookup_table_ptr = b->white_lookup_table;
		memcpy(lookup_table_bk, b->white_lookup_table, sizeof(uint64_t) * 97);
	} else {
		lookup_table_ptr = b->black_lookup_table;
		memcpy(lookup_table_bk, b->black_lookup_table, sizeof(uint64_t) * 97);
	}
	uint64_t white_board_bk = b->white_board;
	uint64_t black_board_bk = b->black_board;

	if (maximizing_player == WHITE) {
		double max_eval = INT_MIN;
		for (int i = 0; i < num_legal_moves; i++) {
			memcpy(legal_moves->moves, legal_moves_bk, sizeof(Move) * num_legal_moves);
			memcpy(lookup_table_ptr, lookup_table_bk, sizeof(uint64_t) * 97);
			b->white_board = white_board_bk;
			b->black_board = black_board_bk;

			Move m = legal_moves->moves[i];

			square src = {
			    .file = m.src.file,
			    .rank = m.src.rank};
			square dest = {
			    .file = m.dest.file,
			    .rank = m.dest.rank};

			short status = make_move(src, dest, maximizing_player, b, true, m.type);

			if (status == INVALID_MOVE) {
				continue;
			}
			evaluated_move eval = minimax(b, depth - 1, BLACK, alpha, beta);
			
			// insert entry in transposition table
			Entry e = {
				.key = key,
				.evaluation = eval.evaluation,
				.depth = depth,
				.best_move = m,
				.alpha = alpha,
				.beta = beta
			};
			insert_entry(&transposition_table, e);


			unmake_move(b);
			if (eval.evaluation > max_eval) {
				max_eval = eval.evaluation;
				_move.best_move = m;
			}

			alpha = alpha > eval.evaluation ? alpha : eval.evaluation;

			if (beta <= alpha) {
				break;
			}
		}
		_move.evaluation = max_eval;
		return _move;
	} else {
		double min_eval = INT_MAX;
		for (int i = 0; i < num_legal_moves; i++) {
			memcpy(legal_moves->moves, legal_moves_bk, sizeof(Move) * num_legal_moves);
			memcpy(lookup_table_ptr, lookup_table_bk, sizeof(uint64_t) * 97);	

			Move m = legal_moves->moves[i];

			square src = {
			    .file = m.src.file,
			    .rank = m.src.rank};
			square dest = {
			    .file = m.dest.file,
			    .rank = m.dest.rank};

			short status = make_move(src, dest, maximizing_player, b, true, m.type);
			if (status == INVALID_MOVE) {
				continue;
			}
			evaluated_move eval = minimax(b, depth - 1, WHITE, alpha, beta);
			Entry e = {
				.key = key,
				.evaluation = eval.evaluation,
				.depth = depth,
				.best_move = m,
				.alpha = alpha,
				.beta = beta
			};
			insert_entry(&transposition_table, e);

			unmake_move(b);
			if (eval.evaluation < min_eval) {
				min_eval = eval.evaluation;
				_move.best_move = m;
			}
			beta = beta < eval.evaluation ? beta : eval.evaluation;
			if (beta <= alpha) {
				break;
			}
		}
		_move.evaluation = min_eval;
		return _move;
	}
}