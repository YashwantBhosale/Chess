#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>

typedef unsigned long long Bitboard;
typedef uint8_t Byte;
typedef struct square {
    Byte file;
    Byte rank;
} Square;

#define NULL_BITBOARD 0ULL 

void print_squares_from_bitboard(Bitboard bitboard);
void get_file_and_rank_from_bitboard(Bitboard bitboard, int *file, int *rank);
Bitboard get_bitboard(int file, int rank);
Square get_square_from_bitboard(Bitboard bitboard);
#endif