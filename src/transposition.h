#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
// #include "chessboard.h"
// #include "move_types.h"
typedef struct {
    unsigned long long key;
    unsigned depth;
    double evaluation;
    Move best_move;
    double alpha;
    double beta;
    bool is_book_move;
} Entry;


/*
    Zobrist keys:
    It is a number that represents the state of the board.
    It is used to store the state of the board in the transposition table. we can retrieve the state
    of the board from the transposition table using the zobrist key.

    1. 12 pieces will have unique keys for each square.
*/

#define TABLE_SIZE 10000000
typedef struct zobrist {
    unsigned long long keys[12][64];
    unsigned long long white_to_move;
    unsigned long long castling[4];
    unsigned long long en_passant;
    Entry table[TABLE_SIZE];
} ZobristTable;

extern ZobristTable transposition_table;

unsigned long long random_64();
void init_zobrist(ZobristTable* z);
void insert_entry(ZobristTable* z, Entry e);
unsigned long long get_zobrist_key(board* b, ZobristTable* z, short turn);
Entry* get_entry(ZobristTable* z, unsigned long long key);
#endif