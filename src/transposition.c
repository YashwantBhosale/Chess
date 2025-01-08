#include "move_types.h"
#include "transposition.h"

unsigned long long random_64() {
	unsigned long long r = 0;
	for (int i = 0; i < 4; i++) {
		r = (r << 16) | (rand() & 0xFFFF);  // Combine 16 bits at a time
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

static bool is_power_of_2(unsigned long long x) {
	return x && !(x & (x - 1));
}

void init_zobrist(ZobristTable* z) {
	if (!is_power_of_2(TABLE_SIZE)) {
		fprintf(stderr, "Error: TABLE_SIZE must be power of 2\n");
		exit(1);
	}

	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 64; j++) {
			z->keys[i][j] = random_64();
		}
	}
	z->white_to_move = random_64();
	for (int i = 0; i < 4; i++) {
		z->castling[i] = random_64();
	}
	z->en_passant = random_64();

	for (size_t i = 0; i < TABLE_SIZE; i++) {
		z->table[i].key = 0; 
		z->table[i].depth = 0;
	}

    z->num_entries = 0;
}

void insert_entry(ZobristTable* z, Entry e) {
	unsigned long long mask = TABLE_SIZE - 1;
	unsigned long long initial_index = e.key & mask;
	unsigned long long index = initial_index;
	int i = 1;

	while (true) {
		if (z->table[index].key == 0 || z->table[index].key == e.key) {
			z->table[index] = e;
            z->num_entries++;
			return;
		}

		if (z->table[index].depth <= e.depth) {
			z->table[index] = e;
			return;
		}

		index = (initial_index + (i * i)) & mask;
		i++;

		if (i >= 100) {
			z->table[index] = e;
			return;
		}
	}
}

Entry* get_entry(ZobristTable* z, unsigned long long key) {
	unsigned long long mask = TABLE_SIZE - 1;
	unsigned long long initial_index = key & mask;
	unsigned long long index = initial_index;
	int i = 1;

	while (z->table[index].key != 0) {
		if (z->table[index].key == key) {
			return &z->table[index];
		}

		index = (initial_index + (i * i)) & mask;
		i++;

		if (i >= 100) {
			return NULL;
		}
	}

	return NULL;
}

unsigned piece_index(uint8_t piece) {
	return (piece_color(piece) * 6) + (piece_type(piece) - 1);
}

unsigned long long get_zobrist_key(board* b, ZobristTable* z, short turn) {
	unsigned long long key = 0;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (b->square_table[i][j]) {
				uint8_t piece = b->square_table[i][j];
				key ^= z->keys[piece_index(piece)][i * 8 + j];
			}
		}
	}

	if (turn == WHITE) {
		key ^= z->white_to_move;
	}

	if (b->castle_rights & WHITE_KING_SIDE_CASTLE_RIGHTS) {
		key ^= z->castling[0];
	}

	if (b->castle_rights & WHITE_QUEEN_SIDE_CASTLE_RIGHTS) {
		key ^= z->castling[1];
	}

	if (b->castle_rights & BLACK_KING_SIDE_CASTLE_RIGHTS) {
		key ^= z->castling[2];
	}

	if (b->castle_rights & BLACK_QUEEN_SIDE_CASTLE_RIGHTS) {
		key ^= z->castling[3];
	}

	if (b->en_passant_square) {
		key ^= z->en_passant;
	}

	return key;
}
