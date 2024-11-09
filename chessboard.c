#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include "chessboard.h"
#include "evaluation.h"
#include "moves.h"
#include "move_stack.h"

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
    board->moves = (move_stack *)malloc(sizeof(move_stack));

    init_pieces(board->white); /* Allocate memory for white pieces */
    init_pieces(board->black); /* Allocate memory for black pieces */
    set_piece_counts(board); /* Set initial piece counts */

    // Initialize the square table
    init_square_table(board);
    set_pieces(board); /* Set initial positions of pieces */
    init_move_stack(board->moves);

    board->attack_tables[WHITE] = 4294901760; // initial attack table for white hardcoded
	board->attack_tables[BLACK] = 281470681743360;// initial attack table for black hardcoded
    board->castle_rights = 0b01110111;

    board->captured_pieces_count[WHITE] = 0;
    board->captured_pieces_count[BLACK] = 0;

    for(int i=0; i<16; i++) {
        board->captured_pieces[WHITE][i] = 0;
        board->captured_pieces[BLACK][i] = 0;
    }
    return;
}


// Chessboard functions
void print_piece_from_id(uint8_t id) {
    switch(piece_type(id)) {
        case PAWN:
            wprintf(L"♙ ");
            break;
        case KNIGHT:
            wprintf(L"♘ ");
            break;
        case BISHOP:
            wprintf(L"♗ ");
            break;
        case ROOK:
            wprintf(L"♖ ");
            break;
        case QUEEN:
            wprintf(L"♕ ");
            break;
        case KING:
            wprintf(L"♔ ");
            break;
        default:
            break;
    }
    return;
}

void print_captured_pieces(board *b) {
    // wchar_t black_pieces[] = {L'♙', L'♘', L'♗', L'♖', L'♕', L'♔',};
	// wchar_t white_pieces[] = {L'♟', L'♞', L'♝', L'♜', L'♛', L'♚',};

    wprintf(L"Captured pieces: \n");
    wprintf(L"White: ");
    for(int i=0; i<b->captured_pieces_count[WHITE]; i++) {
       print_piece_from_id(b->captured_pieces[WHITE][i]);
    }
    wprintf(L"\n");

    wprintf(L"Black: ");
    for(int i=0; i<b->captured_pieces_count[BLACK]; i++) {
       print_piece_from_id(b->captured_pieces[BLACK][i]);
    }
    wprintf(L"\n");
    return;
}


// Function to print the chessboard
void print_board(board *b, short turn) {
    // wchar_t white_pieces[] = {L'P', L'N', L'B', L'R', L'Q', L'K'};
	// wchar_t black_pieces[] = {L'p', L'n', L'b', L'r', L'q', L'k'};
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
	display_evaluation(get_evaluation_of_board(b));

    print_captured_pieces(b);
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
    // wprintf(L"\n");
    return;
}

short *get_pointer_to_piece_counter(board *b, uint8_t piece_id) {
    short *piece_counter;
    switch(piece_type(piece_id)) {
        case PAWN:
            piece_counter = &b->white->count.pawns;
            break;
        case KNIGHT:
            piece_counter = &b->white->count.knights;
            break;
        case BISHOP:
            piece_counter = &b->white->count.bishops;
            break;
        case ROOK:
            piece_counter = &b->white->count.rooks;
            break;
        case QUEEN:
            piece_counter = &b->white->count.queens;
            break;

        default:
            piece_counter = NULL;
    }
    return piece_counter;
}

uint64_t *get_pointer_to_piece_type(short color, uint8_t piece_type, board *b) {
    uint64_t *piece;
    switch(piece_type) {
        case PAWN:
            piece = color == WHITE ? b->white->pawns : b->black->pawns;
            break;
        case KNIGHT:
            piece = color == WHITE ? b->white->knights : b->black->knights;
            break;
        case BISHOP:
            piece = color == WHITE ? b->white->bishops : b->black->bishops;
            break;
        case ROOK:
            piece = color == WHITE ? b->white->rooks : b->black->rooks;
            break;
        case QUEEN:
            piece = color == WHITE ? b->white->queen : b->black->queen;
            break;
        case KING:
            piece = color == WHITE ? &b->white->king : &b->black->king;
            break;
        default:
            piece = NULL;
    }
    return piece;
}

