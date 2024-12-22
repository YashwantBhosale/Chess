#include <stdlib.h>
#include "bitboard.h"
#include "chessboard.h"
#include "move_gen.h"

enum Directions { DIAGONAL, NON_DIAGONAL };

static inline Bitboard move_north(Bitboard position) {
	return position << 8;
}

static inline Bitboard move_south(Bitboard position) {
	return position >> 8;
}

static inline Bitboard move_east(Bitboard position) {
	return (position ^ H_FILE) << 1;
}

static inline Bitboard move_west(Bitboard position) {
	return (position ^ A_FILE) >> 1;
}

static inline Bitboard move_north_east(Bitboard position) {
	return (position ^ H_FILE) << 9;
}

static inline Bitboard move_north_west(Bitboard position) {
	return (position ^ A_FILE) >> 7;
}

static inline Bitboard move_south_east(Bitboard position) {
	return (position ^ H_FILE) >> 7;
}

static inline Bitboard move_south_west(Bitboard position) {
	return (position ^ A_FILE) << 9;
}

static inline Bitboard validate_capture(Bitboard move, Bitboard opponent_pieces) {
	return move & opponent_pieces;
}

static inline Bitboard validate_move(Bitboard move, Bitboard friendly_pieces) {
	return move ^ friendly_pieces;
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

    if(is_knight_check(b, src, color)) 
        return true;
    
    if(is_pawn_check(b, src, color))
        return true;

	return false;
}

MoveList generate_pawn_attacks(Board *b, Byte color) {
    Bitboard pawns = color ? b->black.pawns : b->white.pawns;

	Bitboard player_board = color ? b->black_board : b->white_board;
	Bitboard opponent_board = color ? b->white_board : b->black_board;

	switch (color) {
		case WHITE:
            for(int i = 0;i < 8; i++) {
                Bitboard current_pawn = 1ULL << __builtin_ctz(pawns);

                Bitboard move = validate_move(move_north(current_pawn), player_board | opponent_board);
                if(move) {
                    // add move to list
                }

                if(move && current_pawn & RANK_2){
                    move = validate_move(move_north(current_pawn), player_board | opponent_board);
                    if(move) {
                        // add move to list
                    }
                }

                move = validate_capture(move_north_east(current_pawn), opponent_board);
                if(move) {
                    // add move to list
                }

                move = validate_capture(move_north_west(current_pawn), opponent_board);
                if(move) {
                    // add move to list
                }

                if(current_pawn & RANK_5 && b->en_passant_square) {
                    move = move_north_east(current_pawn) ^ (player_board | opponent_board);
                    if (move == b->en_passant_square) {
                        // add move to list
                    }

                    move = move_north_west(current_pawn) ^ (player_board | opponent_board);
                    if(move == b->en_passant_square) {
                        // add move to list
                    }
                }

            }
			break;
		case BLACK:

			break;
	}
}