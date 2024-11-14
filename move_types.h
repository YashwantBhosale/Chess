#ifndef MOVE_TYPES_H
#define MOVE_TYPES_H

#include <stdint.h>
#include "chessboard.h"


typedef struct {
	square src;
	square dest;
	uint8_t piece;

	uint8_t captured_piece;
	uint8_t promoted_piece;
	uint64_t en_passant_square;

	uint8_t castle_rights;
	uint8_t type;
} Move;

#define NORMAL_MOVE 0
#define CAPTURE_MOVE 1
#define CASTLE_MOVE 2
#define EN_PASSANT_MOVE 3
#define WHITE_PROMOTION_MOVE 4 // 0000 0100
#define BLACK_PROMOTION_MOVE 12 // 0000 1100

#define WHITE_PROMOTES_TO_KNIGHT 0b00010100
#define WHITE_PROMOTES_TO_BISHOP 0b00100100
#define WHITE_PROMOTES_TO_ROOK 0b00110100
#define WHITE_PROMOTES_TO_QUEEN 0b01000100

#define BLACK_PROMOTES_TO_KNIGHT 0b00011100
#define BLACK_PROMOTES_TO_BISHOP 0b00101100
#define BLACK_PROMOTES_TO_ROOK 0b00111100
#define BLACK_PROMOTES_TO_QUEEN 0b01001100

#define PROMOTION_MOVE_MASK 0b00001111

#define CHECK_MOVE 5
#define CHECKMATE_MOVE 6
#define STALEMATE_MOVE 7
#define INVALID_MOVE -1

#endif