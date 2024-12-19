#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "opening_book.h"

void init_opening_book(OpeningBook* book) {
	init_zobrist(&book->OpeningBookTable);
	book->num_entries = 0;
}

short status_from_char(char c, short turn) {
	switch (c) {
		case 'q':
			return turn == WHITE ? WHITE_PROMOTES_TO_QUEEN : BLACK_PROMOTES_TO_QUEEN;
		case 'r':
			return turn == WHITE ? WHITE_PROMOTES_TO_ROOK : BLACK_PROMOTES_TO_ROOK;
		case 'b':
			return turn == WHITE ? WHITE_PROMOTES_TO_BISHOP : BLACK_PROMOTES_TO_BISHOP;
		case 'n':
			return turn == WHITE ? WHITE_PROMOTES_TO_KNIGHT : BLACK_PROMOTES_TO_KNIGHT;
		default:
			break;
	}
	return 0;
}

Move move_from_string(char* move, short turn) {
	Move m;
	m.src = (square){.file = move[0] - 'a' + 1, .rank = move[1] - '0'};
	m.dest = (square){.file = move[2] - 'a' + 1, .rank = move[3] - '0'};
	m.type = status_from_char(move[4], turn);
	return m;
}

void readline(char* line, FILE* file) {
	char c;
	int i = 0;
	while ((c = fgetc(file)) != '\n') {
		line[i++] = c;
	}
	line[i] = '\0';
}

unsigned long load_opening_book(OpeningBook* book, char* filename) {
    wprintf(L"Loading opening book from %s....\n", filename);


	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		wprintf(L"Error opening file\n");
		exit(1);
	}

	char line[256];
    unsigned long malinformed_lines = 0;
	while (fgets(line, sizeof(line), file)) {
		board b;
		char fen[256];
		char move[6], t;
		short turn;

		/*
		    %255[^,] -> read up to 255 characters until a comma is found
		    %hd -> read a short integer
		    %5s -> read up to 5 characters
		*/
		if (sscanf(line, "%255[^,],%c,%6[^\n]", fen, &t, move) != 3) {
			wprintf(L"Malformed line: %s, %s, %c, %s\n", line, fen, t, move);
			malinformed_lines++;
    
			continue;
		}

		load_fen(&b, fen);
        turn = t=='w' ? WHITE : BLACK;

		Entry e;
		e.key = get_zobrist_key(&b, &book->OpeningBookTable, turn);
		e.best_move = move_from_string(move, turn);
		e.is_book_move = true;

		insert_entry(&book->OpeningBookTable, e);
		book->num_entries++;
	}

	fclose(file);
    return malinformed_lines;
}

Move get_book_move(OpeningBook* book, board* b, short turn) {
	unsigned long long key = get_zobrist_key(b, &book->OpeningBookTable, turn);

	Entry* e = get_entry(&book->OpeningBookTable, key);
	if (e && e->is_book_move) {
		return e->best_move;
	}
	return (Move){0};
}