uint8_t generate_id_for_promoted_piece(uint8_t piece_type, short color, board *b) {
    uint8_t piece_color = color == WHITE ? 0 : 8;
    short *piece_counter = get_pointer_to_piece_counter(b, piece_type);
    if (!piece_counter) return 0;
    (*piece_counter)++;
    uint8_t piece_number = *piece_counter;
    return (piece_color | piece_type | ((piece_number - 1) << 4));
}

// pawn promotion : this function needs to be rewritten using realloc, malloc was used for debugging purpose
uint8_t new_piece(short color, uint8_t _piece_type, uint64_t position_bb, board *b) {
    /*
        uint8_t piece_color = color == WHITE ? 0 : 8;

        // Get the pointer to the piece count and increment it
        short *piece_counter = get_pointer_to_piece_counter(b, _piece_type);
        if (!piece_counter) return 0;
        (*piece_counter)++;
        uint8_t piece_number = *piece_counter;

        // Calculate the piece id
        uint8_t piece_id = piece_color | _piece_type | ((piece_number - 1) << 4);
    */

    // Generate the piece id for the promoted piece
    uint8_t piece_id = generate_id_for_promoted_piece(_piece_type, color, b);
    uint8_t piece_number = (piece_id & 0b01110000) >> 4;

    // Get the pointer to the piece type array
    uint64_t *piece_type_ptr = get_pointer_to_piece_type(color, _piece_type, b);

    // Allocate new memory block of increased size
    uint64_t *new_piece_type_ptr = (uint64_t *)malloc(sizeof(uint64_t) * piece_number);
    if (new_piece_type_ptr == NULL) {
        return 0; // Handle allocation failure
    }

    // Copy old data if piece_type_ptr is non-null (i.e., not the first piece)
    if (piece_type_ptr != NULL) {
        for (int i = 0; i < piece_number - 1; i++) {
            new_piece_type_ptr[i] = piece_type_ptr[i];
        }
        free(piece_type_ptr); // Free the old memory
    }

    // Set the new position for the latest piece at the last index
    new_piece_type_ptr[piece_number - 1] = position_bb;

    // Update the board's piece pointer to point to the newly allocated memory
    if (color == WHITE) {
        switch (_piece_type) {
            case PAWN:   b->white->pawns = new_piece_type_ptr; break;
            case KNIGHT: b->white->knights = new_piece_type_ptr; break;
            case BISHOP: b->white->bishops = new_piece_type_ptr; break;
            case ROOK:   b->white->rooks = new_piece_type_ptr; break;
            case QUEEN:  b->white->queen = new_piece_type_ptr; break;
        }
    } else {
        switch (_piece_type) {
            case PAWN:   b->black->pawns = new_piece_type_ptr; break;
            case KNIGHT: b->black->knights = new_piece_type_ptr; break;
            case BISHOP: b->black->bishops = new_piece_type_ptr; break;
            case ROOK:   b->black->rooks = new_piece_type_ptr; break;
            case QUEEN:  b->black->queen = new_piece_type_ptr; break;
        }
    }

    // Debugging print to verify bitboard values
    for (int i = 0; i < piece_number; i++) {
        wprintf(L"Piece %d bitboard: ", i + 1);
        print_square_from_bitboard(new_piece_type_ptr[i]);
        wprintf(L"\n");
    }

    // Update the square table and return the new piece ID
    square dest = get_square_from_bitboard(position_bb);
    update_square_table(dest.file, dest.rank, piece_id, b);

    return piece_id;
}


// PENDING: function to free the memory allocated for the board
