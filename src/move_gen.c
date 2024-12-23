#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include "bitboard.h"
#include "chessboard.h"
#include "move_gen.h"

#define ADD_MOVE_TO_LIST(b, current_pawn, m, list, p)           \
	_move.move = construct_move((b), (current_pawn), (m), (p)); \
	add_move_to_list((list), _move);

enum Directions { DIAGONAL,
	              NON_DIAGONAL };

static inline Bitboard move_north(Bitboard position) {
	return (position << 8);
}

static inline Bitboard move_south(Bitboard position) {
	return (position >> 8);
}

static inline Bitboard move_east(Bitboard position) {
	return (position & ~H_FILE) << 1;
}

static inline Bitboard move_west(Bitboard position) {
	return (position & ~A_FILE) >> 1;
}

static inline Bitboard move_north_east(Bitboard position) {
	return (position & ~H_FILE) << 9;
}

static inline Bitboard move_north_west(Bitboard position) {
	return (position & ~A_FILE) << 7;
}

static inline Bitboard move_south_east(Bitboard position) {
	return (position & ~H_FILE) >> 7;
}

static inline Bitboard move_south_west(Bitboard position) {
	return (position & ~A_FILE) >> 9;
}

static inline Bitboard validate_capture(Bitboard move, Bitboard opponent_pieces) {
	return (move & opponent_pieces);
}

static inline Bitboard validate_move(Bitboard move, Bitboard friendly_pieces) {
	return (move & ~friendly_pieces);
}

// this function assumes that basic check of castling to particular side
// is within player's right, it just confirms that all the squares between
// king and rook are check-free and unoccupied
static Bitboard validate_castle(Board *b, Byte color, Byte side) {
	Bitboard king, opponent_attacks, player_board, opponent_board, move = 0ULL;

	player_board = color ? b->black_board : b->white_board;
	opponent_board = color ? b->white_board : b->black_board;

	king = color ? b->black.king : b->white.king;
	opponent_attacks = b->lookup_tables[color][0];

	if (king & opponent_attacks) {
		return 0ULL;
	}

	switch (side) {
		case KING_SIDE_CASTLE:
			move = king;
			for (int i = 0; i < 2; i++) {
				move = validate_move(move_east(move), player_board | opponent_board);
				if (!move) {
					return 0ULL;
				}
				// potential bug: pawn attacks
				if (move & opponent_attacks) {
					return 0ULL;
				}
			}
			return move;
		case QUEEN_SIDE_CASTLE:
			move = king;
			for (int i = 0; i < 2; i++) {
				move = validate_move(move_west(move), player_board | opponent_board);
				if (!move) {
					return 0ULL;
				}
				// potential bug: pawn attacks
				if (move & opponent_attacks) {
					return 0ULL;
				}
			}

			if (move_west(move) & (player_board | opponent_board))
				return 0ULL;

			return move;
		default:
			break;
	}

	return 0ULL;
}

typedef struct {
	Byte piece;
	Byte direction;
	Byte distance;
} RayData;

static int ray(Board *b, Square src, int d_file, int d_rank, Byte direction, RayData *encountered_pieces) {
	int i = 0;
	short cursor_file = src.file + d_file;
	short cursor_rank = src.rank + d_rank;
	Byte distance = 0;

	while (cursor_file >= A && cursor_file <= H && cursor_rank >= 1 && cursor_rank <= 8) {
		uint8_t piece = b->square_table[cursor_file - 1][cursor_rank - 1];
		if (piece != EMPTY_SQUARE) {
			RayData data;
			data.piece = piece;
			data.distance = distance;
			data.direction = direction;

			encountered_pieces[i++] = data;
			break;  // Stop after encountering a piece
		}
		cursor_file += d_file;
		cursor_rank += d_rank;
		distance++;
	}

	return i;
}

static int raycast(Board *b, Square src, short color, RayData *encountered_pieces) {
	int i = 0;

	int directions[8][3] = {
	    {0, 1, NON_DIAGONAL},
	    {0, -1, NON_DIAGONAL},
	    {1, 0, NON_DIAGONAL},
	    {-1, 0, NON_DIAGONAL},
	    {1, 1, DIAGONAL},
	    {-1, 1, DIAGONAL},
	    {1, -1, DIAGONAL},
	    {-1, -1, DIAGONAL}};

	for (int d = 0; d < 8; d++) {
		i += ray(b, src, directions[d][0], directions[d][1], directions[d][2], encountered_pieces + i);
	}

	return i;
}

