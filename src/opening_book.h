#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include "move_types.h"
#include "transposition.h"

typedef struct {
    ZobristTable OpeningBookTable;    
    int num_entries;
} OpeningBook;

void init_opening_book(OpeningBook* book);
unsigned long load_opening_book(OpeningBook* book, char* filename);
Move get_book_move(OpeningBook* book, board* b, short turn);

extern OpeningBook opening_book;
#endif