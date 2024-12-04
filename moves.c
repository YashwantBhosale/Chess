#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_array.h"
#include "move_stack.h"
#include "evaluation.h"

// helper functions
uint64_t rankmask(int rank) {
	return 0xffULL << (8 * (rank - 1));
}

uint64_t filemask(int file) {
	return 0x0101010101010101ULL << (file - 1);
}

/* This function returns pointer to the piece bitboard with argument as piece is */
uint64_t *get_pointer_to_piece(uint8_t piece_id, board *b) {
	uint8_t type, color, index;
	type = piece_type(piece_id);     // last 3 bits represent type
	color = piece_color(piece_id);   // 4th bit represents color
	index = piece_number(piece_id);  // 5th, 6th and 7th bits represent index

	switch (type) {
		case EMPTY_SQUARE:
			return NULL;
		case PAWN:
			return color == WHITE ? &b->white->pawns[index] : &b->black->pawns[index];

		case KNIGHT:
			return color == WHITE ? &b->white->knights[index] : &b->black->knights[index];

		case BISHOP:
			return color == WHITE ? &b->white->bishops[index] : &b->black->bishops[index];

		case ROOK:
			return color == WHITE ? &b->white->rooks[index] : &b->black->rooks[index];

		case QUEEN:
			return color == WHITE ? &b->white->queen[index] : &b->black->queen[index];

		case KING:
			return color == WHITE ? &b->white->king : &b->black->king;

		default:
			break;
	}
	return NULL;
}

void adjust_type_board_for_make_move(Move move, board *b) {
	uint64_t old_b = get_bitboard(move.src.file, move.src.rank);
	uint64_t new_b = get_bitboard(move.dest.file, move.dest.rank);
	short color = piece_color(move.piece);
	uint64_t *player_type_board = color == WHITE ? &b->white_board : &b->black_board;
	uint64_t *opponent_type_board = color == WHITE ? &b->black_board : &b->white_board;

	switch (move.type) {
		case NORMAL_MOVE:
			*player_type_board &= ~old_b;
			*player_type_board |= new_b;
			break;

		case CAPTURE_MOVE:
			*player_type_board &= ~old_b;
			*player_type_board |= new_b;
			*opponent_type_board &= ~new_b;
			break;

		case EN_PASSANT_MOVE:
			*player_type_board &= ~old_b;
			*player_type_board |= new_b;
			*opponent_type_board &= ~(get_bitboard(move.dest.file, move.src.rank));
			break;

		case CASTLE_MOVE:
			*player_type_board &= ~old_b;
			*player_type_board |= new_b;
			// King side castle
			if (move.dest.file == G) {
				*player_type_board &= ~(get_bitboard(H, move.src.rank));
				*player_type_board |= get_bitboard(F, move.src.rank);
			}
			// Queen side castle
			else if (move.dest.file == C) {
				*player_type_board &= ~(get_bitboard(A, move.src.rank));
				*player_type_board |= get_bitboard(D, move.src.rank);
			}
			break;

		case WHITE_PROMOTES_TO_KNIGHT:
		case WHITE_PROMOTES_TO_ROOK:
		case WHITE_PROMOTES_TO_BISHOP:
		case WHITE_PROMOTES_TO_QUEEN:
		case BLACK_PROMOTES_TO_KNIGHT:
		case BLACK_PROMOTES_TO_ROOK:
		case BLACK_PROMOTES_TO_BISHOP:
		case BLACK_PROMOTES_TO_QUEEN:
			*player_type_board &= ~old_b;
			*player_type_board |= new_b;
			if (move.captured_piece != EMPTY_SQUARE) {
				*opponent_type_board &= ~new_b;
			}
			break;
	}
}

void adjust_type_board_for_unmake_move(Move move, board *b) {
	uint64_t old_b = get_bitboard(move.src.file, move.src.rank);
	uint64_t new_b = get_bitboard(move.dest.file, move.dest.rank);
	short color = piece_color(move.piece);
	uint64_t *player_type_board = color == WHITE ? &b->white_board : &b->black_board;
	uint64_t *opponent_type_board = color == WHITE ? &b->black_board : &b->white_board;

	switch (move.type) {
		case NORMAL_MOVE:
			*player_type_board &= ~new_b;
			*player_type_board |= old_b;
			break;

		case CAPTURE_MOVE:
			*player_type_board &= ~new_b;
			*player_type_board |= old_b;
			*opponent_type_board |= new_b;
			break;

		case EN_PASSANT_MOVE:
			*player_type_board &= ~new_b;
			*player_type_board |= old_b;
			*opponent_type_board |= get_bitboard(move.dest.file, move.src.rank);
			break;

		case CASTLE_MOVE:
			*player_type_board &= ~new_b;
			*player_type_board |= old_b;
			// King side castle
			if (move.dest.file == G) {
				*player_type_board &= ~get_bitboard(F, move.src.rank);
				*player_type_board |= get_bitboard(H, move.src.rank);
			}
			// Queen side castle
			else if (move.dest.file == C) {
				*player_type_board &= ~get_bitboard(D, move.src.rank);
				*player_type_board |= get_bitboard(A, move.src.rank);
			}
			break;

		case WHITE_PROMOTES_TO_KNIGHT:
		case WHITE_PROMOTES_TO_ROOK:
		case WHITE_PROMOTES_TO_BISHOP:
		case WHITE_PROMOTES_TO_QUEEN:
		case BLACK_PROMOTES_TO_KNIGHT:
		case BLACK_PROMOTES_TO_ROOK:
		case BLACK_PROMOTES_TO_BISHOP:
		case BLACK_PROMOTES_TO_QUEEN:
			*player_type_board &= ~new_b;
			*player_type_board |= old_b;
			if (move.captured_piece != EMPTY_SQUARE) {
				*opponent_type_board |= new_b;
			}
			break;
	}
}

uint64_t move_north(uint64_t b) {
	return b << 8;
}

uint64_t move_south(uint64_t b) {
	return b >> 8;
}

uint64_t move_east(uint64_t b) {
	return (b & ~filemask(H)) << 1;
}

uint64_t move_west(uint64_t b) {
	return (b & ~filemask(A)) >> 1;
}

uint64_t move_north_east(uint64_t b) {
	return (b & ~filemask(H)) << 9;
}

uint64_t move_north_west(uint64_t b) {
	return (b & ~filemask(A)) << 7;
}

uint64_t move_south_east(uint64_t b) {
	return (b & ~filemask(H)) >> 7;
}

uint64_t move_south_west(uint64_t b) {
	return (b & ~filemask(A)) >> 9;
}

// ensures you don't step on your own pieces
uint64_t validate_move(uint64_t friendly_pieces, uint64_t move) {
	return move & ~friendly_pieces;
}

// ensures you step on opponent piece
uint64_t validate_capture(uint64_t opponent_pieces, uint64_t move) {
	return move & opponent_pieces;
}

