#ifndef MOVE_TYPES_H
#define MOVE_TYPES_H

#include "bitboard.h"

/*
    Old move struct:
    typedef struct {
        square src;
        square dest;
        uint8_t piece;

        uint8_t captured_piece;
        uint8_t promoted_piece;
        uint64_t en_passant_square;

        uint8_t castle_rights;
        uint8_t type;

        bool is_check;
        unsigned int score;
    } Move;

    Move types:
    1. normal move
    2. capture move
    3. en passant move
    4. castle move
    5. check move
    6. promotion move
    7. invalid move

    Plan is to encode this information in 32-bit number

    unsigned int move;
    #fff EEEE EEpp pCC CPPP TTTT TTFF FFFF

    very complicated but here is what does this mean
    FFFFFF -> this is supposed to indicate the "from" square
    TTTTTT -> this is supposed to indicate the "to" square

    I haven't decided if i am going to store index (out of 64) of
    the square or the FFFRRR format which is File and rank of the
    square (for now we may assume it is index/64)

    PPP -> denotes the piece that is moved
    CCC -> denotes the piece that is captured
    ppp -> denotes the promoted piece

    EEEEEE -> denotes the en passant square

    fff -> denotes the type of the move
    # -> denotes if it was a checkmate move (not sure if this is 
    needed)
*/

typedef struct {
    unsigned int move;
    unsigned int score;
} Move;

// Not sure if this is needed
#define NORMAL_MOVE 0
#define CAPTURE_MOVE 1
#define EN_PASSANT_MOVE 2
#define CASTLE_MOVE 3
#define CHECK_MOVE 4
#define PROMOTION_MOVE 5
#define INVALID_MOVE 6

#define FROMSQ(M) (M & 0b00000000000000000000000000111111)
#define TOSQ(M) ((M & 0b00000000000000000000111111000000)>>6)
#define PIECE(M) ((M & 0b00000000000000000111000000000000)>>12)
#define CAPTURED_PIECE(M) ((M & 0b00000000000000111000000000000000)>>15)
#define PROMOTED_PIECE(M) ((M & 0b00000000000111000000000000000000)>>18)
#define EN_PASSANT_SQ(M) ((M & 0b00001111110000000000000000000000)>>21)
#define TYPE(M) ((M & 0b01110000000000000000000000000000)>>27)

#endif