#include "move_types.h"
#include "transposition.h"

unsigned long long random_64() {
    unsigned long long r = 0;
    for (int i = 0; i < 4; i++) {
        r = (r << 16) | (rand() & 0xFFFF); // Combine 16 bits at a time
    }
    return r;
}

/*
    For Context,
    PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6

    in zobrist table,
    white pawn -> 0,    black pawn -> 6
    white knight -> 2,  black knight -> 7
    white bishop -> 3,  black bishop -> 8
    white rook -> 4,    black rook -> 9
    white queen -> 5,   black queen -> 10
    white king -> 6,    black king -> 11 

    we need to map the piece color and type to the index in the zobrist table.
    so the mapping is as follows:
    we have WHITE = 0, BLACK = 1
    so the index in the zobrist table is calculated as follows:

    index = (color * 6) + (type - 1)
*/


void init_zobrist(ZobristTable* z) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            z->keys[i][j] = random_64();
        }
    }
    z->white_to_move = random_64();
    for (int i = 0; i < 4; i++) {
        z->castling[i] = random_64();
    }
    for (int i = 0; i < 8; i++) {
        z->en_passant[i] = random_64();
    }

    memset(z->table, 0, sizeof(z->table));
}

void insert_entry(ZobristTable* z, Entry e) {
    unsigned long long index = e.key % TABLE_SIZE;
    z->table[index] = e;
}

Entry* get_entry(ZobristTable* z, unsigned long long key) {
    unsigned long long index = key % TABLE_SIZE;
    return &z->table[index];
}

unsigned piece_index(uint8_t piece) {
    return (piece_color(piece) * 6) + (piece_type(piece) - 1);
}

unsigned long long get_zobrist_key(board* b, ZobristTable* z) {
    unsigned long long key = 0;
    
    for(int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if(b->square_table[i][j]){
                uint8_t piece = b->square_table[i][j];
                key ^= z->keys[piece_index(piece)][i * 8 + j];
            }
        }
    }

    /*
        we may hash more stuff as well into the zobrist key.
        1. turn
        2. castling rights
        3. en passant square
    */

    return key;
}