static bool is_knight_check(Board *b, Square king_sq, Byte color) {
	int knight_moves[8][2] = {
	    {1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

	for (int i = 0; i < 8; i++) {
		int file = king_sq.file + knight_moves[i][0];
		int rank = king_sq.rank + knight_moves[i][1];
		if (file >= A && file <= H && rank >= 1 && rank <= 8) {
			Byte piece = b->square_table[file - 1][rank - 1];
			if (PIECE_TYPE(piece) == KNIGHT && PIECE_CLR(piece) != color)
				return true;
		}
	}
	return false;
}

static bool is_pawn_check(Board *b, Square king_sq, Byte color) {
	int pawn_direction = color ? 1 : -1;  // North : South
	int pawn_moves[2][2] = {{1, pawn_direction}, {-1, pawn_direction}};

	for (int i = 0; i < 2; i++) {
		int file = king_sq.file + pawn_moves[i][0];
		int rank = king_sq.rank + pawn_moves[i][1];
		if (file >= A && file <= H && rank >= 1 && rank <= 8) {
			Byte piece = b->square_table[file - 1][rank - 1];
			if (PIECE_TYPE(piece) == PAWN && PIECE_CLR(piece) != color)
				return true;
		}
	}
	return false;
}

bool in_check(Board *b, Byte color) {
	RayData encountered_pieces[8];
	Square src = get_square_from_bitboard(color ? b->black.king : b->white.king);

	int num_encountered_pieces = raycast(b, src, color, encountered_pieces);
	if (num_encountered_pieces == 0)
		return false;

	for (int i = 0; i < num_encountered_pieces; i++) {
		RayData current = encountered_pieces[i];
		Byte type = PIECE_TYPE(current.piece);

		if (PIECE_CLR(current.piece) == color)
			continue;

		if ((type == ROOK || type == QUEEN) && current.direction == NON_DIAGONAL)
			return true;

		if (type == KING && current.distance == 1)
			return true;

		if ((type == BISHOP || type == QUEEN) && current.direction == DIAGONAL)
			return true;
	}

	if (is_knight_check(b, src, color))
		return true;

	if (is_pawn_check(b, src, color))
		return true;

	return false;
}

static unsigned construct_move(Board *b, Bitboard src, Bitboard dest, Byte promoted_piece) {
	int from, to, en_passant_sq = 0;
	Byte moved_piece, captured_piece, flag, color;

	int src_f, src_r, dest_f, dest_r;
	get_file_and_rank_from_bitboard(src, &src_f, &src_r);
	get_file_and_rank_from_bitboard(dest, &dest_f, &dest_r);

	from = ((src_f - 1) << 3) | (src_r - 1);
	to = ((dest_f - 1) << 3) | (dest_r - 1);

	moved_piece = b->square_table[src_f - 1][src_r - 1];
	color = PIECE_CLR(moved_piece);
	captured_piece = b->square_table[dest_f - 1][dest_r - 1];

	if (captured_piece == EMPTY_SQUARE) {
		flag = NORMAL_MOVE;
	} else {
		flag = CAPTURE_MOVE;
	}

	if (moved_piece == PAWN && src_f != dest_f && captured_piece == EMPTY_SQUARE) {
		flag = EN_PASSANT_MOVE;
		captured_piece = b->square_table[dest_f - 1][src_r - 1];
	}
	if (moved_piece == PAWN && src_r == (color ? 2 : 7)) {
		flag = PROMOTION_MOVE;
	}

	return (from | (to << 6) | (moved_piece << 12) | (captured_piece << 15) | (promoted_piece << 18) | (en_passant_sq << 21) | (flag << 27));
}

static void add_move(Board *b, MoveList *list, Bitboard current_pos, Bitboard move, Byte promoted_piece) {
	Move _move;
	_move.move = construct_move(b, current_pos, move, promoted_piece);
	append_move(list, _move);
}

void generate_promotions(Board *b, MoveList *list, Bitboard current_pawn, Bitboard move, Byte color) {
	add_move(b, list, current_pawn, move, color ? BLACK_KNIGHT : WHITE_KNIGHT);
	add_move(b, list, current_pawn, move, color ? BLACK_BISHOP : WHITE_BISHOP);
	add_move(b, list, current_pawn, move, color ? BLACK_ROOK : WHITE_ROOK);
	add_move(b, list, current_pawn, move, color ? BLACK_QUEEN : WHITE_QUEEN);
}

Bitboard generate_pawn_attacks(Board *b, MoveList *list, Byte color) {
	Bitboard pawns = color ? b->black.pawns : b->white.pawns;
	Bitboard player_board = color ? b->black_board : b->white_board;
	Bitboard opponent_board = color ? b->white_board : b->black_board;
	Bitboard attacks = 0ULL;

	while (pawns) {
		Bitboard current_pawn = (1ULL << (ffsll(pawns) - 1));
		pawns &= ~current_pawn;

		Bitboard move = validate_move(color ? move_south(current_pawn) : move_north(current_pawn), player_board | opponent_board);
		int f1, f2, f3, f4;
		get_file_and_rank_from_bitboard(current_pawn, &f1, &f2);
		get_file_and_rank_from_bitboard(move, &f3, &f4);

		if (move) {
			if (color ? (current_pawn & RANK_2) : (current_pawn & RANK_7)) {
				generate_promotions(b, list, current_pawn, move, color);
			} else {
				add_move(b, list, current_pawn, move, EMPTY_SQUARE);

				if (color ? (current_pawn & RANK_7) : (current_pawn & RANK_2)) {
					move = validate_move(color ? move_south(move) : move_north(move), player_board | opponent_board);
					if (move) {
						add_move(b, list, current_pawn, move, EMPTY_SQUARE);
					}
				}
			}
		}

		Bitboard captures[] = {
		    color ? move_south_east(current_pawn) : move_north_east(current_pawn),
		    color ? move_south_west(current_pawn) : move_north_west(current_pawn)};

		for (int i = 0; i < 2; i++) {
			move = validate_capture(captures[i], opponent_board);
			attacks |= move;
			if (move) {
				if (color ? (current_pawn & RANK_2) : (current_pawn & RANK_7)) {
					generate_promotions(b, list, current_pawn, move, color);
				} else {
					add_move(b, list, current_pawn, move, EMPTY_SQUARE);
				}
			}
		}

		if ((current_pawn & (color ? RANK_5 : RANK_4)) && b->en_passant_square) {
			move = (color ? move_south_east(current_pawn) : move_north_east(current_pawn)) ^ (player_board | opponent_board);
			if (move == b->en_passant_square) {
				add_move(b, list, current_pawn, move, EMPTY_SQUARE);
				attacks |= move;
			}
			move = (color ? move_south_west(current_pawn) : move_north_west(current_pawn)) ^ (player_board | opponent_board);
			if (move == b->en_passant_square) {
				add_move(b, list, current_pawn, move, EMPTY_SQUARE);
				attacks |= move;
			}
		}
	}

	return attacks;
}

#define SLIDING_MOVES(func)                                       \
	move = current_piece;                                         \
	while (move) {                                                \
		move = validate_move(func(move), player_board);           \
		if (move) {                                               \
			add_move(b, list, current_piece, move, EMPTY_SQUARE); \
			attacks |= move;                                      \
		}                                                         \
		if (move & opponent_board) {                              \
			add_move(b, list, current_piece, move, EMPTY_SQUARE); \
			break;                                                \
		}                                                         \
	}

Bitboard generate_bishop_attacks(Board *b, MoveList *list, Byte color) {
	Bitboard bishops = color ? b->black.bishops : b->white.bishops;
	Bitboard player_board = color ? b->black_board : b->white_board;
	Bitboard opponent_board = color ? b->white_board : b->black_board;
	Bitboard attacks = 0ULL;

	while (bishops) {
		Bitboard current_piece = (1ULL << (ffsll(bishops) - 1));
		bishops &= ~current_piece;  // Remove the processed bishop

		Bitboard move = 0ULL;

		SLIDING_MOVES(move_north_east)
		SLIDING_MOVES(move_north_west)
		SLIDING_MOVES(move_south_east)
		SLIDING_MOVES(move_south_west)
	}

	return attacks;
}

Bitboard generate_rook_attacks(Board *b, MoveList *list, Byte color) {
	Bitboard rooks = color ? b->black.rooks : b->white.rooks;
	Bitboard player_board = color ? b->black_board : b->white_board;
	Bitboard opponent_board = color ? b->white_board : b->black_board;
	Bitboard attacks = 0ULL;

	while (rooks) {
		Bitboard current_piece = (1ULL << (ffsll(rooks) - 1));
		rooks &= ~current_piece;  // Remove the processed bishop

		Bitboard move = 0ULL;

		SLIDING_MOVES(move_north)
		SLIDING_MOVES(move_south)
		SLIDING_MOVES(move_east)
		SLIDING_MOVES(move_west)
	}

	return attacks;
}

Bitboard generate_queen_attacks(Board *b, MoveList *list, Byte color) {
	Bitboard queens = color ? b->black.queens : b->white.queens;
	Bitboard player_board = color ? b->black_board : b->white_board;
	Bitboard opponent_board = color ? b->white_board : b->black_board;
	Bitboard attacks = 0ULL;

	while (queens) {
		Bitboard current_piece = (1ULL << (ffsll(queens) - 1));
		queens &= ~current_piece;  // Remove the processed bishop

		Bitboard move = 0ULL;

		SLIDING_MOVES(move_north)
		SLIDING_MOVES(move_south)
		SLIDING_MOVES(move_east)
		SLIDING_MOVES(move_west)
		SLIDING_MOVES(move_north_east)
		SLIDING_MOVES(move_north_west)
		SLIDING_MOVES(move_south_east)
		SLIDING_MOVES(move_south_west)
	}

	return attacks;
}

Bitboard generate_knight_attacks(Board *b, MoveList *list, Byte color) {
	Bitboard knights = color ? b->black.knights : b->white.knights;
	Bitboard player_board = color ? b->black_board : b->white_board;

	const int offsets[] = {15, 17, 10, 6, -15, -17, -10, -6};
	Bitboard move = 0ULL, attacks = 0ULL;

	while (knights) {
		Bitboard current_piece = knights & -knights;
		knights &= knights - 1;

		for (int i = 0; i < 8; i++) {
			move = (offsets[i] > 0) ? (current_piece << offsets[i]) : (current_piece >> -offsets[i]);

			if ((offsets[i] == 15 || offsets[i] == -17) && (current_piece & A_FILE)) continue;
			if ((offsets[i] == 17 || offsets[i] == -15) && (current_piece & H_FILE)) continue;
			if ((offsets[i] == 10 || offsets[i] == -6) && (current_piece & (H_FILE | H_FILE >> 1))) continue;
			if ((offsets[i] == 6 || offsets[i] == -10) && (current_piece & (A_FILE | A_FILE << 1))) continue;

			move = validate_move(move, player_board);

			if (move) {
				add_move(b, list, current_piece, move, EMPTY_SQUARE);
				attacks |= move;
			}
		}
	}

	return attacks;
}

Bitboard generate_king_attacks(Board *b, MoveList *list, Byte color) {
	Bitboard king = color ? b->black.king : b->white.king;
	Bitboard player_board = color ? b->black_board : b->white_board;

	const int offsets[] = {1, 7, 8, 9, -1, -7, -8, -9};
	Bitboard move = 0ULL, attacks = 0ULL;

	for (int i = 0; i < 8; i++) {
		move = (offsets[i] > 0) ? (king << offsets[i]) : (king >> -offsets[i]);

		if ((offsets[i] == 1 || offsets[i] == 9 || offsets[i] == -7) && (king & H_FILE)) continue;
		if ((offsets[i] == -1 || offsets[i] == -9 || offsets[i] == 7) && (king & A_FILE)) continue;

		move = validate_move(move, player_board);

		if (move) {
			add_move(b, list, king, move, EMPTY_SQUARE);
			attacks |= move;
		}
	}

	Byte castle_rights = CASTLE_RIGHTS(color, b->castle_rights);
	if (castle_rights & KING_SIDE_CASTLE) {
		move = validate_castle(b, color, KING_SIDE_CASTLE);
		if(move) {
			add_move(b, list, king, move, EMPTY_SQUARE);
			// not sure whether to add this in attack table because 
			// i am not sure if this is "attack"
		}
	}
	if (castle_rights & QUEEN_SIDE_CASTLE) {
		move = validate_castle(b, color, QUEEN_SIDE_CASTLE);
		if(move) {
			add_move(b, list, king, move, EMPTY_SQUARE);
			// not sure whether to add this in attack table because 
			// i am not sure if this is "attack"
		}
	}
	return attacks;
}

void update_attacks_for_color(Board *b, Byte color) {
	Bitboard attacks = 0ULL;
	MoveList main_list, temp_list;
	init_movelist(&temp_list);
	init_movelist(&main_list);

	attacks |= b->lookup_tables[color][PAWN] = generate_pawn_attacks(b, &temp_list, color);
	append_list(&main_list, &temp_list);
	clear_movelist(&temp_list);

	attacks |= b->lookup_tables[color][KNIGHT] = generate_knight_attacks(b, &temp_list, color);
	append_list(&main_list, &temp_list);
	clear_movelist(&temp_list);

	attacks |= b->lookup_tables[color][BISHOP] = generate_bishop_attacks(b, &temp_list, color);
	append_list(&main_list, &temp_list);
	clear_movelist(&temp_list);

	attacks |= b->lookup_tables[color][ROOK] = generate_rook_attacks(b, &temp_list, color);
	append_list(&main_list, &temp_list);
	clear_movelist(&temp_list);

	attacks |= b->lookup_tables[color][QUEEN] = generate_queen_attacks(b, &temp_list, color);
	append_list(&main_list, &temp_list);
	clear_movelist(&temp_list);

	attacks |= b->lookup_tables[color][KING] = generate_king_attacks(b, &temp_list, color);
	append_list(&main_list, &temp_list);
	clear_movelist(&temp_list);

	b->lookup_tables[color][0] = attacks;
}

void update_attacks(Board *b, Byte color) {
	if (color) {
		update_attacks_for_color(b, WHITE);
		update_attacks_for_color(b, BLACK);
	} else {
		update_attacks_for_color(b, BLACK);
		update_attacks_for_color(b, WHITE);
	}
}