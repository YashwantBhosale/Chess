#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "bitboard.h"

Bitboard get_bitboard(int file, int rank) {
	if (rank < 0 || rank > 8 || file < 0 || file > 8) return 0;

	Bitboard bitboard = 1ULL;

	int index = (rank - 1) * 8 + file - 1;
	return (bitboard << index);
}


void get_file_and_rank_from_bitboard(Bitboard bitboard, int *file, int *rank) {
    if (bitboard == NULL_BITBOARD) {
        *file = 0;
        *rank = 0;
        return;
    }

    int index = __builtin_ctz(bitboard);

    *file = (index % 8) + 1;
    *rank = (index / 8) + 1;
}

Square get_square_from_bitboard(Bitboard bitboard) {
    int file=0, rank=0;

    get_file_and_rank_from_bitboard(bitboard, &file, &rank);
    return (Square) {
        .file = file,
        .rank = rank
    };
}


void print_squares_from_bitboard(Bitboard bitboard) {
    for (int i = 0; i < 64; i++) { // Iterate through all 64 bits
        if (bitboard & (1ULL << i)) { // Check if the bit is set
            int file = (i % 8) + 1;       // File from bit index
            int rank = (i / 8) + 1;       // Rank from bit index
            wprintf(L"%c%d ", 'a' + file - 1, rank); // Print in standard chess notation
        }
    }
    wprintf(L"\n");
}
