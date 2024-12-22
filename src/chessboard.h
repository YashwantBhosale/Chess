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
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

#define WHITE_CASTLE_RIGHTS 0b00000011 // 0b000000KQ
#define BLACK_CASTLE_RIGHTS 0b00110000 // 0b00kq0000

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
    bool turn;    
} Board;

void init_board(Board *b);
void init_pieces(Pieces *type);
void print_board(Board *b, Byte turn);
void load_fen(Board *b, const char *fen);

#endif