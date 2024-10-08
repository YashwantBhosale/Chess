#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include "chessboard.h"

/* Uitility functions*/
/* This function returns the bitboard taking square (rank and file) as an argument */
uint64_t get_bitboard(int file, int rank){
    // Check if the square is valid
    if(rank < 0 || rank > 7 || file < 0 || file > 7) return 0;
    // Create a bitboard with the bit set at the index of the square
    uint64_t bitboard = 1ULL;
    
    // calculate the no. of times bit needs to be shifted
    int index = (rank-1) * 8 + file - 1;

    // return the bitboard with the bit set at the appropriate postion of the square
    return (bitboard << index);
}


/* square table functions */
void update_square_table(int file, int rank, uint8_t piece, board *b) {
    b->square_table[file-1][rank-1] = piece;
    return;
}


/* chessboard functions */
/* Function to set the initial postitions of pieces in the chessboard */
void set_pieces(board *b) {
    // Set initial positions for pawns
    uint8_t cursor_1 = WHITE_PAWN_1, cursor_2 = BLACK_PAWN_1;
    for(int i=1; i <= 8; i++){
        b->white->pawns[i-1] = get_bitboard(i, 2);
        update_square_table(i, 2, cursor_1, b);

        b->black->pawns[i-1] = get_bitboard(i, 7);
        update_square_table(i, 7, cursor_2, b);

        cursor_1+=16;
        cursor_2+=16;
    }

    // Set initial positions for knights
    b->white->knights[0] = get_bitboard(B, 1);
    update_square_table(B, 1, WHITE_KNIGHT_1, b);

    b->white->knights[1] = get_bitboard(G, 1);
    update_square_table(G, 1, WHITE_KNIGHT_2, b);

    b->black->knights[0] = get_bitboard(B, 8);
    update_square_table(B, 8, BLACK_KNIGHT_1, b);

    b->black->knights[1] = get_bitboard(G, 8);
    update_square_table(G, 8, BLACK_KNIGHT_2, b);

    // Set initial positions for bishops
    b->white->bishops[0] = get_bitboard(C, 1);
    update_square_table(C, 1, WHITE_BISHOP_1, b);

    b->white->bishops[1] = get_bitboard(F, 1);
    update_square_table(F, 1, WHITE_BISHOP_2, b);

    b->black->bishops[0] = get_bitboard(C, 8);
    update_square_table(C, 8, BLACK_BISHOP_1, b);

    b->black->bishops[1] = get_bitboard(F, 8);
    update_square_table(F, 8, BLACK_BISHOP_2, b);

    // Set initial positions for rooks
    b->white->rooks[0] = get_bitboard(A, 1);
    update_square_table(A, 1, WHITE_ROOK_1, b);

    b->white->rooks[1] = get_bitboard(H, 1);
    update_square_table(H, 1, WHITE_ROOK_2, b);

    b->black->rooks[0] = get_bitboard(A, 8);
    update_square_table(A, 8, BLACK_ROOK_1, b);

    b->black->rooks[1] = get_bitboard(H, 8);
    update_square_table(H, 8, BLACK_ROOK_2, b);

    // Set initial positions for queens
    b->white->queen[0] = get_bitboard(D, 1);
    update_square_table(D, 1, WHITE_QUEEN, b);

    b->black->queen[0] = get_bitboard(D, 8);
    update_square_table(D, 8, BLACK_QUEEN, b);

    // Set initial positions for kings
    b->white->king = get_bitboard(E, 1);
    update_square_table(E, 1, WHITE_KING, b);

    b->black->king = get_bitboard(E, 8);
    update_square_table(E, 8, BLACK_KING, b);
    return;
}

/* Function to print the chessboard */
void print_board(board *b, short turn) {
    // wchar_t white_pieces[] = {L'K', L'Q', L'R', L'B', L'N', L'P'};
	// wchar_t black_pieces[] = {L'k', L'q', L'r', L'b', L'n', L'p'};
	wchar_t white_pieces[] = {L'♙', L'♘', L'♗', L'♖', L'♕', L'♔',};
	wchar_t black_pieces[] = {L'♟', L'♞', L'♝', L'♜', L'♛', L'♚',};

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

    for(int rank = start_rank; turn == WHITE ? rank>=end_rank : rank <= end_rank  ; turn == WHITE ? rank-- : rank++){
        wprintf(L"+---+---+---+---+---+---+---+---+\n|");
        for(int file = start_file; turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--){
            piece = b->square_table[file-1][rank-1];
    
            /* If piece is present on the square then print the piece*/
            if(piece)
                wprintf(L" %lc |", piece & 8 ? black_pieces[(piece & 7)-1] : white_pieces[(piece & 7)-1]); //  

            /* If square is empty then denote it by printing _ */
            else
                wprintf(L" _ |");
            
        }
        /* Print rank after printing each line */
		wprintf(L" %d\n", rank);
    }

    /* Print files to guide the user */
    wprintf(L"+---+---+---+---+---+---+---+---+\n");
    wprintf(L" ");
	char _files[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

    for(int file = start_file; file <= end_file; turn == WHITE ? file++ : file--){
        wprintf(L" %c  ", _files[file-1]);
    }
    wprintf(L"\n");
    return;
}


/* Initialisation functions */
void init_pieces(pieces *type) {
    // Memory is allocated dynamically to implement the pawn promotion feature by using realloc function
    type->pawns = (uint64_t *)malloc(sizeof(uint64_t) * 8);
    type->knights = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    type->bishops = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    type->rooks = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    type->queen = (uint64_t *)malloc(sizeof(uint64_t));
}

void init_square_table(board *b) {
    for(int i=0; i < 8; i++){
        for(int j=0; j < 8; j++){
            b->square_table[i][j] = EMPTY_SQUARE;
        }
    }
}

void init_board(board *board) {
    if(!board) return;

    board->black = (pieces *)malloc(sizeof(pieces));
    board->white = (pieces *)malloc(sizeof(pieces));

    init_pieces(board->white); /* Allocate memory for white pieces */
    init_pieces(board->black); /* Allocate memory for black pieces */

    init_square_table(board); /* Set initial positions for pieces in the square table */    
    set_pieces(board); /* Set initial positions for pieces */
    return;
}