void add_move_to_list(uint64_t piece_position, uint64_t move, uint8_t type, board *b) {
	square src, dest;
	src = get_square_from_bitboard(piece_position);
	dest = get_square_from_bitboard(move);

	uint8_t piece = b->square_table[src.file - 1][src.rank - 1];
	uint8_t captured_piece = b->square_table[dest.file - 1][dest.rank - 1];
	uint8_t color = piece_color(piece);
	uint8_t promoted_piece = 0;
	uint64_t en_passant_square = 0;

	if (captured_piece != EMPTY_SQUARE) {
		type = CAPTURE_MOVE;
	} else if (type == EN_PASSANT_MOVE) {
		captured_piece = b->square_table[dest.file - 1][src.rank - 1];
	} else if ((piece_type(piece) == PAWN) && abs(src.rank - dest.rank) == 2) {
		en_passant_square = get_bitboard(src.file, color == WHITE ? 3 : 6);
	} else if (((type & PROMOTION_MOVE_MASK) == WHITE_PROMOTION_MOVE) || ((type & PROMOTION_MOVE_MASK) == BLACK_PROMOTION_MOVE)) {
		uint8_t piece_type = piece_type_from_promotion_flag(type);
		uint8_t _piece_number = *get_pointer_to_piece_counter(b, piece_type);
		promoted_piece = get_id_of_promoted_piece(piece_type, color, _piece_number + 1);
	}

	Move m = {
	    .src = src,
	    .dest = dest,
	    .piece = piece,
	    .captured_piece = captured_piece,
	    .en_passant_square = en_passant_square,
	    .promoted_piece = promoted_piece,
	    .castle_rights = b->castle_rights,
	    .type = type};

	switch (color) {
		case WHITE:
			add_move(b->white_attacks, m);
			break;
		case BLACK:
			add_move(b->black_attacks, m);
			break;
	}
	return;
}

uint64_t generate_pawn_attacks(uint8_t pawn_id, uint64_t pawn_position, board *b) {
	uint64_t attacks = 0ULL, move = 0ULL, player_board, opponent_board;
	uint8_t color = piece_color(pawn_id);

	player_board = color == WHITE ? b->white_board : b->black_board;
	opponent_board = color == WHITE ? b->black_board : b->white_board;

	switch (color) {
		case WHITE:
			move = validate_move(player_board | opponent_board, move_north(pawn_position));
			if (move & ~(rankmask(8))) {
				add_move_to_list(pawn_position, move, NORMAL_MOVE, b);
				attacks |= move;
			}

			if (pawn_position & rankmask(2) && move) {
				move = validate_move(player_board | opponent_board, move_north(move_north(pawn_position)));
				if (move) {
					add_move_to_list(pawn_position, move, NORMAL_MOVE, b);
					attacks |= move;
				}
			}

			move = validate_capture(opponent_board, move_north_east(pawn_position));
			if (move & ~rankmask(8)) {
				add_move_to_list(pawn_position, move, CAPTURE_MOVE, b);
				attacks |= move;
			}

			move = validate_capture(opponent_board, move_north_west(pawn_position));
			if (move & ~rankmask(8)) {
				add_move_to_list(pawn_position, move, CAPTURE_MOVE, b);
				attacks |= move;
			}

			if (pawn_position & rankmask(5) && b->en_passant_square) {
				move = move_north_east(pawn_position) & ~(player_board | opponent_board);
				if (move == b->en_passant_square) {
					add_move_to_list(pawn_position, move, EN_PASSANT_MOVE, b);
					attacks |= move;
				}

				move = move_north_west(pawn_position) & ~(player_board | opponent_board);
				if (move == b->en_passant_square) {
					add_move_to_list(pawn_position, move, EN_PASSANT_MOVE, b);
					attacks |= move;
				}
			}

			// add promotion moves
			move = validate_move(player_board | opponent_board, move_north(pawn_position));
			uint64_t move_ne = validate_capture(opponent_board, move_north_east(pawn_position));
			uint64_t move_nw = validate_capture(opponent_board, move_north_west(pawn_position));
			if ((move & rankmask(8)) || (move_ne & rankmask(8)) || (move_nw & rankmask(8))) {
				if (move) {
					add_move_to_list(pawn_position, move, WHITE_PROMOTES_TO_QUEEN, b);
					add_move_to_list(pawn_position, move, WHITE_PROMOTES_TO_ROOK, b);
					add_move_to_list(pawn_position, move, WHITE_PROMOTES_TO_BISHOP, b);
					add_move_to_list(pawn_position, move, WHITE_PROMOTES_TO_KNIGHT, b);
				}
				if (move_ne) {
					add_move_to_list(pawn_position, move_ne, WHITE_PROMOTES_TO_QUEEN, b);
					add_move_to_list(pawn_position, move_ne, WHITE_PROMOTES_TO_ROOK, b);
					add_move_to_list(pawn_position, move_ne, WHITE_PROMOTES_TO_BISHOP, b);
					add_move_to_list(pawn_position, move_ne, WHITE_PROMOTES_TO_KNIGHT, b);
				}
				if (move_nw) {
					add_move_to_list(pawn_position, move_nw, WHITE_PROMOTES_TO_QUEEN, b);
					add_move_to_list(pawn_position, move_nw, WHITE_PROMOTES_TO_ROOK, b);
					add_move_to_list(pawn_position, move_nw, WHITE_PROMOTES_TO_BISHOP, b);
					add_move_to_list(pawn_position, move_nw, WHITE_PROMOTES_TO_KNIGHT, b);
				}
				attacks |= (move | move_ne | move_nw);
			}
			break;
		case BLACK: {
			move = validate_move(player_board | opponent_board, move_south(pawn_position));
			if (move & ~(rankmask(1))) {
				add_move_to_list(pawn_position, move, NORMAL_MOVE, b);
				attacks |= move;
			}

			if (pawn_position & rankmask(7) && move) {
				move = validate_move(player_board | opponent_board, move_south(move_south(pawn_position)));
				if (move) {
					add_move_to_list(pawn_position, move, NORMAL_MOVE, b);
					attacks |= move;
				}
			}

			move = validate_capture(opponent_board, move_south_east(pawn_position));
			if (move & ~rankmask(1)) {
				add_move_to_list(pawn_position, move, CAPTURE_MOVE, b);
				attacks |= move;
			}

			move = validate_capture(opponent_board, move_south_west(pawn_position));
			if (move & ~rankmask(1)) {
				add_move_to_list(pawn_position, move, CAPTURE_MOVE, b);
				attacks |= move;
			}

			if (pawn_position & rankmask(4) && b->en_passant_square) {
				move = move_south_east(pawn_position) & ~(player_board | opponent_board);
				if (move == b->en_passant_square) {
					add_move_to_list(pawn_position, move, EN_PASSANT_MOVE, b);
					attacks |= move;
				}

				move = move_south_west(pawn_position) & ~(player_board | opponent_board);
				if (move == b->en_passant_square) {
					add_move_to_list(pawn_position, move, EN_PASSANT_MOVE, b);
					attacks |= move;
				}
			}

			// add promotion moves
			move = validate_move(player_board | opponent_board, move_south(pawn_position));
			uint64_t move_se = validate_capture(opponent_board, move_south_east(pawn_position));
			uint64_t move_sw = validate_capture(opponent_board, move_south_west(pawn_position));

			if ((move & rankmask(1)) || (move_se & rankmask(1)) || (move_sw & rankmask(1))) {
				if (move) {
					add_move_to_list(pawn_position, move, BLACK_PROMOTES_TO_QUEEN, b);
					add_move_to_list(pawn_position, move, BLACK_PROMOTES_TO_ROOK, b);
					add_move_to_list(pawn_position, move, BLACK_PROMOTES_TO_BISHOP, b);
					add_move_to_list(pawn_position, move, BLACK_PROMOTES_TO_KNIGHT, b);
				}
				if (move_se) {
					add_move_to_list(pawn_position, move_se, BLACK_PROMOTES_TO_QUEEN, b);
					add_move_to_list(pawn_position, move_se, BLACK_PROMOTES_TO_ROOK, b);
					add_move_to_list(pawn_position, move_se, BLACK_PROMOTES_TO_BISHOP, b);
					add_move_to_list(pawn_position, move_se, BLACK_PROMOTES_TO_KNIGHT, b);
				}
				if (move_sw) {
					add_move_to_list(pawn_position, move_sw, BLACK_PROMOTES_TO_QUEEN, b);
					add_move_to_list(pawn_position, move_sw, BLACK_PROMOTES_TO_ROOK, b);
					add_move_to_list(pawn_position, move_sw, BLACK_PROMOTES_TO_BISHOP, b);
					add_move_to_list(pawn_position, move_sw, BLACK_PROMOTES_TO_KNIGHT, b);
				}

				attacks |= (move | move_se | move_sw);
			}
		} break;
	}

	return attacks;
}

