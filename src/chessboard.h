#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <stdint.h>
#include <stdbool.h>

#include "bitboard.h"
#include "move_types.h"
#include "move_list.h"

#define WHITE 0
#define BLACK 1

enum Files {
	A = 1,
	B = 2,
	C = 3,
	D = 4,
	E = 5,
	F = 6,
	G = 7,
	H = 8
};

enum Pieces {
    EMPTY_SQUARE = 0,
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

#define WHITE_PAWN 1
#define WHITE_KNIGHT 2
#define WHITE_BISHOP 3
#define WHITE_ROOK 4
#define WHITE_QUEEN 5
#define WHITE_KING 6

#define BLACK_PAWN 9
#define BLACK_KNIGHT 10
#define BLACK_BISHOP 11
#define BLACK_ROOK 12
#define BLACK_QUEEN 13
#define BLACK_KING 14

#define PIECE_CLR(P) (P & 8) ? (BLACK) : (WHITE)
#define PIECE_TYPE(P) (P & 7)

#define WHITE_CASTLE_RIGHTS 0b00000011 // 0b000000KQ
#define BLACK_CASTLE_RIGHTS 0b00110000 // 0b00kq0000
#define KING_SIDE_CASTLE 2
#define QUEEN_SIDE_CASTLE 1

#define CASTLE_RIGHTS(C, R) C ? ((R & BLACK_CASTLE_RIGHTS) >> 4) : ((R & WHITE_CASTLE_RIGHTS))

#define A_FILE 0x0101010101010101ULL
#define B_FILE 0x0202020202020202ULL
#define C_FILE 0x0404040404040404ULL
#define D_FILE 0x0808080808080808ULL
#define E_FILE 0x1010101010101010ULL
#define F_FILE 0x2020202020202020ULL
#define G_FILE 0x4040404040404040ULL
#define H_FILE 0x8080808080808080ULL

#define RANK_1 0x00000000000000FFULL
#define RANK_2 0x000000000000FF00ULL
#define RANK_3 0x0000000000FF0000ULL
#define RANK_4 0x00000000FF000000ULL
#define RANK_5 0x000000FF00000000ULL
#define RANK_6 0x0000FF0000000000ULL
#define RANK_7 0x00FF000000000000ULL
#define RANK_8 0xFF00000000000000ULL


typedef struct {
	short pawns, knights, bishops, rooks, queens;
} PieceCount;

typedef struct pieces {
    Bitboard pawns;
    Bitboard knights;
    Bitboard bishops;
    Bitboard rooks;
    Bitboard queens;
    Bitboard king;
    PieceCount count;
} Pieces;

typedef struct {
    Pieces white, black;
    Byte square_table[8][8];
    Byte castle_rights;
    Byte captured_pieces[2][16];
    Byte captured_pieces_count[2];
    Bitboard en_passant_square;
    Bitboard white_board;
    Bitboard black_board;    
    MoveList moves;

    Bitboard lookup_tables[2][7];
    bool turn;    
} Board;

void init_board(Board *b);
void init_pieces(Pieces *type);
void print_board(Board *b, Byte turn);
void load_fen(Board *b, const char *fen);

#endif