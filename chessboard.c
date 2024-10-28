#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include "chessboard.h"

/* Utility Functions / Helper functions */
// piece color
uint8_t piece_color(uint8_t piece_id) {
    return piece_id & 8 ? BLACK : WHITE;
}

// piece type
uint8_t piece_type(uint8_t piece_id) {
    return piece_id & 7;
}

// piece number
uint8_t piece_number(uint8_t piece_id) {
    return (piece_id & 112) >> 4;
}


// function to get bitboard from rank and file
uint64_t get_bitboard(uint8_t file, uint8_t rank){
    // Check if the square is valid
    if(rank < 0 || rank > 8 || file < 0 || file > 8) return 0;

    // Create a bitboard with the bit set at the index of the square
    uint64_t bitboard = 1ULL;
    
    // calculate the no. of times bit needs to be shifted
    int index = (rank-1) * 8 + file - 1;

    // return the bitboard with the bit set at the appropriate postion of the square
    return (bitboard << index);
}

// function to get rank and file from bitboard
void get_rank_and_file_from_bitboard(uint64_t bitboard, int *file, int *rank) {
	if (!bitboard) {
		*file = 0;
		*rank = 0;
		return;
	}
	int bit_no = 0;

	while (bitboard) {
		bit_no++;
		bitboard >>= 1;
	}

	bit_no--;

	*file = (bit_no % 8) + 1;
	*rank = (bit_no / 8) + 1;
}


// function to get square from bitboard
square get_square_from_bitboard(uint64_t bitboard) {
    square s;
    int file, rank;

    // get the rank and file from the bitboard
    get_rank_and_file_from_bitboard(bitboard, &file, &rank);
    
    // Pach the rank and file to the square structure
    s.file = file;
    s.rank = rank;
    return s;
}

/* function to get compiled postion of white or black pieces*/
uint64_t get_type_board(pieces *type, board *b) {
    uint64_t type_board = 0ULL;

    for (int i = 0; i < type->count.pawns; i++) {
        type_board |= type->pawns[i];
    }

    for (int i = 0; i < type->count.knights; i++) {
        type_board |= type->knights[i];
    }

    for (int i = 0; i < type->count.bishops; i++) {
        type_board |= type->bishops[i];
    }

    for (int i = 0; i < type->count.rooks; i++) {
        type_board |= type->rooks[i];
    }

    for(int i = 0; i < type->count.queens; i++) {
        type_board |= type->queen[i];
    }

    type_board |= type->king;
    return type_board;
}

uint64_t white_board(board *b) {
    return get_type_board(b->white, b);
}

uint64_t black_board(board *b) {
    return get_type_board(b->black, b);
}

/* square table functions */
void update_square_table(int file, int rank, uint8_t piece, board *b) {
    b->square_table[file-1][rank-1] = piece;
    return;
}


/* Initialization functions */

// function to initialize the pieces: allocate memory
void init_pieces(pieces *type) {
    // Memory is allocated dynamically to implement the pawn promotion feature by using realloc function
    type->pawns = (uint64_t *)malloc(sizeof(uint64_t) * 8);
    type->knights = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    type->bishops = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    type->rooks = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    type->queen = (uint64_t *)malloc(sizeof(uint64_t));
    return;
}

// function to set initial piece counts
void set_piece_counts(board *b) {
    b->white->count.pawns = 8;
    b->white->count.knights = 2;
    b->white->count.bishops = 2;
    b->white->count.rooks = 2;
    b->white->count.queens = 1;

    b->black->count.pawns = 8;
    b->black->count.knights = 2;
    b->black->count.bishops = 2;
    b->black->count.rooks = 2;
    b->black->count.queens = 1;
    return;
}

// function to initialize the square table
void init_square_table(board *b) {
    for(int i=0; i < 8; i++){
        for(int j=0; j < 8; j++){
            b->square_table[i][j] = EMPTY_SQUARE;
        }
    }
    return;
}

void place_piece(uint64_t *piece_board, int file, int rank, uint8_t piece, board *b) {
    *piece_board = get_bitboard(file, rank);
    update_square_table(file, rank, piece, b);
    return;
}