uint64_t generate_bishop_attacks(uint8_t bishop_id, uint64_t bishop_position, board *b) {
	uint64_t attacks = 0ULL, move = 0ULL, player_board, opponent_board;
	uint8_t color = piece_color(bishop_id);

	player_board = color == WHITE ? b->white_board : b->black_board;
	opponent_board = color == WHITE ? b->black_board : b->white_board;

	move = bishop_position;
	while (move) {
		move = validate_move(player_board, move_north_east(move));
		if (move) {
			add_move_to_list(bishop_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}

	move = bishop_position;
	while (move) {
		move = validate_move(player_board, move_north_west(move));
		if (move) {
			add_move_to_list(bishop_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}

	move = bishop_position;
	while (move) {
		move = validate_move(player_board, move_south_east(move));
		if (move) {
			add_move_to_list(bishop_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}

	move = bishop_position;
	while (move) {
		move = validate_move(player_board, move_south_west(move));
		if (move) {
			add_move_to_list(bishop_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}
	return attacks;
}

uint64_t generate_rook_attacks(uint8_t rook_id, uint64_t rook_position, board *b) {
	uint64_t attacks = 0ULL, move = 0ULL, player_board, opponent_board;
	uint8_t color = piece_color(rook_id);

	player_board = color == WHITE ? b->white_board : b->black_board;
	opponent_board = color == WHITE ? b->black_board : b->white_board;

	move = rook_position;
	while (move) {
		move = validate_move(player_board, move_north(move));
		if (move) {
			add_move_to_list(rook_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}

	move = rook_position;
	while (move) {
		move = validate_move(player_board, move_south(move));
		if (move) {
			add_move_to_list(rook_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}

	move = rook_position;
	while (move) {
		move = validate_move(player_board, move_east(move));
		if (move) {
			add_move_to_list(rook_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}

	move = rook_position;
	while (move) {
		move = validate_move(player_board, move_west(move));
		if (move) {
			add_move_to_list(rook_position, move, NORMAL_MOVE, b);
			attacks |= move;
		}
		if (move & opponent_board) {
			break;
		}
	}
	return attacks;
}

uint64_t generate_queen_attacks(uint8_t queen_id, uint64_t queen_position, board *b) {
	uint64_t attacks = 0ULL;

	attacks |= generate_bishop_attacks(queen_id, queen_position, b);
	attacks |= generate_rook_attacks(queen_id, queen_position, b);

	return attacks;
}

uint64_t generate_knight_attacks(uint8_t knight_id, uint64_t knight_position, board *b) {
	uint64_t attacks = 0ULL, move = 0ULL, player_board;
	uint8_t color = piece_color(knight_id);

	player_board = color == WHITE ? b->white_board : b->black_board;

	move = validate_move(player_board, move_north(move_north_east(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_north(move_north_west(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_east(move_north_east(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_east(move_south_east(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_south(move_south_east(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_south(move_south_west(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_west(move_north_west(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_west(move_south_west(knight_position)));
	if (move) {
		add_move_to_list(knight_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	return attacks;
}

uint64_t validate_castle(uint64_t king_position, short color, board *b) {
	uint64_t player_board, opponent_board, king_side_castle, queen_side_castle, move;

	player_board = color == WHITE ? b->white_board : b->black_board;
	opponent_board = color == WHITE ? b->black_board : b->white_board;

	move = 0ULL;

	uint64_t opponent_attacks = color == WHITE ? b->black_lookup_table[0] : b->white_lookup_table[0];

	if (king_position & opponent_attacks) {
		return 0ULL;
	}

	// king side castle
	if (b->castle_rights & (color == WHITE ? WHITE_KING_SIDE_CASTLE_RIGHTS : BLACK_KING_SIDE_CASTLE_RIGHTS)) {
		move = king_position;
		for (int i = 0; i < 2; i++) {
			move = validate_move(player_board | opponent_board, move_east(move));
			if (move & (player_board | opponent_board)) {
				move = 0ULL;
				break;
			}
			if (move & opponent_attacks) {
				move = 0ULL;
				break;
			}
		}
		king_side_castle = move;
	}

	// queen side castle
	if (b->castle_rights & (color == WHITE ? WHITE_QUEEN_SIDE_CASTLE_RIGHTS : BLACK_QUEEN_SIDE_CASTLE_RIGHTS)) {
		move = king_position;
		for (int i = 0; i < 3; i++) {
			move = validate_move(player_board | opponent_board, move_west(move));
			if (move & (player_board | opponent_board)) {
				move = 0ULL;
				break;
			}
			if (move & opponent_attacks) {
				move = 0ULL;
				break;
			}
		}
		queen_side_castle = move;
	}

	return king_side_castle | queen_side_castle;
}

uint64_t generate_king_attacks(uint8_t king_id, uint64_t king_position, board *b) {
	uint64_t attacks = 0ULL, move = 0ULL, player_board;
	uint8_t color = piece_color(king_id);

	player_board = color == WHITE ? b->white_board : b->black_board;

	move = validate_move(player_board, move_north(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_south(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_east(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_west(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_north_east(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_north_west(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_south_east(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	move = validate_move(player_board, move_south_west(king_position));
	if (move) {
		add_move_to_list(king_position, move, NORMAL_MOVE, b);
		attacks |= move;
	}

	// castle moves
	if ((b->castle_rights & (color == WHITE ? WHITE_CASTLE_RIGHTS : BLACK_CASTLE_RIGHTS)) != 0) {
		move = validate_castle(king_position, color, b);
		int moves_count = __builtin_popcountll(move);
		for (int i = 0; i < moves_count; i++) {
			int index = __builtin_ctzll(move);
			uint64_t _m = 1ULL << index;
			add_move_to_list(king_position, _m, CASTLE_MOVE, b);
			attacks |= _m;
			move &= ~_m;
		}
	}

	return attacks;
}

uint8_t generate_id_for_promoted_piece(uint8_t piece_type, short color, board *b) {
	uint8_t piece_color = color == WHITE ? 0 : 8;
	short *piece_counter = get_pointer_to_piece_counter(b, piece_type);
	if (!piece_counter) return 0;
	(*piece_counter)++;
	uint8_t piece_number = *piece_counter;
	return (piece_color | piece_type | ((piece_number - 1) << 4)) | 64;
}

uint8_t get_id_of_promoted_piece(uint8_t piece_type, short color, short piece_number) {
	uint8_t piece_color = color == WHITE ? 0 : 8;
	return (piece_color | piece_type | ((piece_number - 1) << 4)) | 64;
}

uint8_t new_piece(uint8_t _piece_type, uint8_t color, uint64_t position, board *b) {
	uint8_t piece_id = generate_id_for_promoted_piece(_piece_type, color, b);
	uint64_t **piece_type_ptr = get_pointer_to_piece_type(color, _piece_type, b);

	if (!piece_type_ptr) {
		return 0;
	}

	// Store the result of realloc in a temporary variable
	uint64_t *temp = (uint64_t *)realloc(*piece_type_ptr, sizeof(uint64_t) * (piece_number(piece_id) + 1));

	// Check if realloc succeeded
	if (!temp) {
		fprintf(stderr, "Memory allocation failed\n");
		return 0;
	}

	// Update piece_type_ptr to the new memory location
	*piece_type_ptr = temp;

	// Now use *piece_type_ptr safely
	(*piece_type_ptr)[piece_number(piece_id)] = position;

	square dest = get_square_from_bitboard(position);
	update_square_table(dest.file, dest.rank, piece_id, b);
	return piece_id;
}

uint8_t piece_type_from_promotion_flag(uint8_t flag) {
	switch (flag) {
		case WHITE_PROMOTES_TO_QUEEN:
		case BLACK_PROMOTES_TO_QUEEN:
			return QUEEN;

		case WHITE_PROMOTES_TO_ROOK:
		case BLACK_PROMOTES_TO_ROOK:
			return ROOK;

		case WHITE_PROMOTES_TO_BISHOP:
		case BLACK_PROMOTES_TO_BISHOP:
			return BISHOP;

		case WHITE_PROMOTES_TO_KNIGHT:
		case BLACK_PROMOTES_TO_KNIGHT:
			return KNIGHT;

		default:
			break;
	}
	return 0;
}

bool move(square src, square dest, short move_type, board *b) {
	uint64_t *src_piece, *dest_piece;
	uint8_t piece, dest_piece_id, color;

	piece = b->square_table[src.file - 1][src.rank - 1];
	dest_piece_id = b->square_table[dest.file - 1][dest.rank - 1];

	if (piece == EMPTY_SQUARE) {
		return false;
	}

	color = piece_color(piece);
	src_piece = get_pointer_to_piece(piece, b);

	if (dest_piece_id != EMPTY_SQUARE) {
		dest_piece = get_pointer_to_piece(dest_piece_id, b);
	}

	switch (move_type) {
		case NORMAL_MOVE:
			*src_piece = get_bitboard(dest.file, dest.rank);
			update_square_table(dest.file, dest.rank, piece, b);
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			break;
		case CAPTURE_MOVE:
			*src_piece = get_bitboard(dest.file, dest.rank);
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			update_square_table(dest.file, dest.rank, piece, b);

			*dest_piece = 0ULL;

			b->captured_pieces[color][b->captured_pieces_count[color]] = dest_piece_id;
			b->captured_pieces_count[color]++;
			break;
		case EN_PASSANT_MOVE:
			if (piece_type(piece) == PAWN) {
				uint8_t captured_pawn = b->square_table[dest.file - 1][src.rank - 1];

				if (captured_pawn == EMPTY_SQUARE) {
					return false;
				}
				uint64_t *captured_pawn_ptr = get_pointer_to_piece(captured_pawn, b);
				if (!captured_pawn_ptr) {
					return false;
				}
				*src_piece = get_bitboard(dest.file, dest.rank);
				update_square_table(dest.file, dest.rank, piece, b);
				update_square_table(src.file, src.rank, EMPTY_SQUARE, b);

				*captured_pawn_ptr = 0ULL;
				update_square_table(dest.file, src.rank, EMPTY_SQUARE, b);

				b->captured_pieces[color][b->captured_pieces_count[color]] = captured_pawn;
				b->captured_pieces_count[color]++;
			}
			break;
		case CASTLE_MOVE:
			if (piece_type(piece) != KING) {
				return false;
			}

			if ((b->castle_rights & (color == WHITE ? WHITE_CASTLE_RIGHTS : BLACK_CASTLE_RIGHTS)) == 0) {
				return false;
			}

			if (dest.file == G) {
				uint8_t rook = b->square_table[H - 1][src.rank - 1];
				if (piece_type(rook) != ROOK || piece_color(rook) != color) {
					return false;
				}

				uint64_t *rook_ptr = get_pointer_to_piece(rook, b);
				if (!rook_ptr) {
					return false;
				}

				*rook_ptr = get_bitboard(F, src.rank);
				update_square_table(F, src.rank, rook, b);
				update_square_table(H, src.rank, EMPTY_SQUARE, b);
			} else {
				uint8_t rook = b->square_table[A - 1][src.rank - 1];
				if (piece_type(rook) != ROOK || piece_color(rook) != color) {
					return false;
				}

				uint64_t *rook_ptr = get_pointer_to_piece(rook, b);
				if (!rook_ptr) {
					return false;
				}

				*rook_ptr = get_bitboard(D, src.rank);
				update_square_table(D, src.rank, rook, b);
				update_square_table(A, src.rank, EMPTY_SQUARE, b);
			}
			*src_piece = get_bitboard(dest.file, dest.rank);
			update_square_table(dest.file, dest.rank, piece, b);
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			break;
		case WHITE_PROMOTES_TO_KNIGHT:
		case WHITE_PROMOTES_TO_ROOK:
		case WHITE_PROMOTES_TO_BISHOP:
		case WHITE_PROMOTES_TO_QUEEN:
		case BLACK_PROMOTES_TO_KNIGHT:
		case BLACK_PROMOTES_TO_ROOK:
		case BLACK_PROMOTES_TO_BISHOP:
		case BLACK_PROMOTES_TO_QUEEN:
			uint8_t _p = new_piece(piece_type_from_promotion_flag(move_type), color, get_bitboard(dest.file, dest.rank), b);
			if (!_p) {
				return false;
			}
			*src_piece = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);

			if (dest_piece_id != EMPTY_SQUARE) {
				*dest_piece = 0ULL;

				b->captured_pieces[color][b->captured_pieces_count[color]] = dest_piece_id;
				b->captured_pieces_count[color]++;
			}
			break;

		case INVALID_MOVE:
			break;
	}

	return true;
}

void promotion_move_menu() {
	wprintf(L"Promotion Menu\n");
	wprintf(L"1. Kngiht  ");
	wprintf(L"2. Bishop  ");
	wprintf(L"3. Rook  ");
	wprintf(L"4. Queen\n");
	return;
}

short make_move(square src, square dest, short turn, board *b, bool is_engine) {
	/*
	    THIS FUNCTION WILL:
	    1. indentify the type of the move
	    2. make the move on the board: update the bitboards and square table
	    3. update the move stack
	    4. update the white and black boards
	    5. return the type of the move
	*/

	uint8_t piece, dest_piece, color;
	piece = b->square_table[src.file - 1][src.rank - 1];
	dest_piece = b->square_table[dest.file - 1][dest.rank - 1];
	color = piece_color(piece);

	// wprintf(L"make move 1\n");
	Move m = {
	    .src = src,
	    .dest = dest,
	    .piece = piece,
	    .captured_piece = dest_piece,
	    .en_passant_square = b->en_passant_square,
	    .promoted_piece = 0,
	    .castle_rights = b->castle_rights,
	    .type = INVALID_MOVE};

	// check if the source square is empty
	if (piece == EMPTY_SQUARE) {
		// wprintf(L"Returned from make move because source square is empty.\n");
		return INVALID_MOVE;
	}
	// check if the piece is of the same color as the turn
	if (color != turn) {
		// wprintf(L"Returned from make move because color and turn did not match. (%c,%d) -> (%c,%d)\n", src.file + 'A' - 1, src.rank, dest.file + 'A' - 1, dest.rank);
		return INVALID_MOVE;
	}

	short status;

	status = INVALID_MOVE;

	if (dest_piece != EMPTY_SQUARE && piece_color(dest_piece) == color) {
		return INVALID_MOVE;
	}

	if (dest_piece != EMPTY_SQUARE) {
		status = CAPTURE_MOVE;
	} else if (piece_type(piece) == PAWN) {
		if (dest.file != src.file) {
			if (dest_piece == EMPTY_SQUARE) {
				// en passant move
				status = EN_PASSANT_MOVE;
				m.captured_piece = b->square_table[dest.file - 1][src.rank - 1];

			} else {
				status = CAPTURE_MOVE;
			}
		} else {
			status = NORMAL_MOVE;
		}
	} else if (piece_type(piece) == KING) {
		if (abs(dest.file - src.file) == 2) {
			if (b->castle_rights)
				status = CASTLE_MOVE;
			else
				return INVALID_MOVE;
		} else {
			status = NORMAL_MOVE;
		}
	} else {
		status = NORMAL_MOVE;
	}

	if (piece_type(piece) == PAWN && ((color == BLACK && dest.rank == 1) || (color == WHITE && dest.rank == 8))) {
		// promotion move
		if (!is_engine) {
			promotion_move_menu();
			int choice;
			scanf("%d", &choice);
			switch (choice) {
				case 1:
					status = color == WHITE ? WHITE_PROMOTES_TO_KNIGHT : BLACK_PROMOTES_TO_KNIGHT;
					m.promoted_piece = get_id_of_promoted_piece(KNIGHT, color, color == WHITE ? b->white->count.knights + 1 : b->black->count.knights + 1);
					break;
				case 2:
					status = color == WHITE ? WHITE_PROMOTES_TO_BISHOP : BLACK_PROMOTES_TO_BISHOP;
					m.promoted_piece = get_id_of_promoted_piece(BISHOP, color, color == WHITE ? b->white->count.bishops + 1 : b->black->count.bishops + 1);
					break;
				case 3:
					status = color == WHITE ? WHITE_PROMOTES_TO_ROOK : BLACK_PROMOTES_TO_ROOK;
					m.promoted_piece = get_id_of_promoted_piece(ROOK, color, color == WHITE ? b->white->count.rooks + 1 : b->black->count.rooks + 1);
					break;
				case 4:
					status = color == WHITE ? WHITE_PROMOTES_TO_QUEEN : BLACK_PROMOTES_TO_QUEEN;
					m.promoted_piece = get_id_of_promoted_piece(QUEEN, color, color == WHITE ? b->white->count.queens + 1 : b->black->count.queens + 1);
					break;
				default:
					return INVALID_MOVE;
			}
		} else {
			// IMPORTANT: ENGINE WILL ALWAYS PROMOTE TO QUEEN CHANGE BELOW SNIPPET IF YOU WANT TO MODIFY
			status = color == WHITE ? WHITE_PROMOTES_TO_QUEEN : BLACK_PROMOTES_TO_QUEEN;
			m.promoted_piece = get_id_of_promoted_piece(QUEEN, color, color == WHITE ? b->white->count.queens + 1 : b->black->count.queens + 1);
		}
	}

	/* ================================MOVE===============================================*/

	m.type = status;
	bool flag = false;
	if (get_bitboard(dest.file, dest.rank) & (color == WHITE ? b->white_lookup_table[lookup_index(piece)] : b->black_lookup_table[lookup_index(piece)])) {
		flag = move(src, dest, status, b);
		adjust_type_board_for_make_move(m, b);
		// update_attacks(b);
	} else {
		// wprintf(L"Returned from make move because move is not valid.\n");
		return INVALID_MOVE;
	}
	if (!flag) {
		return INVALID_MOVE;
	}
	push(b->moves, m);

	if (piece_type(piece) == PAWN && abs(src.rank - dest.rank) == 2) {
		b->en_passant_square = get_bitboard(src.file, color == WHITE ? 3 : 6);
	} else {
		b->en_passant_square = 0ULL;
	}

	// update castle rights

	// do not check for castle flags if they are already set to invalid
	if ((b->castle_rights & (color == WHITE ? WHITE_CASTLE_RIGHTS : BLACK_CASTLE_RIGHTS)) == 0)
		return status;

	// set the flags
	// if king moves, remove all castle rights
	if (piece_type(piece) == KING) {
		if (color == WHITE) {
			b->castle_rights &= 0b11110000;
		} else {
			b->castle_rights &= 0b00001111;
		}
	}
	// if rook moves, remove the respective castle rights
	else if (piece_type(piece) == ROOK) {
		if (color == WHITE) {
			if (src.file == A && src.rank == 1) {
				b->castle_rights &= ~WHITE_QUEEN_SIDE_CASTLE_RIGHTS;
			} else if (src.file == H && src.rank == 1) {
				b->castle_rights &= ~WHITE_KING_SIDE_CASTLE_RIGHTS;
			}
		} else {
			if (src.file == A && src.rank == 8) {
				b->castle_rights &= ~BLACK_KING_SIDE_CASTLE_RIGHTS;
			} else if (src.file == H && src.rank == 8) {
				b->castle_rights &= ~BLACK_QUEEN_SIDE_CASTLE_RIGHTS;
			}
		}
	}

	// if two step pawn move, set en passant square

	return status;
}
bool remove_memory_for_promoted_piece(uint8_t promoted_piece, board *b) {
	short *counter = get_pointer_to_piece_counter(b, promoted_piece);
	*counter -= 1;

	uint64_t **type_ptr = get_pointer_to_piece_type(piece_color(promoted_piece), piece_type(promoted_piece), b);
	if (!type_ptr || *counter < 0) {
		return false;
	}

	// Store the result of realloc in a temporary variable
	uint64_t *temp = (uint64_t *)realloc(*type_ptr, *counter * sizeof(uint64_t));

	// Check if realloc succeeded
	if (!temp && *counter > 0) {
		fprintf(stderr, "Memory reallocation failed\n");
		return false;
	}

	// Update type_ptr to the new memory location
	*type_ptr = temp;

	return true;
}

void undo_promotion(Move last_move, board *b) {
	// restore the pawn to the source square
	uint64_t *pawn_ptr = get_pointer_to_piece(last_move.piece, b);
	*pawn_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
	update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);

	// remove the promoted piece
	uint64_t *promoted_piece_ptr = get_pointer_to_piece(last_move.promoted_piece, b);
	*promoted_piece_ptr = 0ULL;

	if (last_move.captured_piece != EMPTY_SQUARE) {
		uint64_t *captured_piece_ptr = get_pointer_to_piece(last_move.captured_piece, b);
		*captured_piece_ptr = get_bitboard(last_move.dest.file, last_move.dest.rank);
		update_square_table(last_move.dest.file, last_move.dest.rank, last_move.captured_piece, b);

		b->captured_pieces_count[piece_color(last_move.piece)]--;
	} else {
		update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);
	}
	// deallocate memory for the promoted piece
	remove_memory_for_promoted_piece(last_move.promoted_piece, b);

	// restore castle rights and attack tables
	b->castle_rights = last_move.castle_rights;
	b->en_passant_square = last_move.en_passant_square;
	return;
}

short unmake_move(board *b) {
	Move last_move = pop(b->moves);
	if (last_move.type == INVALID_MOVE) {
		return INVALID_MOVE;
	}
	switch (last_move.type) {
		case NORMAL_MOVE:
			/*
			    1. restore the piece bitboard to the biboard of the source square
			    2. update the square table to reflect it
			    3. restore the destination square to empty
			    4. (additional) restore the castle rights, en passant square
			*/
			uint64_t *piece_ptr = get_pointer_to_piece(last_move.piece, b);
			if (!piece_ptr) {
				return false;
			}
			*piece_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			// restore castle rights
			b->castle_rights = last_move.castle_rights;
			b->en_passant_square = last_move.en_passant_square;
			break;

		case CAPTURE_MOVE:
			/*
			    1. restore the piece bitboard to the bitboard of the source square
			    2. update the square table to reflect it
			    3. restore the destination square to the captured piece
			    4. restore the captured piece bitboard
			    5. (optional) restore the castle rights
			*/
			uint64_t *src_piece_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *dest_piece_ptr = get_pointer_to_piece(last_move.captured_piece, b);

			if (!src_piece_ptr || !dest_piece_ptr) {
				return false;
			}

			*src_piece_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);

			*dest_piece_ptr = get_bitboard(last_move.dest.file, last_move.dest.rank);
			update_square_table(last_move.dest.file, last_move.dest.rank, last_move.captured_piece, b);

			// restore castle rights
			b->castle_rights = last_move.castle_rights;
			b->en_passant_square = last_move.en_passant_square;
			b->captured_pieces_count[piece_color(last_move.piece)]--;
			break;

		case EN_PASSANT_MOVE:
			/*
			    1. restore the piece bitboard to the bitboard of the source square
			    2. update the square table to reflect it
			    3. restore the destination square to empty
			    4. restore the captured piece bitboard
			    5. update the square table to reflect it
			    6. (optional) restore the castle rights
			*/
			uint64_t *source_piece_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *captured_piece_ptr = get_pointer_to_piece(last_move.captured_piece, b);

			if (!source_piece_ptr || !captured_piece_ptr) {
				return false;
			}

			*source_piece_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			*captured_piece_ptr = get_bitboard(last_move.dest.file, last_move.src.rank);
			update_square_table(last_move.dest.file, last_move.src.rank, last_move.captured_piece, b);

			// restore castle rights
			b->castle_rights = last_move.castle_rights;
			b->en_passant_square = last_move.en_passant_square;
			b->captured_pieces_count[piece_color(last_move.piece)]--;
			break;

		case CASTLE_MOVE:
			/*
			    1. restore the piece bitboard to the bitboard of the source square
			    2. update the square table to reflect it
			    3. restore the destination square to empty
			    4. restore the rook bitboard
			    5. update the square table to reflect it
			    6. (optional) restore the castle rights
			*/
			uint64_t *king_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *rook_ptr = NULL;

			if (!king_ptr) {
				return false;
			}

			*king_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			// king side castle
			if (last_move.dest.file == G) {
				uint8_t moved_rook = b->square_table[F - 1][last_move.src.rank - 1];
				rook_ptr = get_pointer_to_piece(moved_rook, b);

				if (!rook_ptr) {
					wprintf(L"Rook pointer is null move couldn't be undone\n");
					// we should ideally restore king to its original position
					return false;
				}

				*rook_ptr = get_bitboard(H, last_move.src.rank);
				update_square_table(H, last_move.src.rank, moved_rook, b);
				update_square_table(F, last_move.src.rank, EMPTY_SQUARE, b);
			}

			// queen side castle
			else if (last_move.dest.file == C) {
				uint8_t moved_rook = b->square_table[D - 1][last_move.src.rank - 1];
				rook_ptr = get_pointer_to_piece(moved_rook, b);

				if (!rook_ptr) {
					wprintf(L"Rook pointer is null move couldn't be undone\n");
					// we should ideally restore king to its original position

					/*
					    *king_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
					    update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
					    update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);
					    push(b->moves, last_move);
					*/
					return false;
				}

				*rook_ptr = get_bitboard(A, last_move.src.rank);
				update_square_table(A, last_move.src.rank, moved_rook, b);
				update_square_table(D, last_move.src.rank, EMPTY_SQUARE, b);
			} else {
				// restore king to its original position
				*king_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
				update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
				update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);
				push(b->moves, last_move);
				return false;
			}

			// restore castle rights
			b->castle_rights = last_move.castle_rights;
			b->en_passant_square = last_move.en_passant_square;
			break;
		case WHITE_PROMOTES_TO_KNIGHT:
		case WHITE_PROMOTES_TO_BISHOP:
		case WHITE_PROMOTES_TO_ROOK:
		case WHITE_PROMOTES_TO_QUEEN:
		case BLACK_PROMOTES_TO_KNIGHT:
		case BLACK_PROMOTES_TO_BISHOP:
		case BLACK_PROMOTES_TO_ROOK:
		case BLACK_PROMOTES_TO_QUEEN:
			undo_promotion(last_move, b);
			break;
		default:
			break;
	}

	adjust_type_board_for_unmake_move(last_move, b);
	return true;
}

void update_type_board(board *b, short turn) {
	if (turn == WHITE) {
		b->white_board = white_board(b);
		b->black_board = black_board(b);
	} else {
		b->black_board = black_board(b);
		b->white_board = white_board(b);
	}
}

int lookup_index(uint8_t id) {
	int promotion_flag = id & 64 ? 1 : 0;
	int _piece_type = piece_type(id);
	int _piece_number = piece_number(id);

	int index = 1 + (promotion_flag * 48) + (_piece_type * 8) + _piece_number;
	return index;
}

void update_attacks_for_color(board *b, short color) {
	uint64_t white_attacks = 0ULL, black_attacks = 0ULL, piece_attacks = 0ULL;
	int standard_bishop_knight_rooks = 2;
	int standard_queens = 1;

	short cursor = 0;
	switch (color) {
		case WHITE:

			cursor = WHITE_PAWN_1;
			for (int i = 0; i < b->white->count.pawns; i++) {
				piece_attacks = generate_pawn_attacks(cursor, b->white->pawns[i], b);
				b->white_lookup_table[lookup_index(cursor)] = piece_attacks;
				white_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = WHITE_KNIGHT_1;
			for (int i = 0; i < b->white->count.knights; i++) {
				if (i > standard_bishop_knight_rooks)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_knight_attacks(cursor, b->white->knights[i], b);
				b->white_lookup_table[lookup_index(cursor)] = piece_attacks;
				white_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = WHITE_BISHOP_1;
			for (int i = 0; i < b->white->count.bishops; i++) {
				if (i > standard_bishop_knight_rooks)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_bishop_attacks(cursor, b->white->bishops[i], b);

				b->white_lookup_table[lookup_index(cursor)] = piece_attacks;
				white_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = WHITE_ROOK_1;
			for (int i = 0; i < b->white->count.rooks; i++) {
				if (i > standard_bishop_knight_rooks)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_rook_attacks(cursor, b->white->rooks[i], b);
				b->white_lookup_table[lookup_index(cursor)] = piece_attacks;
				white_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = WHITE_QUEEN;
			for (int i = 0; i < b->white->count.queens; i++) {
				if (i > standard_queens)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_queen_attacks(cursor, b->white->queen[i], b);
				b->white_lookup_table[lookup_index(cursor)] = piece_attacks;
				white_attacks |= piece_attacks;
				cursor += 16;
			}

			piece_attacks = generate_king_attacks(WHITE_KING, b->white->king, b);
			b->white_lookup_table[lookup_index(WHITE_KING)] = piece_attacks;
			white_attacks |= piece_attacks;

			b->white_lookup_table[0] = white_attacks;
			break;
		case BLACK:

			cursor = BLACK_PAWN_1;
			for (int i = 0; i < b->black->count.pawns; i++) {
				piece_attacks = generate_pawn_attacks(cursor, b->black->pawns[i], b);
				b->black_lookup_table[lookup_index(cursor)] = piece_attacks;
				black_attacks |= piece_attacks;
				cursor += 16;
			}
			cursor = BLACK_KNIGHT_1;
			for (int i = 0; i < b->black->count.knights; i++) {
				if (i > standard_bishop_knight_rooks)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_knight_attacks(cursor, b->black->knights[i], b);
				b->black_lookup_table[lookup_index(cursor)] = piece_attacks;
				black_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = BLACK_BISHOP_1;
			for (int i = 0; i < b->black->count.bishops; i++) {
				if (i > standard_bishop_knight_rooks)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_bishop_attacks(cursor, b->black->bishops[i], b);
				b->black_lookup_table[lookup_index(cursor)] = piece_attacks;
				black_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = BLACK_ROOK_1;
			for (int i = 0; i < b->black->count.rooks; i++) {
				if (i > standard_bishop_knight_rooks)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_rook_attacks(cursor, b->black->rooks[i], b);
				b->black_lookup_table[lookup_index(cursor)] = piece_attacks;
				black_attacks |= piece_attacks;
				cursor += 16;
			}

			cursor = BLACK_QUEEN;
			for (int i = 0; i < b->black->count.queens; i++) {
				if (i > standard_queens)
					cursor |= 64;  // set the MSB to 1

				piece_attacks = generate_queen_attacks(cursor, b->black->queen[i], b);
				b->black_lookup_table[lookup_index(cursor)] = piece_attacks;
				black_attacks |= piece_attacks;
				cursor += 16;
			}
			piece_attacks = generate_king_attacks(BLACK_KING, b->black->king, b);
			b->black_lookup_table[lookup_index(BLACK_KING)] = piece_attacks;
			black_attacks |= piece_attacks;

			b->black_lookup_table[0] = black_attacks;
			break;
		default:
			break;
	}
}

void update_attacks(board *b) {
	b->white_attacks->move_count = 0;
	update_attacks_for_color(b, WHITE);

	b->black_attacks->move_count = 0;
	update_attacks_for_color(b, BLACK);
	return;
}

bool in_check(short color, board *b) {
	uint64_t king_position = color == WHITE ? b->white->king : b->black->king;

	if (color == WHITE) {
		b->black_attacks->move_count = 0;
	} else {
		b->white_attacks->move_count = 0;
	}
	clear_move_list(color == WHITE ? b->black_attacks : b->white_attacks);
	update_attacks_for_color(b, color == WHITE ? BLACK : WHITE);

	uint64_t opponent_attacks = color == WHITE ? b->black_lookup_table[0] : b->white_lookup_table[0];

	return (king_position & opponent_attacks);
}

bool in_check_alt(short color, board *b) {
	/*
	 * ideas for more efficient implementation
	   1. the inefficient thing we are doing above is we are recalculating attacks of every piece
	      of opponent again and again to check if player's king is in check
	   2. while in true sense only pieces that matter are those from which the player's king is
	      reachable we should only update their attacks and not entire pieces
	   3. so we will shoot up rays from the player's king in all 8 directions and find all the
	      opponent pieces which are in those rays and only update their attacks to check if it is
	      a check to player's king
	      (if it is a sliding piece that we are encountering then we don't need to update attacks
	       we can just straight up conclude that it's a check)
	    4. we need to specially handle case for knight by genrating knight attacks from king's
	       position and checking if any opponent knight is there at that position
	    5. also we need to specially take care of the discovered attacks, pins etc.

	    ... more things to consider
	 */

	uint64_t king_position = color == WHITE ? b->white->king : b->black->king;
	int king_rank, king_file;
	get_rank_and_file_from_bitboard(king_position, &king_file, &king_rank);

	// check northward
	for (int i = king_rank + 1; i <= 8; i++) {
		uint8_t piece = b->square_table[king_file - 1][i - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}
		if (piece_type(piece) == ROOK || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check southward
	for (int i = king_rank - 1; i >= 1; i--) {
		uint8_t piece = b->square_table[king_file - 1][i - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}
		if (piece_type(piece) == ROOK || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check eastward
	for (int i = king_file + 1; i <= H; i++) {
		uint8_t piece = b->square_table[i - 1][king_rank - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}
		if (piece_type(piece) == ROOK || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check westward
	for (int i = king_file - 1; i >= A; i--) {
		uint8_t piece = b->square_table[i - 1][king_rank - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}
		if (piece_type(piece) == ROOK || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check north-eastward
	for (int i = king_file + 1, j = king_rank + 1; i <= H && j <= 8; i++, j++) {
		uint8_t piece = b->square_table[i - 1][j - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}

		if (piece_type(piece) == PAWN) {
			if (color == WHITE && j == king_rank + 1) {
				return true;
			}

		}

		if (piece_type(piece) == BISHOP || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check south-eastward
	for (int i = king_file + 1, j = king_rank - 1; i <= H && j >= 1; i++, j--) {
		uint8_t piece = b->square_table[i - 1][j - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}

		if (piece_type(piece) == PAWN) {
			if (color == BLACK && j == king_rank - 1) {
				return true;
			}
		}

		if (piece_type(piece) == BISHOP || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check south-westward
	for (int i = king_file - 1, j = king_rank - 1; i >= A && j >= 1; i--, j--) {
		uint8_t piece = b->square_table[i - 1][j - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}

		if(piece_type(piece) == PAWN) {
			if(color == BLACK && j == king_rank - 1) {
				return true;
			}
		}

		if (piece_type(piece) == BISHOP || piece_type(piece) == QUEEN) {
			return true;
		}
		break;
	}

	// check north-westward
	for (int i = king_file - 1, j = king_rank + 1; i >= A && j <= 8; i--, j++) {
		uint8_t piece = b->square_table[i - 1][j - 1];
		if (piece == EMPTY_SQUARE) {
			continue;
		}
		if (piece_color(piece) == color) {
			break;
		}
		// check for pawn
		if (piece_type(piece) == PAWN) {
			if (color == WHITE && j == king_rank + 1) {
				return true;
			}
		}

		if (piece_type(piece) == BISHOP || piece_type(piece) == QUEEN) {
			return true;
		}

		break;
	}

	// check for knights
	int knight_moves[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
	for (int i = 0; i < 8; i++) {
		int new_file = king_file + knight_moves[i][0];
		int new_rank = king_rank + knight_moves[i][1];
		if (new_file >= A && new_file <= H && new_rank >= 1 && new_rank <= 8) {
			uint8_t piece = b->square_table[new_file - 1][new_rank - 1];
			if (piece == EMPTY_SQUARE) {
				continue;
			}
			if (piece_color(piece) == !color && piece_type(piece) == KNIGHT) {
				return true;
			}
		}
	}

	return false;
}

void print_move(Move m) {
	wprintf(L"(%c, %d) -> (%c, %d), piece : %d, captured piece : %d, promoted piece : %d, castle rights : %d, type : %d\n", m.src.file + 'A' - 1, m.src.rank, m.dest.file + 'A' - 1, m.dest.rank, m.piece, m.captured_piece, m.promoted_piece, m.castle_rights, m.type);
}

unsigned int get_score(Move m, board *b) {
	/*
	    Ideas:
	    1. In general, a capture move has greater probability of being great move
	    2. A move that puts the opponent in check is a good move
	    3. A move that puts the opponent in checkmate is the best move (PENDING)
	    4. A move that puts the opponent in stalemate is the worst move (PENDING)
	    5. promotion moves are generally good moves

	    more polished ideas:
	    1. A move that involves capture of high value piece with a low value piece is a good move
	       here, we will assign extra points to the move
	    2. promotion + capture is a great move
	    3. promotion + check is a great move
	    4. castling is generally a good move (especially if the 7th or 2nd rank pawns are still there)
	    5. a move that reduces opponent's number of legal moves is a good move (here we can just
	       check the pseudo legal moves as filtering opponent's legal moves is again compuationally
	       expensive).
	    6. a move that increases the number of legal moves for the player is a good move (Not sure
	       about this one)

	    more specific to piece ideas:
	    1. A move that encourages knights and bishops to move to the center of the board is a good move
	       (we will assign very little extra points to the move so that this is only done when all other
	        moves are almost equal)
	        NOTE: here if both the pieces have equal probability of moving to the center, we will choose
	        knight, as it seems more useless at the corner of the board

	    there is no computationally cheap way to determine if a move is a checkmate or stalemate
	    at the current design so we will not consider them for now
	*/

	unsigned int score = 0;

	// 1. capture move advantage
	if (m.type == CAPTURE_MOVE) {
		score += 10;
	}

	// 2. check move advantage
	if (m.is_check) {
		score += 20;
	}

	// 3. promotion move advantage
	if (m.promoted_piece != 0) {
		score += 15;
	}

	// 4. promotion + capture move advantage
	if (m.type == CAPTURE_MOVE && m.promoted_piece != 0) {
		score += 10;
	}

	// 5. promotion + check move advantage
	if (m.captured_piece != 0) {
		score += 15;
	}

	// 6. castling move advantage
	if (m.type == CASTLE_MOVE) {
		score += 10;
	}

	// 7. move that encourages knights and bishops to move to the center of the board
	if (piece_type(m.piece) == KNIGHT || piece_type(m.piece) == BISHOP) {
		if (m.dest.file == D || m.dest.file == E || m.dest.file == D || m.dest.file == F) {
			if (m.dest.rank == 4 || m.dest.rank == 5 || m.dest.rank == 4 || m.dest.rank == 5) {
				score += 5;
			}
		}
	}

	// (additional) centre pawn moves are encouraged (when all other moves are almost equal)
	if (piece_type(m.piece) == PAWN) {
		if (m.dest.file == D || m.dest.file == E) {
			if (m.dest.rank == 4 || m.dest.rank == 5) {
				score += 2;
			}
		}
	}

	return score;
}

void filter_legal_moves(board *b, short turn) {
	MoveList *pseudo_legal_moves = turn == WHITE ? b->white_attacks : b->black_attacks;   // pseudo legal
	MoveList *legal_moves = turn == WHITE ? b->white_legal_moves : b->black_legal_moves;  // legal

	if (!pseudo_legal_moves || !pseudo_legal_moves->moves || pseudo_legal_moves->move_count == 0) {
		wprintf(L"No pseudo-legal moves to filter for %s.\n", turn == WHITE ? "WHITE" : "BLACK");
		return;
	}

	// Clear the legal moves list to start fresh
	clear_move_list(legal_moves);

	for (int i = 0; i < pseudo_legal_moves->move_count; i++) {
		Move *current_move_ptr = pseudo_legal_moves->moves[i];
		if (!current_move_ptr) continue;

		Move current_move = *current_move_ptr;

		short status = make_move(current_move.src, current_move.dest, turn, b, true);

		if (status == INVALID_MOVE) {
			continue;
		}

		if (!in_check_alt(turn, b)) {
			// Check if the move puts the opponent in check
			if (turn == WHITE) {
				if (b->white_lookup_table[0] & b->black->king) {
					// If the move puts the opponent in check, it's a legal move
					current_move.is_check = true;
				} else {
					current_move.is_check = false;
				}
			} else {
				if (b->black_lookup_table[0] & b->white->king) {
					// If the move puts the opponent in check, it's a legal move
					current_move.is_check = true;
				} else {
					current_move.is_check = false;
				}
			}

			current_move.score = get_score(current_move, b);

			// If the move does not leave the player in check, it's legal
			add_move(legal_moves, current_move);
		}else {
			// remove from lookup table
			if (turn == WHITE) {
				uint64_t dest_bb = get_bitboard(current_move.dest.file, current_move.dest.rank);
				b->white_lookup_table[lookup_index(current_move.piece)] &= ~dest_bb;
				b->white_lookup_table[0] &= ~dest_bb;

			} else {
				uint64_t dest_bb = get_bitboard(current_move.dest.file, current_move.dest.rank);
				b->black_lookup_table[lookup_index(current_move.piece)] &= ~dest_bb;
				b->black_lookup_table[0] &= ~dest_bb;
			}
		}

		unmake_move(b);  // Undo the move to restore board state
	}
}

/*

void filter_legal_moves(board *b, short turn) {
    MoveList *list = turn == WHITE ? b->white_attacks : b->black_attacks;
    if (!list || !list->head) return;

    wprintf(L"Filtering moves: turn = %c\n", turn == WHITE ? 'W' : 'B');
    print_movelist(list);


    MoveNode *p = list->head;
    int count = list->move_count;
    int i = 0;
    while (p != NULL && i < count) {
        MoveNode *next = p->next;  // Store the next node before potentially removing `p`

        short status = make_move((p->move).src, p->move.dest, turn, b, true);
        if (status == INVALID_MOVE || in_check(turn, b)) {
            // Remove the move if it's invalid or puts the king in check
            remove_node(list, p);
            if (turn == WHITE) {
                b->white_lookup_table &= ~(get_bitboard((p->move).dest.file, (p->move).dest.rank));
            } else {
                b->black_lookup_table &= ~(get_bitboard((p->move).dest.file, (p->move).dest.rank));
            }
        }
        if(status != INVALID_MOVE) {
            unmake_move(b);  // Undo the move on the board
        }
        p = next;  // Move to the next node

        i++;
    }
    wprintf(L"Filtered moves\n");
    return;
}

*/
