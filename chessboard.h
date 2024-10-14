#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

enum colors {
    WHITE = 0,
    BLACK = 1
};



#define EMPTY_SQUARE 0
// White pieces
#define WHITE_PAWN_1 1
#define WHITE_PAWN_2 17
#define WHITE_PAWN_3 33
#define WHITE_PAWN_4 49
#define WHITE_PAWN_5 65
#define WHITE_PAWN_6 81
#define WHITE_PAWN_7 97
#define WHITE_PAWN_8 113

#define WHITE_KNIGHT_1 2
#define WHITE_KNIGHT_2 18

#define WHITE_BISHOP_1 3
#define WHITE_BISHOP_2 19

#define WHITE_ROOK_1 4
#define WHITE_ROOK_2 20

#define WHITE_QUEEN 5
#define WHITE_KING 6

// Black pieces
#define BLACK_PAWN_1 9
#define BLACK_PAWN_2 25
#define BLACK_PAWN_3 41
#define BLACK_PAWN_4 57
#define BLACK_PAWN_5 73
#define BLACK_PAWN_6 89
#define BLACK_PAWN_7 105
#define BLACK_PAWN_8 121

#define BLACK_KNIGHT_1 10
#define BLACK_KNIGHT_2 26

#define BLACK_BISHOP_1 11
#define BLACK_BISHOP_2 27

#define BLACK_ROOK_1 12
#define BLACK_ROOK_2 28

#define BLACK_QUEEN 13
#define BLACK_KING 14

typedef struct square {
    uint8_t file;
    uint8_t rank;
} square;

typedef struct pieces {
    uint64_t *pawns; // array of 8 pawns
    uint64_t *knights; // array of 2 knights
    uint64_t *bishops; // array of 2 bishops
    uint64_t *rooks; // array of 2 rooks
    uint64_t *queen; // array of 1 queen
    uint64_t king; // 1 king
} pieces;

typedef struct {
    pieces *white, *black;
    uint8_t square_table[8][8];

    uint64_t attack_tables[2];
} board;

/* Global helper functions */
void rank_and_file_from_bitboard(uint64_t bitboard, int *file, int *rank);
square get_square(uint64_t bitboard);

/* chess board functions*/
void init_board(board *board);
void print_board(board *board, short turn);
uint64_t get_bitboard(int file, int rank);
void rank_and_file_from_bitboard(uint64_t bitboard, int *file, int *rank);
uint64_t white_board(board *b);
uint64_t black_board(board *b);
void print_legal_moves(uint64_t legal_moves);

/* square table functions */
void init_square_table(board *board);
void update_square_table(int file, int rank, uint8_t piece, board *b) ;
void view_square_table(board *b);
