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

void swap(Move** a, Move** b) {
	Move* temp = *a;
	*a = *b;
	*b = temp;
}

int partition(MoveList* list, int low, int high) {
	Move* pivot = list->moves[high];
	int i = low - 1;

	for (int j = low; j <= high - 1; j++) {
		if (list->moves[j]->score > pivot->score) {
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

	// Step 1: Select the correct move list based on the turn
	MoveList* pseudo_legal_moves = maximizing_player == WHITE ? b->white_attacks : b->black_attacks;
	MoveList* legal_moves = maximizing_player == WHITE ? b->white_legal_moves : b->black_legal_moves;

	// Step 2: Clear the pseudo-legal and legal move lists before generating moves
	pseudo_legal_moves->move_count = 0;
	legal_moves->move_count = 0;

	clear_move_list(pseudo_legal_moves);
	clear_move_list(legal_moves);

	update_attacks(b);
	filter_legal_moves(b, maximizing_player);

	int num_legal_moves = legal_moves->move_count;

	// Sort the legal moves based on their scores
	quick_sort(legal_moves, 0, num_legal_moves - 1);

	if (maximizing_player == WHITE) {
		double max_eval = INT_MIN;


		for (int i = 0; i < num_legal_moves; i++) {
			clear_move_list(pseudo_legal_moves);
			update_attacks_for_color(b, WHITE);
			clear_move_list(legal_moves);
			filter_legal_moves(b, WHITE);

			Move m = *(legal_moves->moves[i]);

			square src = {
			    .file = m.src.file,
			    .rank = m.src.rank};
			square dest = {
			    .file = m.dest.file,
			    .rank = m.dest.rank};

			short status = make_move(src, dest, maximizing_player, b, true);

			if (status == INVALID_MOVE) {
				wprintf(L"Invalid move from the engine:\n");
				// wprintf(L"")
				return (evaluated_move){INT_MIN, PLACEHOLDER_MOVE};
			}
			evaluated_move eval = minimax(b, depth - 1, BLACK, alpha, beta);
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
			clear_move_list(pseudo_legal_moves);
			update_attacks_for_color(b, BLACK);
			clear_move_list(legal_moves);
			filter_legal_moves(b, BLACK);

			Move m = *(legal_moves->moves[i]);

			square src = {
			    .file = m.src.file,
			    .rank = m.src.rank};
			square dest = {
			    .file = m.dest.file,
			    .rank = m.dest.rank};

			short status = make_move(src, dest, maximizing_player, b, true);
			if (status == INVALID_MOVE) {
				return (evaluated_move){INT_MAX, PLACEHOLDER_MOVE};
			}
			evaluated_move eval = minimax(b, depth - 1, WHITE, alpha, beta);
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