// Function to set the initial postitions of pieces in the chessboard 
void set_pieces(board *b) {
    const uint8_t white_pawns[] ={ WHITE_PAWN_1, WHITE_PAWN_2, WHITE_PAWN_3, WHITE_PAWN_4, WHITE_PAWN_5, WHITE_PAWN_6, WHITE_PAWN_7, WHITE_PAWN_8 };
    for(int i=0; i < 8; i++) {
        place_piece(b->white->pawns + i, i+1, 2, white_pawns[i], b); // Note: here b->white->pawns + i is an address. !!!
    }

    const uint8_t black_pawns[] = { BLACK_PAWN_1, BLACK_PAWN_2, BLACK_PAWN_3, BLACK_PAWN_4, BLACK_PAWN_5, BLACK_PAWN_6, BLACK_PAWN_7, BLACK_PAWN_8 };
    for(int i=0; i < 8; i++) {
        place_piece(b->black->pawns + i, i+1, 7, black_pawns[i], b);
    }

    place_piece(b->white->rooks, A, 1, WHITE_ROOK_1, b);
    place_piece(b->white->rooks + 1, H, 1, WHITE_ROOK_2, b);

    place_piece(b->black->rooks, A, 8, BLACK_ROOK_1, b);
    place_piece(b->black->rooks + 1, H, 8, BLACK_ROOK_2, b);

    place_piece(b->white->knights, B, 1, WHITE_KNIGHT_1, b);
    place_piece(b->white->knights + 1, G, 1, WHITE_KNIGHT_2, b);

    place_piece(b->black->knights, B, 8, BLACK_KNIGHT_1, b);
    place_piece(b->black->knights + 1, G, 8, BLACK_KNIGHT_2, b);

    place_piece(b->white->bishops, C, 1, WHITE_BISHOP_1, b);
    place_piece(b->white->bishops + 1, F, 1, WHITE_BISHOP_2, b);

    place_piece(b->black->bishops, C, 8, BLACK_BISHOP_1, b);
    place_piece(b->black->bishops + 1, F, 8, BLACK_BISHOP_2, b);

    place_piece(b->white->queen, D, 1, WHITE_QUEEN, b);
    place_piece(&b->white->king, E, 1, WHITE_KING, b);

    place_piece(b->black->queen, D, 8, BLACK_QUEEN, b);
    place_piece(&b->black->king, E, 8, BLACK_KING, b);
    return;
}

// function to initialize the board
void init_board(board *board) {
     if(!board) return;

    board->black = (pieces *)malloc(sizeof(pieces));
    board->white = (pieces *)malloc(sizeof(pieces));

    init_pieces(board->white); /* Allocate memory for white pieces */
    init_pieces(board->black); /* Allocate memory for black pieces */
    set_piece_counts(board); /* Set initial piece counts */

    // Initialize the square table
    init_square_table(board);
    set_pieces(board); /* Set initial positions of pieces */

    board->attack_tables[WHITE] = 4294901760; // initial attack table for white hardcoded
	board->attack_tables[BLACK] = 281470681743360;// initial attack table for black hardcoded
    board->castle_rights = 0b01110111;
    return;
}


// Chessboard functions

// Function to print the chessboard
void print_board(board *b, short turn) {
    // wchar_t white_pieces[] = {L'K', L'Q', L'R', L'B', L'N', L'P'};
	// wchar_t black_pieces[] = {L'k', L'q', L'r', L'b', L'n', L'p'};
	wchar_t black_pieces[] = {L'♙', L'♘', L'♗', L'♖', L'♕', L'♔',};
	wchar_t white_pieces[] = {L'♟', L'♞', L'♝', L'♜', L'♛', L'♚',};

    /*
    WHY & 7? : The piece is stored in the square table as a 8-bit number. The lower 3 bits represent the piece type.
    */
    uint8_t piece, start_rank, end_rank, start_file, end_file;

    /*
    Note: Context of start and end are from printing pov
    for e.g. for white side, A1 might be the start and H8 might be the end but A8 wil be printed first and H1 will be printed last
    */
    switch (turn){
        case WHITE:{
            start_file = A;
            start_rank = 8;
            end_file = H;
            end_rank = 1;
            break;
        }
        case BLACK:{
            start_file = H;
            start_rank = 1;
            end_file = A;
            end_rank = 8;
            break;
        }

        default:
            return;
    }
    wprintf(L"\n\n\n");
    for(int rank = start_rank; turn == WHITE ? rank>=end_rank : rank <= end_rank  ; turn == WHITE ? rank-- : rank++){
        wprintf(L"\t\t+---+---+---+---+---+---+---+---+\n");
        wprintf(L"\t\t|");
        for(int file = start_file; turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--){
            piece = b->square_table[file-1][rank-1];
            /* If piece is present on the square then print the piece*/
            if(piece)
                wprintf(L" %lc |", piece & 8 ? black_pieces[(piece & 7)-1] : white_pieces[(piece & 7)-1]);

            /* If square is empty then denote it by printing _ */
            else
                wprintf(L"   |");
            
        }
        /* Print rank after printing each line */
		wprintf(L" %d\n", rank);
    }

    /* Print files to guide the user */
    wprintf(L"\t\t+---+---+---+---+---+---+---+---+\n\t\t");
    wprintf(L" ");
	char _files[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

    for(int file = start_file;turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--){
        wprintf(L" %c  ", _files[file-1]);
    }
    wprintf(L"\n");
    return;
}

void print_square_from_bitboard(uint64_t bitboard) {
    int file, rank;
    get_rank_and_file_from_bitboard(bitboard, &file, &rank);
    wprintf(L"%c%d", 'A' + file - 1, rank);
    return;
}

void print_moves(uint64_t moves) {
    uint64_t bitboard = 1ULL;
    for(int i=0; i<64; i++) {
        if(moves & bitboard) {
            print_square_from_bitboard(bitboard);
            wprintf(L", ");
        }
        bitboard <<= 1;
    }
    wprintf(L"\n");
    return;
}

// PENDING: function to free the memory allocated for the board
