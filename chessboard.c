#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "chessboard.h"
#include "move_types.h"
#include "moves.h"
#include "move_stack.h"
#include "move_array.h"

// Utility functions
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
	return (piece_id & 0b01110000) >> 4;
}

// function to get bitboard from rank and file
uint64_t get_bitboard(uint8_t file, uint8_t rank) {
	// Check if the square is valid
	if (rank < 0 || rank > 8 || file < 0 || file > 8) return 0;

	// Create a bitboard with the bit set at the index of the square
	uint64_t bitboard = 1ULL;

	// calculate the no. of times bit needs to be shifted
	int index = (rank - 1) * 8 + file - 1;

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

	for (int i = 0; i < type->count.queens; i++) {
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

short *get_pointer_to_piece_counter(board *b, uint8_t piece_id) {
	short *piece_counter;
	switch (piece_type(piece_id)) {
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

uint64_t **get_pointer_to_piece_type(short color, uint8_t piece_type, board *b) {
	uint64_t **piece;
	switch (piece_type) {
		case PAWN:
			piece = color == WHITE ? &b->white->pawns : &b->black->pawns;
			break;
		case KNIGHT:
			piece = color == WHITE ? &b->white->knights : &b->black->knights;
			break;
		case BISHOP:
			piece = color == WHITE ? &b->white->bishops : &b->black->bishops;
			break;
		case ROOK:
			piece = color == WHITE ? &b->white->rooks : &b->black->rooks;
			break;
		case QUEEN:
			piece = color == WHITE ? &b->white->queen : &b->black->queen;
			break;
		default:
			piece = NULL;
	}
	return piece;
}
/* square table functions */
void update_square_table(int file, int rank, uint8_t piece, board *b) {
	b->square_table[file - 1][rank - 1] = piece;
	return;
}

// Chessboard functions

// Chessboard functions
void init_board(board *b) {
	if (!b) return;

	// Allocate memory for pieces
	b->white = (pieces *)malloc(sizeof(pieces));
	if (!b->white) {
		printf("Error: Failed to allocate memory for white pieces\n");
		return;
	}

	b->black = (pieces *)malloc(sizeof(pieces));
	if (!b->black) {
		free(b->white);
		printf("Error: Failed to allocate memory for black pieces\n");
		return;
	}

	// Allocate and initialize move stack
	b->moves = (move_stack *)malloc(sizeof(move_stack));
	if (!b->moves) {
		free(b->white);
		free(b->black);
		printf("Error: Failed to allocate memory for move stack\n");
		return;
	}
	init_move_stack(b->moves);

	// Allocate and initialize attack move lists
	b->white_attacks = (MoveList *)malloc(sizeof(MoveList));
	if (!b->white_attacks) {
		free(b->white);
		free(b->black);
		free(b->moves);
		printf("Error: Failed to allocate memory for white attacks\n");
		return;
	}
	init_movelist(b->white_attacks);  // This now initializes array instead of linked list

	b->black_attacks = (MoveList *)malloc(sizeof(MoveList));
	if (!b->black_attacks) {
		free(b->white);
		free(b->black);
		free(b->moves);
		free(b->white_attacks->moves);  // Free the moves array
		free(b->white_attacks);
		printf("Error: Failed to allocate memory for black attacks\n");
		return;
	}
	init_movelist(b->black_attacks);  // This now initializes array instead of linked list

	b->white_legal_moves = (MoveList *)malloc(sizeof(MoveList));
	if (!b->white_legal_moves) {
		free(b->white);
		free(b->black);
		free(b->moves);
		free(b->white_attacks->moves);  // Free the moves array
		free(b->white_attacks);
		free(b->black_attacks->moves);  // Free the moves array
		free(b->black_attacks);
		printf("Error: Failed to allocate memory for white legal moves\n");
		return;
	}
	init_movelist(b->white_legal_moves);  // This now initializes array instead of linked list

	b->black_legal_moves = (MoveList *)malloc(sizeof(MoveList));
	if (!b->black_legal_moves) {
		free(b->white);
		free(b->black);
		free(b->moves);
		free(b->white_attacks->moves);  // Free the moves array
		free(b->white_attacks);
		free(b->black_attacks->moves);  // Free the moves array
		free(b->black_attacks);
		free(b->white_legal_moves->moves);  // Free the moves array
		free(b->white_legal_moves);
		printf("Error: Failed to allocate memory for black legal moves\n");
		return;
	}
	init_movelist(b->black_legal_moves);  // This now initializes array instead of linked list

	// Initialize bitboards and other fields
	b->white_board = 0ULL;
	b->black_board = 0ULL;
	b->en_passant_square = 0ULL;
	b->castle_rights = 0b01110111;

	// Initialize captured pieces count
	b->captured_pieces_count[0] = 0;
	b->captured_pieces_count[1] = 0;

	// Clear square table
	memset(b->square_table, 0, sizeof(b->square_table));
	memset(b->white_lookup_table, 0, sizeof(b->white_lookup_table));
	memset(b->black_lookup_table, 0, sizeof(b->black_lookup_table));

	// Clear captured pieces arrays
	memset(b->captured_pieces[0], 0, sizeof(b->captured_pieces[0]));
	memset(b->captured_pieces[1], 0, sizeof(b->captured_pieces[1]));
}
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
	b->white->count.pawns = 0;
	b->white->count.knights = 0;
	b->white->count.bishops = 0;
	b->white->count.rooks = 0;
	b->white->count.queens = 0;

	b->black->count.pawns = 0;
	b->black->count.knights = 0;
	b->black->count.bishops = 0;
	b->black->count.rooks = 0;
	b->black->count.queens = 0;
	return;
}

uint8_t generate_id_for_piece(uint8_t piece_type, short color, short piece_number) {
	return (piece_type | (color << 3) | (piece_number << 4));
}

uint64_t place_piece(uint64_t *piece_board, int file, int rank, uint8_t piece, board *b) {
	*piece_board = get_bitboard(file, rank);
	update_square_table(file, rank, piece, b);
	return *piece_board;
}

void load_fen(board *b, char *fen) {
	if (!b || !fen) return;

	memset(b->square_table, 0, sizeof(b->square_table));
	memset(b->captured_pieces, 0, sizeof(b->captured_pieces));
	memset(b->captured_pieces_count, 0, sizeof(b->captured_pieces_count));

	// Initialize the board and pieces
	init_board(b);
	init_pieces(b->white);
	init_pieces(b->black);
	set_piece_counts(b);

	int expected_piece_count[2][6] = {
	    {8, 2, 2, 2, 1, 1},
	    {8, 2, 2, 2, 1, 1}};

	// Parse the FEN and place pieces
	int rank = 8, file = 1;

	for (int i = 0; fen[i] != ' '; i++) {
		if (fen[i] == '/') {
			rank--;
			file = 1;
			continue;
		}

		if (fen[i] >= '1' && fen[i] <= '8') {
			file += fen[i] - '0';
			continue;
		}

		uint8_t piece = 0;

		switch (fen[i]) {
			case 'P':
				b->white->count.pawns++;
				expected_piece_count[WHITE][0]--;
				piece = generate_id_for_piece(PAWN, WHITE, b->white->count.pawns - 1);
				b->white_board |= place_piece(b->white->pawns + (b->white->count.pawns - 1), file, rank, piece, b);
				break;
			case 'p':
				b->black->count.pawns++;
				expected_piece_count[BLACK][0]--;
				piece = generate_id_for_piece(PAWN, BLACK, b->black->count.pawns - 1);
				b->black_board |= place_piece(b->black->pawns + (b->black->count.pawns - 1), file, rank, piece, b);
				break;
			case 'R':
				b->white->count.rooks++;
				expected_piece_count[WHITE][1]--;
				piece = generate_id_for_piece(ROOK, WHITE, b->white->count.rooks - 1);
				b->white_board |= place_piece(b->white->rooks + (b->white->count.rooks - 1), file, rank, piece, b);
				break;
			case 'r':
				b->black->count.rooks++;
				expected_piece_count[BLACK][1]--;
				piece = generate_id_for_piece(ROOK, BLACK, b->black->count.rooks - 1);
				b->black_board |= place_piece(b->black->rooks + (b->black->count.rooks - 1), file, rank, piece, b);
				break;
			case 'N':
				b->white->count.knights++;
				expected_piece_count[WHITE][2]--;
				piece = generate_id_for_piece(KNIGHT, WHITE, b->white->count.knights - 1);
				b->white_board |= place_piece(b->white->knights + (b->white->count.knights - 1), file, rank, piece, b);
				break;
			case 'n':
				b->black->count.knights++;
				expected_piece_count[BLACK][2]--;
				piece = generate_id_for_piece(KNIGHT, BLACK, b->black->count.knights - 1);
				b->black_board |= place_piece(b->black->knights + (b->black->count.knights - 1), file, rank, piece, b);
				break;
			case 'B':
				b->white->count.bishops++;
				expected_piece_count[WHITE][3]--;
				piece = generate_id_for_piece(BISHOP, WHITE, b->white->count.bishops - 1);
				b->white_board |= place_piece(b->white->bishops + (b->white->count.bishops - 1), file, rank, piece, b);
				break;
			case 'b':
				b->black->count.bishops++;
				expected_piece_count[BLACK][3]--;
				piece = generate_id_for_piece(BISHOP, BLACK, b->black->count.bishops - 1);
				b->black_board |= place_piece(b->black->bishops + (b->black->count.bishops - 1), file, rank, piece, b);
				break;
			case 'Q':
				b->white->count.queens++;
				expected_piece_count[WHITE][4]--;
				piece = generate_id_for_piece(QUEEN, WHITE, b->white->count.queens - 1);
				b->white_board |= place_piece(b->white->queen + (b->white->count.queens - 1), file, rank, piece, b);
				break;
			case 'q':
				b->black->count.queens++;
				expected_piece_count[BLACK][4]--;
				piece = generate_id_for_piece(QUEEN, BLACK, b->black->count.queens - 1);
				b->black_board |= place_piece(b->black->queen + (b->black->count.queens - 1), file, rank, piece, b);
				break;
			case 'K':
				expected_piece_count[WHITE][5]--;
				piece = generate_id_for_piece(KING, WHITE, 0);
				b->white_board |= place_piece(&b->white->king, file, rank, piece, b);
				break;
			case 'k':
				expected_piece_count[BLACK][5]--;
				piece = generate_id_for_piece(KING, BLACK, 0);
				b->black_board |= place_piece(&b->black->king, file, rank, piece, b);
				break;
			default:
				break;
		}
		file++;
	}

	// Now process the en passant target square (it's the part after the first space)
	char *en_passant = strchr(fen, ' ');  // Find the first space, then move to the next part

	while (*en_passant == ' ' || *en_passant == '-' || *en_passant == 'w' || *en_passant == 'b') {
		en_passant++;
	}

	// Extract the en passant square, check if valid (not '-')
	if (*en_passant != '-') {
		char file_char = en_passant[0];  // File character (e.g., 'c')
		char rank_char = en_passant[1];  // Rank character (e.g., '6')

		// Convert to file and rank (file 'a' = 1, 'b' = 2, ..., 'h' = 8)
		int en_passant_file = file_char - 'a' + 1;
		int en_passant_rank = rank_char - '0';

		// Set the en passant square bitboard
		b->en_passant_square = get_bitboard(en_passant_file, en_passant_rank);
	} else {
		// No en passant available
		b->en_passant_square = 0;
	}

	// castling rights
	char *castle_rights = strchr(fen, ' ');              // Find the first space, then move to the next part
	castle_rights = strchr(castle_rights + 1, ' ') + 1;  // Find the second space, then move to the next part

	if (strchr(castle_rights, 'K')) {
		// White can castle kingside
		b->castle_rights |= WHITE_KING_SIDE_CASTLE_RIGHTS;
	}
	if (strchr(castle_rights, 'Q')) {
		// White can castle queenside
		b->castle_rights |= WHITE_QUEEN_SIDE_CASTLE_RIGHTS;
	}
	if (strchr(castle_rights, 'k')) {
		// Black can castle kingside
		b->castle_rights |= BLACK_KING_SIDE_CASTLE_RIGHTS;
	}
	if (strchr(castle_rights, 'q')) {
		// Black can castle queenside
		b->castle_rights |= BLACK_QUEEN_SIDE_CASTLE_RIGHTS;
	}

	if (strchr(castle_rights, 'K') == NULL && strchr(castle_rights, 'Q') == NULL && strchr(castle_rights, 'k') == NULL && strchr(castle_rights, 'q') == NULL) {
		// No castling rights available
		b->castle_rights = 0;
	}

	// Populate captured pieces based on remaining counts in expected_piece_count
	for (int color = 0; color < 2; color++) {
		int index = 0;
		for (int type = 0; type < 6; type++) {
			for (int count = 0; count < expected_piece_count[color][type]; count++) {
				b->captured_pieces[color][index++] = generate_id_for_piece(type, color, count);
			}
			b->captured_pieces_count[color] += expected_piece_count[color][type];
		}
	}
}

// Chessboard functions
void print_piece_from_id(uint8_t id) {
    switch(piece_type(id)) {
        case PAWN:
            wprintf(!piece_color(id) ? L"♟ " : L"♙ ");
            break;
        case KNIGHT:
            // wprintf(L"♘ ");
			wprintf(!piece_color(id) ? L"♞ " : L"♘ ");
            break;
        case BISHOP:
            // wprintf(L"♗ ");
			wprintf(!piece_color(id) ? L"♝ " : L"♗ ");
            break;
        case ROOK:
            // wprintf(L"♖ ");
			wprintf(!piece_color(id) ? L"♜ " : L"♖ ");
            break;
        case QUEEN:
            // wprintf(L"♕ ");
			wprintf(!piece_color(id) ? L"♛ " : L"♕ ");
            break;
        case KING:
            // wprintf(L"♔ ");
			wprintf(!piece_color(id) ? L"♚ " : L"♔ ");
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
	print_captured_pieces(b);
	wchar_t white_pieces[] = {L'P', L'N', L'B', L'R', L'Q', L'K'};
	wchar_t black_pieces[] = {L'p', L'n', L'b', L'r', L'q', L'k'};
	// wchar_t black_pieces[] = {
	//     L'♙',
	//     L'♘',
	//     L'♗',
	//     L'♖',
	//     L'♕',
	//     L'♔',
	// };
	// wchar_t white_pieces[] = {
	//     L'♟',
	//     L'♞',
	//     L'♝',
	//     L'♜',
	//     L'♛',
	//     L'♚',
	// };

	/*
	WHY & 7? : The piece is stored in the square table as a 8-bit number. The lower 3 bits represent the piece type.
	*/
	uint8_t piece, start_rank, end_rank, start_file, end_file;

	/*
	Note: Context of start and end are from printing pov
	for e.g. for white side, A1 might be the start and H8 might be the end but A8 wil be printed first and H1 will be printed last
	*/
	switch (turn) {
		case WHITE: {
			start_file = A;
			start_rank = 8;
			end_file = H;
			end_rank = 1;
			break;
		}
		case BLACK: {
			start_file = H;
			start_rank = 1;
			end_file = A;
			end_rank = 8;
			break;
		}

		default:
			return;
	}

	// print_captured_pieces(b);
	for (int rank = start_rank; turn == WHITE ? rank >= end_rank : rank <= end_rank; turn == WHITE ? rank-- : rank++) {
		wprintf(L"\t\t+---+---+---+---+---+---+---+---+\n");
		wprintf(L"\t\t|");
		for (int file = start_file; turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--) {
			piece = b->square_table[file - 1][rank - 1];
			/* If piece is present on the square then print the piece*/
			if (piece)
				wprintf(L" %lc |", piece & 8 ? black_pieces[(piece & 7) - 1] : white_pieces[(piece & 7) - 1]);

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

	for (int file = start_file; turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--) {
		wprintf(L" %c  ", _files[file - 1]);
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
	for (int i = 0; i < 64; i++) {
		if (moves & bitboard) {
			print_square_from_bitboard(bitboard);
			wprintf(L", ");
		}
		bitboard <<= 1;
	}
	// wprintf(L"\n");
	return;
}