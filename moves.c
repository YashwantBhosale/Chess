#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "chessboard.h"
#include "moves.h"
#include "move_stack.h"

// some prototypes
uint64_t get_combined_lookup_table(board *b, short color);
bool in_check(short color, board *b);


// Helper functions
uint64_t rankmask(int rank) {
	return 0xFFULL << (8 * (rank - 1));
}

// small helper function
unsigned int absolute(int x) {
	return (x < 0) ? -x : x;
}


uint64_t filemask(int file) {
	// return 0x0101010101010101ULL << (file - 1);
	return 0b0000000100000001000000010000000100000001000000010000000100000001ULL << (file - 1);
}
/*
101 < 1 = 010
111100011 < 2 = 111000110

00000001 00000001 00000001 00000001 00000001 00000001 00000001 00000001
00000010 00000010 00000010 00000010 00000010 00000010 00000010 00000010
00000100 00000100 00000100 00000100 00000100 00000100 00000100 00000100

10000000 10000000 10000000 10000000 10000000 10000000 10000000 10000000
*/

void get_player_board_and_opp_board(short color, board *b, uint64_t *pb, uint64_t *ob) {
	switch (color) {
		case WHITE: {
			*pb = white_board(b);
			*ob = black_board(b);
			break;
		}
		case BLACK: {
			*pb = black_board(b);
			*ob = white_board(b);
			break;
		}
		default:
			return;
	}
}

/* This function returns pointer to the piece bitboard with argument as piece is */
uint64_t *get_pointer_to_piece(uint8_t piece_id, board *b) {
	uint8_t type, color, index;
	type = piece_type(piece_id);     // last 3 bits represent type
	color = piece_color(piece_id);   // 4th bit represents color
	index = piece_number(piece_id);  // 5th, 6th and 7th bits represent index

	switch (type) {
		case EMPTY_SQUARE:
			return NULL;
		case PAWN:
			return color == WHITE ? &b->white->pawns[index] : &b->black->pawns[index];

		case KNIGHT:
			return color == WHITE ? &b->white->knights[index] : &b->black->knights[index];

		case BISHOP:
			return color == WHITE ? &b->white->bishops[index] : &b->black->bishops[index];

		case ROOK:
			return color == WHITE ? &b->white->rooks[index] : &b->black->rooks[index];

		case QUEEN:
			return color == WHITE ? &b->white->queen[index] : &b->black->queen[index];

		case KING:
			return color == WHITE ? &b->white->king : &b->black->king;

		default:
			break;
	}
	return NULL;
}

/*
    Move generation functions:
    For most of the pieces direction of movement is same for both colors, except for pawns but directions are considered based on color so that it is easier to imagine while writing lookup functions for each piece
    This statergy is subject to change based on the implementation of lookup tables
*/

// function to generate forward direction move
uint64_t move_north(uint64_t move, short color) {
	uint64_t new_move = move << 8;
	return new_move;
}

// function to generate backward direction move
uint64_t move_south(uint64_t move, short color) {
	return move >> 8;
}

// function to generate left direction move
uint64_t move_west(uint64_t move, short color) {
	// if the move is on the leftmost file then it should not be allowed to move left
	if (move & filemask(1)) {
		return 0ULL;
	}

	return move >> 1;
}

// function to generate right direction move
uint64_t move_east(uint64_t move, short color) {
	// if the move is on the rightmost file then it should not be allowed to move right
	if (move & filemask(8)) {
		return 0ULL;
	}

	return move << 1;
}

// function to generate north-east direction move
uint64_t move_north_east(uint64_t move, short color) {
	uint64_t new_move = move_north(move, color);
	new_move = move_east(new_move, color);

	return new_move;
}

// function to generate north-west direction move
uint64_t move_north_west(uint64_t move, short color) {
	uint64_t new_move = move_north(move, color);
	new_move = move_west(new_move, color);

	return new_move;
}

// function to generate south-east direction move
uint64_t move_south_east(uint64_t move, short color) {
	uint64_t new_move = move_south(move, color);
	new_move = move_east(new_move, color);

	return new_move;
}

// function to generate south-west direction move
uint64_t move_south_west(uint64_t move, short color) {
	uint64_t new_move = move_south(move, color);
	new_move = move_west(new_move, color);

	return new_move;
}


// function to validate a move so that we do not step on our own piece: pb->player board, ob->opponent board
uint64_t validate_move(uint64_t move, uint64_t pb, uint64_t ob) {
	return move & ~pb;
}
// function to Make sure that the piece that is being captured is enemy piece
uint64_t validate_capture(uint64_t move, uint64_t pb, uint64_t ob) {
	return move & ob;
}

// function to validate en passant
uint64_t validate_en_passant(uint64_t piece_bitboard, short color, uint64_t en_passant, board *b) {
	square src, dest;
	src = get_square_from_bitboard(piece_bitboard);
	dest = get_square_from_bitboard(en_passant);

	uint8_t piece, dest_piece;
	piece = b->square_table[src.file-1][src.rank-1];
	dest_piece = b->square_table[dest.file-1][dest.rank-1];

	// // en passant check
	if (piece_type(piece) == PAWN && absolute(src.rank - dest.rank) == 1 && dest_piece == EMPTY_SQUARE) {
		Move last_move = peek(*(b->moves));
		if ((last_move.piece & 7) == PAWN && absolute(last_move.src.rank - last_move.dest.rank) == 2 && last_move.dest.file == dest.file && piece_color(last_move.piece) != color) {
			
			if(color == WHITE && piece_bitboard & rankmask(5)){
				return en_passant;
			}
			else if(color == BLACK && piece_bitboard & rankmask(4)) {
				return en_passant;
			}
			else {
				return 0ULL;
			}
		}
	} 

	

	return 0ULL;
}


/* 
	CASTLING:
	Base conditions for castling:
	1. It should be the first move of the king and the rook.
	2. There should be no pieces between the king and the rook.
	3. The king should not be in check.
	4. The squares the king moves over should not be under attack.
	5. The king should not move through a square that is attacked.

	King side castling:
	1. King moves two squares to the right.
	2. Rook moves two squares to the left.

	Queen side castling:
	1. King moves two squares to the left.
	2. Rook moves three squares to the right.

	Raw idea:
	Implementation:
	1. Castling rights:
	8-bit flag may be used to store castling rights.
	format: xBBB xWWW
	BBB -> represents black castling rights
	WWW -> represents white castling rights

	in 3-bit number: 
	MSB = 1 means king hasn't moved
	so, masks to check castling rights: { 0b101, 0b110 }
	0XX -> no castling rights
	101 -> king side castling rights
	110 -> queen side castling rights
	111 -> both castling rights

	in this configuration:
	for black:
	masks = { 0b01010000, 0b01100000, 0b01110000 }
	for white:
	masks = { 0b00000101, 0b00000110, 0b00000111 }

	which bitwise operation would be useful?
	-> the main idea is :
	flag <bitwise-operator> mask = <Result>
	This Result should give us exactly one mask which is present in the masks array.
	For this, we can use bitwise AND operation.
*/

// function to validate castling
uint64_t validate_castle(uint64_t king, short color, board *b) {


	/*
		!! IMPORTANT NOTE: This function assumes that the basic check of castle rights is done.
		that is the castle rights are checked before calling this function. as a consequence,
		at least one of the castle rights should be present in the castle rights flag for 
		given color.
	*/ 

	uint64_t player_board, opp_board, king_move, cursor, opponent_attacks, queen_move;
	get_player_board_and_opp_board(color, b, &player_board, &opp_board);

	// opponent_attacks = get_combined_lookup_table(b, !color);
	opponent_attacks = b->attack_tables[!color];

	king_move = 0ULL;
	queen_move = 0ULL;

	if(king & opponent_attacks) {
		return 0ULL;
	}
	// king side castle
	if (b->castle_rights & (color == WHITE ? WHITE_KING_SIDE_CASTLE_RIGHTS : BLACK_KING_SIDE_CASTLE_RIGHTS)) {

		cursor = king;

		// we gotta move 2 squares in the east direction for king side castle
		for (int i = 0; i < 2; i++) {
			cursor = move_east(cursor, color);
			if (cursor & (player_board | opp_board)) {
				king_move = 0ULL;
				break;
			}
			
			if(cursor & opponent_attacks) {
				king_move = 0ULL;
				break;
			}

			king_move |= cursor;
		}

		print_moves(king_move);

	}
	if(b->castle_rights & (color == WHITE ? WHITE_QUEEN_SIDE_CASTLE_RIGHTS : BLACK_QUEEN_SIDE_CASTLE_RIGHTS)){


		cursor = king;

		// we gotta move 2 squares in the west direction for queen side castle
		for (int i = 0; i < 2; i++) {
			cursor = move_west(cursor, color);
			if (cursor & (player_board | opp_board)) {
				queen_move = 0ULL;
				break;
			}
			
			if(cursor & opponent_attacks) {
				queen_move = 0ULL;
				break;
			}

			queen_move |= cursor;
		}

		print_moves(queen_move);
	}



	return king_move | queen_move;
}



/* piece wise lookup functions */
uint64_t pawn_lookup(uint64_t pawn, short color, board *b) {
	/*
	1. check if pawn can step forward (that square should not be occupied by enemy / friendly piece)
	2. check if its pawn's first move (if yes then it can move 2 squares forward)
	3. check if pawn can capture enemy piece on two diagonals
	4. enpassant
	5. promotion
	*/
	uint64_t player_board, opp_board, move, lookup;

	lookup = 0ULL;
	move = 0ULL;
	/*
	 NOTE:
	 Always make sure that you are checking color in right way
	 as an example,
	 we use d3 bit to represent color of the piece so if we are using that information we need to right shift by 3 as our program as a whole recognizes BLACK = 1, WHITE = 0
	 so after masking if we get 8 === 0b00001000 then we need to right shift by 3 to get 1
	*/
	get_player_board_and_opp_board(color, b, &player_board, &opp_board);

	switch (color) {
		case WHITE: {
			/* 1. check if pawn can move forward */
			move = move_north(pawn, color);

			/* IMPORTANT: AS WE CONSIDER PIECES OF BOTH COLORS AS NON-CAPTURABLE PIECES IN CASE OF FORWARD MOVES OF PAWN, WE PASS COMBINED BOARD OF PLAYER AND OPPONENT AS PLAYER BOARD */
			move = validate_move(move, player_board | opp_board, 0ULL);
			lookup |= move;

			/* 2. check if its pawn's first move */
			if (pawn & rankmask(2)) {
				move = move_north(move, color);
				move = validate_move(move, player_board | opp_board, 0ULL);
				lookup |= move;
			}

			/* 3. check if pawn can capture enemy piece on two diagonals */
			move = move_north_east(pawn, color);
			move = validate_capture(move, player_board, opp_board);
			lookup |= move;

			move = move_north_west(pawn, color);
			move = validate_capture(move, player_board, opp_board);
			lookup |= move;

			/* 4. enpassant */
			uint64_t en_passant;
			en_passant = move_north_east(pawn, color);
			if (en_passant) {
				move = validate_en_passant(pawn, color, en_passant, b);
				lookup |= move;
			}

			en_passant = move_north_west(pawn, color);
			if (en_passant) {
				move = validate_en_passant(pawn, color, en_passant, b);
				lookup |= move;
			}
			break;
		}
		case BLACK: {
			move = move_south(pawn, color);
			move = validate_move(move, player_board | opp_board, 0ULL);
			lookup |= move;

			if (pawn & rankmask(7)) {
				move = move_south(move, color);
				move = validate_move(move, player_board | opp_board, 0ULL);
				lookup |= move;
			}

			move = move_south_west(pawn, color);
			move = validate_capture(move, player_board, opp_board);
			lookup |= move;

			move = move_south_east(pawn, color);
			move = validate_capture(move, player_board, opp_board);
			lookup |= move;

			uint64_t en_passant;
			en_passant = move_south_west(pawn, color);
			if (en_passant) {
				move = validate_en_passant(pawn, color, en_passant, b);
				lookup |= move;
			}

			en_passant = move_south_east(pawn, color);
			if (en_passant) {
				move = validate_en_passant(pawn, color, en_passant, b);
				lookup |= move;
			}
			break;
		}
		default:
			break;
	}

	/*
	    PENDING: 1. Enpassant, 2. Promotion
	*/

	return lookup;
}

uint64_t knight_lookup(uint64_t knight, short color, board *b) {
	uint64_t player_board, opp_board, move, lookup;

	move = 0ULL;
	lookup = 0ULL;

	get_player_board_and_opp_board(color, b, &player_board, &opp_board);

	/* 1. check if knight can move north-north-east */
	move = move_north(move_north(move_east(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 2. check if knight can move north-east-east */
	move = move_north(move_east(move_east(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 3. check if knight can move south-east-east */
	move = move_south(move_east(move_east(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 4. check if knight can move south-south-east */
	move = move_south(move_south(move_east(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 5. check if knight can move south-south-west */
	move = move_south(move_south(move_west(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 6. check if knight can move south-west-west */
	move = move_south(move_west(move_west(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 7. check if knight can move north-west-west */
	move = move_north(move_west(move_west(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 8. check if knight can move north-north-west */
	move = move_north(move_north(move_west(knight, color), color), color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	return lookup;
}

uint64_t bishop_lookup(uint64_t bishop, short color, board *b) {
	uint64_t player_board, opp_board, move, cursor, lookup;

	move = 0ULL;
	lookup = 0ULL;

	get_player_board_and_opp_board(color, b, &player_board, &opp_board);

	/* 1. check if bishop can move north-east */
	cursor = bishop;
	while (cursor) {
		move = move_north_east(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if bishop can capture enemy piece then break
			break;
	}

	/* 2. check if bishop can move north-west */
	cursor = bishop;
	while (cursor) {
		move = move_north_west(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if bishop can capture enemy piece then break
			break;
	}

	/* 3. check if bishop can move south-east */
	cursor = bishop;
	while (cursor) {
		move = move_south_east(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if bishop can capture enemy piece then break
			break;
	}

	/* 4. check if bishop can move south-west */
	cursor = bishop;
	while (cursor) {
		move = move_south_west(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if bishop can capture enemy piece then break
			break;
	}

	return lookup;
}
uint64_t rook_lookup(uint64_t rook, short color, board *b) {
	uint64_t player_board, opp_board, move, cursor, lookup;

	move = 0ULL;
	lookup = 0ULL;

	get_player_board_and_opp_board(color, b, &player_board, &opp_board);

	/* 1. check if rook can move north */
	cursor = rook;
	while (cursor) {
		move = move_north(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if rook can capture enemy piece then break
			break;
	}

	/* 2. check if rook can move south */
	cursor = rook;
	while (cursor) {
		move = move_south(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if rook can capture enemy piece then break
			break;
	}

	/* 3. check if rook can move east */
	cursor = rook;
	while (cursor) {
		move = move_east(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if rook can capture enemy piece then break
			break;
	}

	/* 4. check if rook can move west */
	cursor = rook;
	while (cursor) {
		move = move_west(cursor, color);
		move = validate_move(move, player_board, opp_board);
		lookup |= move;  // update the lookup table with the new move
		cursor = move;   // update the cursor to the new position

		if (move & opp_board)  // if rook can capture enemy piece then break
			break;
	}

	return lookup;
}

uint64_t queen_lookup(uint64_t queen, short color, board *b) {
	return bishop_lookup(queen, color, b) | rook_lookup(queen, color, b);
}

uint64_t king_lookup(uint64_t king, short color, board *b) {
	uint64_t player_board, opp_board, move, lookup;

	move = 0ULL;
	lookup = 0ULL;

	get_player_board_and_opp_board(color, b, &player_board, &opp_board);

	/* 1. check if king can move north */
	move = move_north(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 2. check if king can move south */
	move = move_south(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 3. check if king can move east */
	move = move_east(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 4. check if king can move west */
	move = move_west(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 5. check if king can move north-east */
	move = move_north_east(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 6. check if king can move north-west */
	move = move_north_west(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 7. check if king can move south-east */
	move = move_south_east(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 8. check if king can move south-west */
	move = move_south_west(king, color);
	move = validate_move(move, player_board, opp_board);
	lookup |= move;

	/* 9. check if king can castle */
	if(b->castle_rights & (color == WHITE ? WHITE_CASTLE_RIGHTS : BLACK_CASTLE_RIGHTS)){
		// wprintf(L"%s King can castle\n", color == WHITE ? "White" : "Black");
		move = validate_castle(king, color, b);
		print_moves(move);
		lookup |= move;
	}

	return lookup;
}

// function to generate lookup tables
uint64_t generate_lookup_table(uint64_t piece_bitboard, uint8_t piece_id, board *b) {
	if (!piece_bitboard)
		return 0ULL;

	uint64_t lookup_table = 0ULL;
	uint8_t type, color;
	type = piece_type(piece_id);
	color = piece_color(piece_id);

	switch (type) {
		case PAWN:
			lookup_table = pawn_lookup(piece_bitboard, color, b);
			break;
		case KNIGHT:
			lookup_table = knight_lookup(piece_bitboard, color, b);
			break;
		case BISHOP:
			lookup_table = bishop_lookup(piece_bitboard, color, b);
			break;
		case ROOK:
			lookup_table = rook_lookup(piece_bitboard, color, b);
			break;
		case QUEEN:
			lookup_table = queen_lookup(piece_bitboard, color, b);
			break;
		case KING:
			lookup_table = king_lookup(piece_bitboard, color, b);
			break;

		default:
			break;
	}

	return lookup_table;
}

uint64_t get_combined_lookup_table(board *b, short color) {
	uint64_t lookup_table = 0ULL;
	uint8_t cursor;
	switch (color) {
		case WHITE: {
			cursor = WHITE_PAWN_1;
			for (int i = 0; i < b->white->count.pawns; i++) {
				lookup_table |= generate_lookup_table(b->white->pawns[i], cursor, b);
				cursor += 16;
			}

			cursor = WHITE_KNIGHT_1;
			for (int i = 0; i < b->white->count.knights; i++) {
				lookup_table |= generate_lookup_table(b->white->knights[i], cursor, b);
				cursor += 16;
			}

			cursor = WHITE_BISHOP_1;
			for (int i = 0; i < b->white->count.bishops; i++) {
				lookup_table |= generate_lookup_table(b->white->bishops[i], cursor, b);
				cursor += 16;
			}

			cursor = WHITE_ROOK_1;
			for (int i = 0; i < b->white->count.rooks; i++) {
				lookup_table |= generate_lookup_table(b->white->rooks[i], cursor, b);
				cursor += 16;
			}

			cursor = WHITE_QUEEN;
			for (int i = 0; i < b->white->count.queens; i++) {
				lookup_table |= generate_lookup_table(b->white->queen[i], cursor, b);
				cursor += 16;
			}

			cursor = WHITE_KING;
			lookup_table |= generate_lookup_table(b->white->king, cursor, b);

			break;
		}
		case BLACK: {
			cursor = BLACK_PAWN_1;
			for (int i = 0; i < b->black->count.pawns; i++) {
				lookup_table |= generate_lookup_table(b->black->pawns[i], cursor, b);
				cursor += 16;
			}

			cursor = BLACK_KNIGHT_1;
			for (int i = 0; i < b->black->count.knights; i++) {
				lookup_table |= generate_lookup_table(b->black->knights[i], cursor, b);
				cursor += 16;
			}

			cursor = BLACK_BISHOP_1;
			for (int i = 0; i < b->black->count.bishops; i++) {
				lookup_table |= generate_lookup_table(b->black->bishops[i], cursor, b);
				cursor += 16;
			}

			cursor = BLACK_ROOK_1;
			for (int i = 0; i < b->black->count.rooks; i++) {
				lookup_table |= generate_lookup_table(b->black->rooks[i], cursor, b);
				cursor += 16;
			}

			cursor = BLACK_QUEEN;
			for (int i = 0; i < b->black->count.queens; i++) {
				lookup_table |= generate_lookup_table(b->black->queen[i], cursor, b);
				cursor += 16;
			}

			cursor = BLACK_KING;
			lookup_table |= generate_lookup_table(b->black->king, cursor, b);

			break;
		}
	}
	return lookup_table;
}

board *copy_board(board *b) {
	board *new_board = (board *)malloc(sizeof(board));
	if (!new_board) {
		return NULL;
	}

	// Allocate memory for white and black pieces
	new_board->white = (pieces *)malloc(sizeof(pieces));
	new_board->black = (pieces *)malloc(sizeof(pieces));
	new_board->moves = (move_stack *)malloc(sizeof(move_stack));

	init_pieces(new_board->white);
	init_pieces(new_board->black);
	init_move_stack(new_board->moves);

	push(new_board->moves, peek(*(b->moves)));

	// Copy piece counts
	new_board->white->count = b->white->count;
	new_board->black->count = b->black->count;

	// Copy the piece positions

	/*
	    Note: memcpy function
	    why memcpy?
	    The memcpy() function in C and C++ is used to copy a block of memory from one location to another.
	    Unlike other copy functions, the memcpy function copies the specified number of bytes from one memory
	    location to the other memory location regardless of the type of data stored.

	    reference:
	    1. https://dev.to/namantam1/ways-to-copy-struct-in-cc-3fl3 : This approach wouldn't work as it will copy dynamic
	       address of the board structure and changes made on simulated board will affect original board.
	    2. https://cstdspace.quora.com/How-to-copy-one-array-to-another-using-the-C-programming-language
	*/

	memcpy(new_board->white->pawns, b->white->pawns, sizeof(uint64_t) * b->white->count.pawns);
	memcpy(new_board->white->knights, b->white->knights, sizeof(uint64_t) * b->white->count.knights);
	memcpy(new_board->white->bishops, b->white->bishops, sizeof(uint64_t) * b->white->count.bishops);
	memcpy(new_board->white->rooks, b->white->rooks, sizeof(uint64_t) * b->white->count.rooks);
	memcpy(new_board->white->queen, b->white->queen, sizeof(uint64_t) * b->white->count.queens);

	memcpy(new_board->black->pawns, b->black->pawns, sizeof(uint64_t) * b->black->count.pawns);
	memcpy(new_board->black->knights, b->black->knights, sizeof(uint64_t) * b->black->count.knights);
	memcpy(new_board->black->bishops, b->black->bishops, sizeof(uint64_t) * b->black->count.bishops);
	memcpy(new_board->black->rooks, b->black->rooks, sizeof(uint64_t) * b->black->count.rooks);
	memcpy(new_board->black->queen, b->black->queen, sizeof(uint64_t) * b->black->count.queens);

	new_board->white->king = b->white->king;
	new_board->black->king = b->black->king;

	// Copy the square table
	memcpy(new_board->square_table, b->square_table, sizeof(b->square_table));

	// new_board->en_pass_pawn = b->en_pass_pawn;
	// new_board->en_passant = b->en_passant;
	new_board->castle_rights = b->castle_rights;
	memcpy(new_board->attack_tables, b->attack_tables, sizeof(b->attack_tables));

	return new_board;
}

// Checks if king of the given color is in check
bool in_check(short color, board *b) {
	if (b == NULL) return false;  // Check if board is NULL
	uint64_t king, attack_table;

	king = color == WHITE ? b->white->king : b->black->king;

	// Get opponents attack table
	attack_table = get_combined_lookup_table(b, !color);

	return king & attack_table;
}


bool move(square src, square dest, uint64_t *src_piece_ptr, uint64_t *dest_piece_ptr, short move_type, board *b) {
	if (move_type == INVALID_MOVE) {
		return false;
	}

	if (dest_piece_ptr) {
		move_type = CAPTURE_MOVE;
	}

	uint64_t move = get_bitboard(dest.file, dest.rank);


	switch (move_type) {
		case NORMAL_MOVE: {
			*src_piece_ptr = move;
			update_square_table(dest.file, dest.rank, b->square_table[src.file - 1][src.rank - 1], b);
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case CAPTURE_MOVE: {
			*src_piece_ptr = move;
			if (dest_piece_ptr) {
				*dest_piece_ptr = 0ULL;  // Clear the bitboard of the captured piece
			}
			update_square_table(dest.file, dest.rank, b->square_table[src.file - 1][src.rank - 1], b);
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case EN_PASSANT_MOVE: {
			uint8_t captured_pawn = b->square_table[dest.file - 1][src.rank - 1];
			uint64_t *captured_pawn_ptr = get_pointer_to_piece(captured_pawn, b);

			if(!captured_pawn_ptr)
				return false;

			*src_piece_ptr = move;
			*captured_pawn_ptr = 0ULL;

			update_square_table(dest.file, dest.rank, b->square_table[src.file - 1][src.rank - 1], b);  // move source pawn to destination
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b); // clear source square
			update_square_table(dest.file, src.rank, EMPTY_SQUARE, b); // clear captured pawn square
			return true;
		}
		case CASTLE_MOVE: {
			/*
				Here it is assumed that the castle rights are checked before calling this function.
			*/
			uint8_t color, piece;
			piece = b->square_table[src.file - 1][src.rank - 1];

			if(piece_type(piece) != KING)
				return false;

			color = piece_color(b->square_table[src.file - 1][src.rank - 1]);			

			/*
				king movement is sorted out in validate_castle function
				if king side castle then we have to move rook from hfile to ffile(2 squares to the left)
				if queen side castle then we have to move rook from afile to dfile(3 squares to the right)
			*/

			/*
				flow: 
				1. update king bitboard as well as square table
				2. check if its king side castle or queen side castle
				3. move rook accordingly: update bitboard and square table
			*/

			*src_piece_ptr = move;
			update_square_table(dest.file, dest.rank, piece, b);
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);

			if(dest.file == G) {
				// king side castle
				uint8_t rook = b->square_table[H - 1][src.rank - 1];
				uint64_t *rook_ptr = get_pointer_to_piece(rook, b);

				// move rook to the left side(west) by 2 squares
				*rook_ptr = move_west(*rook_ptr, color);
				*rook_ptr = move_west(*rook_ptr, color);
				update_square_table(F, src.rank, rook, b);
				update_square_table(H, src.rank, EMPTY_SQUARE, b);
			}
			else if(dest.file == C) {
				// queen side castle
				uint8_t rook = b->square_table[A - 1][src.rank - 1];
				uint64_t *rook_ptr = get_pointer_to_piece(rook, b);

				// move rook to the right side(east) by 3 squares
				*rook_ptr = move_east(*rook_ptr, color);
				*rook_ptr = move_east(*rook_ptr, color);
				*rook_ptr = move_east(*rook_ptr, color);
				update_square_table(D, src.rank, rook, b);
				update_square_table(A, src.rank, EMPTY_SQUARE, b);
			}
			return true;
		}
		case WHITE_PROMOTES_TO_KNIGHT: {
			// this new piece function allocates memory for new piece and also updates its bitboard and square table entry
			uint8_t new_knight = new_piece(WHITE, KNIGHT, move, b);
			
			// the src piece is pawn so we need to clear the pawn bitboard
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);

			return true;
		}
		case WHITE_PROMOTES_TO_BISHOP: {
			uint8_t new_bishop = new_piece(WHITE, BISHOP, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case WHITE_PROMOTES_TO_ROOK: {
			uint8_t new_rook = new_piece(WHITE, ROOK, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case WHITE_PROMOTES_TO_QUEEN: {
			uint8_t new_queen = new_piece(WHITE, QUEEN, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case BLACK_PROMOTES_TO_KNIGHT: {
			uint8_t new_knight = new_piece(BLACK, KNIGHT, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case BLACK_PROMOTES_TO_BISHOP: {
			uint8_t new_bishop = new_piece(BLACK, BISHOP, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case BLACK_PROMOTES_TO_ROOK: {
			uint8_t new_rook = new_piece(BLACK, ROOK, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		case BLACK_PROMOTES_TO_QUEEN: {
			uint8_t new_queen = new_piece(BLACK, QUEEN, move, b);
			*src_piece_ptr = 0ULL;
			update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
			return true;
		}
		default:
			break;
	}
	return false;
}

short simulate_move(square src, square dest, short turn, board *b) {
	if (b == NULL) return INVALID_MOVE;  // Check if board is NULL

	board *simulation_board = copy_board(b);
	if (simulation_board == NULL) {
		return INVALID_MOVE;  // Handle allocation failure
	}

	uint8_t piece = simulation_board->square_table[src.file - 1][src.rank - 1];
	uint8_t dest_piece = simulation_board->square_table[dest.file - 1][dest.rank - 1];

	// Validate that the piece belongs to the current turn
	if (piece_color(piece) != turn) {
		free(simulation_board->black);
		free(simulation_board->white);
		free(simulation_board);
		return INVALID_MOVE;
	}

	// Determine move type (normal move, capture, etc.)
	short move_type = (dest_piece != EMPTY_SQUARE) ? CAPTURE_MOVE : NORMAL_MOVE;

	// Get pointers to the pieces
	uint64_t *src_piece_ptr = get_pointer_to_piece(piece, simulation_board);
	uint64_t *dest_piece_ptr = (dest_piece != EMPTY_SQUARE) ? get_pointer_to_piece(dest_piece, simulation_board) : NULL;


	if (piece_type(piece) == PAWN && absolute(src.rank - dest.rank) == 1 && dest_piece == EMPTY_SQUARE) {
		Move last_move = peek(*(b->moves));
		if ((last_move.piece & 7)== PAWN && absolute(last_move.src.rank - last_move.dest.rank) == 2 && absolute(last_move.dest.file - src.file) == 1) {
			move_type = EN_PASSANT_MOVE;
		}
	}


	// check if the move is castling
	if (piece_type(piece) == KING && absolute(src.file - dest.file) == 2) {
		move_type = CASTLE_MOVE;
	}

	// Simulate the move using the move() function
	move(src, dest, src_piece_ptr, dest_piece_ptr, move_type, simulation_board);

	// Check if the move results in the king being in check
	if (in_check(turn, simulation_board)) {
		free(simulation_board->black);
		free(simulation_board->white);
		free(simulation_board);
		return 0;  // King is in check after move
	}

	// Free allocated memory for the simulation board
	free(simulation_board->black);
	free(simulation_board->white);
	free(simulation_board);

	return 1;  // Move is valid, king is not in check
}

// Ensure you only call this in a controlled manner
uint64_t generate_legal_moves(uint64_t piece_bitboard, uint8_t piece_id, short turn, board *b) {
	uint64_t legal_moves;
	square src, dest;

	legal_moves = generate_lookup_table(piece_bitboard, piece_id, b);
	src = get_square_from_bitboard(piece_bitboard);

	// Check each potential move to ensure it doesn't leave the king in check
	for (int i = 0; i < 64; i++) {
		uint64_t dest_bitboard = 1ULL << i;
		if (legal_moves & dest_bitboard) {
			dest = get_square_from_bitboard(dest_bitboard);
			if (!simulate_move(src, dest, turn, b)) {
				legal_moves &= ~dest_bitboard;  // Mask out invalid move
			}
		}
	}

	return legal_moves;
}


int get_legal_moves_from_attack_table(uint64_t generated_move, uint64_t piece, short int legal_moves_array[MAX_LEGAL_MOVES][4], int legal_moves_array_count) {
	square src_sq, dest_sq;
	uint64_t cursor = 1ULL;
	src_sq = get_square_from_bitboard(piece);
	for (int i = 0; i < 64; i++) {
		if(generated_move & cursor) {
			dest_sq = get_square_from_bitboard(cursor);
			legal_moves_array[legal_moves_array_count][0] = src_sq.file;
			legal_moves_array[legal_moves_array_count][1] = src_sq.rank;
			legal_moves_array[legal_moves_array_count][2] = dest_sq.file;
			legal_moves_array[legal_moves_array_count][3] = dest_sq.rank;
			legal_moves_array_count++;
		}
		cursor <<= 1;
	}
	return legal_moves_array_count;
}

int get_all_legal_moves(uint8_t color, board* b, short int legal_moves_array[MAX_LEGAL_MOVES][4]) {
	int legal_moves_array_count = 0;
	uint64_t generated_move;
	uint8_t cursor;
	if (color == WHITE) {
		cursor = WHITE_PAWN_1;
		for (int i = 0; i < 8; i++) {
			generated_move = generate_legal_moves(b->white->pawns[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->white->pawns[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = WHITE_KNIGHT_1;
		for (int i = 0; i < 2; i++) {
			generated_move = generate_legal_moves(b->white->knights[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->white->knights[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = WHITE_BISHOP_1;
		for (int i = 0; i < 2; i++) {
			generated_move = generate_legal_moves(b->white->bishops[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->white->bishops[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = WHITE_ROOK_1;
		for (int i = 0; i < 2; i++) {
			generated_move = generate_legal_moves(b->white->rooks[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->white->rooks[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = WHITE_QUEEN;
		generated_move = generate_legal_moves(*b->white->queen, cursor, color, b);
		legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, *b->white->queen, legal_moves_array, legal_moves_array_count);

		cursor = WHITE_KING;
		generated_move = generate_legal_moves(b->white->king, cursor, color, b);
		legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->white->king, legal_moves_array, legal_moves_array_count);
	}
	else {
		cursor = BLACK_PAWN_1;
		for (int i = 0; i < 8; i++) {
			generated_move = generate_legal_moves(b->black->pawns[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->black->pawns[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = BLACK_KNIGHT_1;
		for (int i = 0; i < 2; i++) {
			generated_move = generate_legal_moves(b->black->knights[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->black->knights[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = BLACK_BISHOP_1;
		for (int i = 0; i < 2; i++) {
			generated_move = generate_legal_moves(b->black->bishops[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->black->bishops[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = BLACK_ROOK_1;
		for (int i = 0; i < 2; i++) {
			generated_move = generate_legal_moves(b->black->rooks[i], cursor, color, b);
			legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->black->rooks[i], legal_moves_array, legal_moves_array_count);
			cursor += 16;
		}

		cursor = BLACK_QUEEN;
		generated_move = generate_legal_moves(*b->black->queen, cursor, color, b);
		legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, *b->black->queen, legal_moves_array, legal_moves_array_count);

		cursor = BLACK_KING;
		generated_move = generate_legal_moves(b->black->king, cursor, color, b);
		legal_moves_array_count = get_legal_moves_from_attack_table(generated_move, b->black->king, legal_moves_array, legal_moves_array_count);
	}
	wprintf(L"Legal moves: ");
	for(int i = 0; i < legal_moves_array_count; i++) {
		wprintf(L"%d%d%d%d ", legal_moves_array[i][0], legal_moves_array[i][1], legal_moves_array[i][2], legal_moves_array[i][3]);
	}
	wprintf(L"\n");
	wprintf(L"Legal moves: ");
	for(int i = 0; i < legal_moves_array_count; i++) {
		wprintf(L"%c%d%c%d ", legal_moves_array[i][0] + 'a' - 1, legal_moves_array[i][1], legal_moves_array[i][2] + 'a' - 1, legal_moves_array[i][3]);
	}
	wprintf(L"\n");
	return legal_moves_array_count;
}


/*
    Function to generate attack table of white and black pieces
    Attack tables are different from lookup tables. Lookup tables just give us lookup vectors for one piece but attack tables give us all the possible moves of all the pieces of a particular color.

    Attack table is generated by ORing all the legal moves of all the pieces of a particular color

    Why do we need attack tables? :- attack tables are needed to check if the king is in check or not after every move. If the king is in check then the move is invalid.
*/
uint64_t generate_attack_tables_for_color(uint8_t color, board *b) {
	uint64_t attack_table = 0ULL;
	uint8_t cursor;

	if (color == WHITE) {
		cursor = WHITE_PAWN_1;
		for (int i = 0; i < b->white->count.pawns; i++) {
			attack_table |= generate_legal_moves(b->white->pawns[i], cursor, color, b);
			cursor += 16;
		}

		cursor = WHITE_KNIGHT_1;
		for (int i = 0; i < b->white->count.knights; i++) {
			attack_table |= generate_legal_moves(b->white->knights[i], cursor, color, b);
			cursor += 16;
		}

		cursor = WHITE_BISHOP_1;
		for (int i = 0; i < b->white->count.bishops; i++) {
			attack_table |= generate_legal_moves(b->white->bishops[i], cursor, color, b);
			cursor += 16;
		}

		cursor = WHITE_ROOK_1;
		for (int i = 0; i < b->white->count.rooks; i++) {
			attack_table |= generate_legal_moves(b->white->rooks[i], cursor, color, b);
			cursor += 16;
		}

		cursor = WHITE_QUEEN;
		for (int i = 0; i < b->white->count.queens; i++){
			attack_table |= generate_legal_moves(b->white->queen[i], cursor, color, b);
			cursor += 16;
		}

		cursor = WHITE_KING;
		attack_table |= generate_legal_moves(b->white->king, cursor, color, b);
	} else {
		cursor = BLACK_PAWN_1;
		for (int i = 0; i < b->black->count.pawns; i++) {
			attack_table |= generate_legal_moves(b->black->pawns[i], cursor, color, b);
			cursor += 16;
		}

		cursor = BLACK_KNIGHT_1;
		for (int i = 0; i < b->black->count.knights; i++) {
			attack_table |= generate_legal_moves(b->black->knights[i], cursor, color, b);
			cursor += 16;
		}

		cursor = BLACK_BISHOP_1;
		for (int i = 0; i < b->black->count.bishops; i++) {
			attack_table |= generate_legal_moves(b->black->bishops[i], cursor, color, b);
			cursor += 16;
		}

		cursor = BLACK_ROOK_1;
		for (int i = 0; i < b->black->count.rooks; i++) {
			attack_table |= generate_legal_moves(b->black->rooks[i], cursor, color, b);
			cursor += 16;
		}

		cursor = BLACK_QUEEN;
		for (int i = 0; i < b->black->count.queens; i++){
			attack_table |= generate_legal_moves(b->black->queen[i], cursor, color, b);
			cursor += 16;
		}


		cursor = BLACK_KING;
		attack_table |= generate_legal_moves(b->black->king, cursor, color, b);
	}

	return attack_table;
}

void update_attack_tables(board *b, short turn) {
	if (turn == WHITE) {
		b->attack_tables[WHITE] = generate_attack_tables_for_color(WHITE, b);
		b->attack_tables[BLACK] = generate_attack_tables_for_color(BLACK, b);
	} else {
		b->attack_tables[BLACK] = generate_attack_tables_for_color(BLACK, b);
		b->attack_tables[WHITE] = generate_attack_tables_for_color(WHITE, b);
	}

	wprintf(L"Attack tables updated\nwhite: ");
	print_moves(b->attack_tables[WHITE]);

	wprintf(L"black: ");
	print_moves(b->attack_tables[BLACK]);
}

void promotion_move_menu() {
	wprintf(L"Promotion Menu\n");
	wprintf(L"1. Queen  ");
	wprintf(L"2. Rook  ");
	wprintf(L"3. Bishop  ");
	wprintf(L"4. Knight\n");
	return;
}

// function to make a move
short make_move(square src, square dest, short turn, board *b) {
	/*
	    throughout the function:
	    * piece refers to the piece that is being moved
	    * dest_piece refers to the piece that is being captured rather piece at the
	      destination square
	*/
	uint8_t piece, dest_piece, color;
	piece = b->square_table[src.file - 1][src.rank - 1];
	dest_piece = b->square_table[dest.file - 1][dest.rank - 1];
	color = piece_color(piece);


	// check if the piece is of the same color as the turn
	if (color != turn) {
		return INVALID_MOVE;
	}

	// check if the destination square is empty or not
	if (piece == EMPTY_SQUARE) {
		return INVALID_MOVE;
	}

	uint64_t piece_bitboard, _move, legal_moves, *piece_ptr, *dest_piece_ptr;
	short status;

	status = INVALID_MOVE;
	piece_ptr = get_pointer_to_piece(piece, b);
	dest_piece_ptr = NULL;

	// get bitboard of the piece
	piece_bitboard = *piece_ptr;
	legal_moves = generate_legal_moves(piece_bitboard, piece, turn, b);

	// represent a move with a bitboard
	_move = get_bitboard(dest.file, dest.rank);
	

	// check if the move is legal
	if (_move & legal_moves) {
		Move m = {
			.src = src,
			.dest = dest,
			.piece = piece,
			.captured_piece = dest_piece,
		};

		// !!: This is a caputure move
		if (dest_piece != EMPTY_SQUARE) {
			dest_piece_ptr = get_pointer_to_piece(dest_piece, b);
			status = CAPTURE_MOVE;

			b->captured_pieces[color][b->captured_pieces_count[color]] = dest_piece;
			b->captured_pieces_count[color]++;
		} else {
			dest_piece_ptr = NULL;
			status = NORMAL_MOVE;
		}

		// check if the move is enpassant
		if (piece_type(piece) == PAWN && absolute(src.rank - dest.rank) == 1 && dest_piece == EMPTY_SQUARE) {
			Move last_move = peek(*(b->moves));
			if ((last_move.piece & (uint8_t)7) == PAWN && absolute(last_move.src.rank - last_move.dest.rank) == 2 &&
			    last_move.dest.file == dest.file) {
				status = EN_PASSANT_MOVE;

				// store the id of the captured pawn
				m.captured_piece = b->square_table[dest.file - 1][src.rank - 1];

				
				b->captured_pieces[color][b->captured_pieces_count[color]] = m.captured_piece;
				b->captured_pieces_count[color]++;
			}
		}

		// check if the move is castling
		if (piece_type(piece) == KING && absolute(src.file - dest.file) == 2) {
			status = CASTLE_MOVE;
		}

		// PROMOTION MOVE
		if(piece_type(piece) == PAWN && color == WHITE && dest.rank == 8) {
			uint8_t promoted_piece_type = 0;

			promotion_move_menu();
			int choice;
			scanf("%d", &choice);
			
			switch(choice) {
				case 1:{
					status = WHITE_PROMOTES_TO_QUEEN;
					promoted_piece_type = QUEEN;
					break;
				}
				case 2:{
					status = WHITE_PROMOTES_TO_ROOK;
					promoted_piece_type = ROOK;
					break;
				}
				case 3:{
					status = WHITE_PROMOTES_TO_BISHOP;
					promoted_piece_type = BISHOP;
					break;
				}
				case 4:{
					status = WHITE_PROMOTES_TO_KNIGHT;
					promoted_piece_type = KNIGHT;
					break;
				}
				default:{
					printf("Invalid choice\n");
					return INVALID_MOVE;
				}
			}

			m.promoted_piece = promoted_piece_type == 0 ? 0 : generate_id_for_promoted_piece(color, promoted_piece_type, b);
		}
		else if(piece_type(piece) == PAWN && color == BLACK && dest.rank == 1) {
			uint8_t promoted_piece_type = 0;
			promotion_move_menu();
			int choice;
			wscanf(L"%d", &choice);

			switch(choice) {
				case 1:{
					status = BLACK_PROMOTES_TO_QUEEN;
					promoted_piece_type = QUEEN;
					break;
				}
				case 2:{
					status = BLACK_PROMOTES_TO_ROOK;
					promoted_piece_type = ROOK;
					break;
				}
				case 3:{
					status = BLACK_PROMOTES_TO_BISHOP;
					promoted_piece_type = BISHOP;	
					break;
				}
				case 4:{
					status = BLACK_PROMOTES_TO_KNIGHT;
					promoted_piece_type = KNIGHT;
					break;
				}
				default:
					printf("Invalid choice\n");
					return INVALID_MOVE;
			}			

			m.promoted_piece = promoted_piece_type == 0 ? 0 : generate_id_for_promoted_piece(color, promoted_piece_type, b);
		}

		m.castle_rights = b->castle_rights;
		m.attack_tables[0] = b->attack_tables[0];
		m.attack_tables[1] = b->attack_tables[1];

		bool move_status = move(src, dest, piece_ptr, dest_piece_ptr, status, b);
		
		if(!move_status) {
			return INVALID_MOVE;
		}

		/*
			typedef struct {
				square src;
				square dest;
				uint8_t piece;

				uint8_t captured_piece;
				uint8_t promoted_piece;
				uint8_t flags;
			} Move;			
		*/

		m.flags = status;
		push(b->moves, m);


		// do not check for castle flags if they are already set to invalid
		if((b->castle_rights & (color == WHITE ? WHITE_CASTLE_RIGHTS : BLACK_CASTLE_RIGHTS)) == 0)
			return status;

		// set the flags
		if (piece_type(piece) == KING) {
			if (color == WHITE) {
				b->castle_rights &= ~WHITE_CASTLE_RIGHTS;
			} else {
				b->castle_rights &= ~BLACK_CASTLE_RIGHTS;
			}
		}
		else if(piece_type(piece) == ROOK) {
			if (color == WHITE) {
				if (src.file == A && src.rank == 1) {
					b->castle_rights &= ~WHITE_QUEEN_SIDE_CASTLE_RIGHTS;
				} else if (src.file == H && src.rank == 1) {
					b->castle_rights &= ~WHITE_KING_SIDE_CASTLE_RIGHTS;
				}
			} else {
				if (src.file == A && src.rank == 8) {
					b->castle_rights &= ~BLACK_KING_SIDE_CASTLE_RIGHTS;
				} else if (src.file == H && src.rank == 8) {
					b->castle_rights &= ~BLACK_QUEEN_SIDE_CASTLE_RIGHTS;
				}
			}
		}

	}
	return status;
}

bool remove_memory_for_promoted_piece(uint8_t promoted_piece, board *b) {
	short *counter = get_pointer_to_piece_counter(b, promoted_piece);
	*counter -= 1;
	uint64_t *type_ptr = get_pointer_to_piece_type(piece_color(promoted_piece), QUEEN, b);
	type_ptr = (uint64_t *)realloc(type_ptr, *counter * sizeof(uint64_t));

	if(!type_ptr)
		return false;
	
	return true;
}

void undo_promotion(Move last_move, board *b) {
	// restore the pawn to the source square
	uint64_t *pawn_ptr = get_pointer_to_piece(last_move.piece, b);
	*pawn_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
	update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);

	// remove the promoted piece
	uint64_t *promoted_piece_ptr = get_pointer_to_piece(last_move.promoted_piece, b);
	*promoted_piece_ptr = 0ULL;
	update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

	// deallocate memory for the promoted piece
	remove_memory_for_promoted_piece(last_move.promoted_piece, b);

	// restore castle rights and attack tables
	b->castle_rights = last_move.castle_rights;
	b->attack_tables[0] = last_move.attack_tables[0];
	b->attack_tables[1] = last_move.attack_tables[1];
}

/* SHOULD RETURN STATUS OF UNDO MOVE */
void undo_move(board *b){
	// PENDING: check if all the essential fields are present / valid
	Move last_move = pop(b->moves);

	switch(last_move.flags){
		case NORMAL_MOVE: {
			/*
				1. restore the piece bitboard to the bitboard of the source square
				2. update the square table to reflect it
				3. restore the destination square to empty
				4. (optional) restore castle rights and attack tables
			*/
			uint64_t *piece_ptr = get_pointer_to_piece(last_move.piece, b);
			*piece_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);
			
			b->castle_rights = last_move.castle_rights;
			b->attack_tables[0] = last_move.attack_tables[0];
			b->attack_tables[1] = last_move.attack_tables[1];
			break;
		}

		case CAPTURE_MOVE: {
			/*
				1. restore the piece bitboard to the bitboard of the source square
				2. update the square table to reflect it
				3. restore the bitboard of the captured piece to the destination square
				4. update the square table to reflect it
				5. (optional) restore castle rights and attack tables
			*/
			uint64_t *piece_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *dest_piece_ptr = get_pointer_to_piece(last_move.captured_piece, b);
			
			// if missing piece then return
			if(!dest_piece_ptr)
				return;

			*piece_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);

			*dest_piece_ptr = get_bitboard(last_move.dest.file, last_move.dest.rank);
			update_square_table(last_move.dest.file, last_move.dest.rank, last_move.captured_piece, b);

			b->castle_rights = last_move.castle_rights;
			b->attack_tables[0] = last_move.attack_tables[0];
			b->attack_tables[1] = last_move.attack_tables[1];

			// PENDING: REMOVE THE CAPTURED PIECE FROM THE CAPTURED PIECES ARRAY AND DECREMENT THE COUNT
			b->captured_pieces_count[!piece_color(last_move.captured_piece)]--;
			break;
		}

		case EN_PASSANT_MOVE: {
			/*
				1. restore the piece bitboard to the bitboard of the source square
				2. update the square table to reflect it
				3. restore the destination square to empty
				4. restore bitboard of the captured pawn to the square where it was captured
				5. update the square table to reflect it
				6. (optional) restore castle rights and attack tables
			*/
			uint64_t *piece_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *dest_piece_ptr = get_pointer_to_piece(last_move.captured_piece, b);

			// if missing piece then return
			if(!dest_piece_ptr || !piece_ptr)
				return;

			*piece_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			*dest_piece_ptr = get_bitboard(last_move.dest.file, last_move.src.rank);
			update_square_table(last_move.dest.file, last_move.src.rank, last_move.captured_piece, b);

			b->castle_rights = last_move.castle_rights;
			b->attack_tables[0] = last_move.attack_tables[0];
			b->attack_tables[1] = last_move.attack_tables[1];

			// PENDING: REMOVE THE CAPTURED PIECE FROM THE CAPTURED PIECES ARRAY AND DECREMENT THE COUNT
			break;
		}

		case CASTLE_MOVE: {
			/*
				1. restore the king bitboard to the bitboard of the source square
				2. update the square table to reflect it
				3. identify the rook that was moved
				4. restore the rook bitboard to the square where it was before castling (king side / queen side)
				5. update the square table to reflect it
				6. (important) restore castle rights and attack tables
			*/
			uint64_t *king_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *rook_ptr = NULL;

			// if missing piece then return
			if(!king_ptr)
				return;

			*king_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			// king side castle
			if(last_move.dest.file == G) {
				uint8_t moved_rook = b->square_table[F - 1][last_move.src.rank - 1];
				rook_ptr = get_pointer_to_piece(moved_rook, b);
				*rook_ptr = get_bitboard(H, last_move.src.rank);
				update_square_table(H, last_move.src.rank, moved_rook, b);
				update_square_table(F, last_move.src.rank, EMPTY_SQUARE, b);
			}

			// queen side castle
			else if(last_move.dest.file == C) {
				uint8_t moved_rook = b->square_table[D - 1][last_move.src.rank - 1];
				rook_ptr = get_pointer_to_piece(moved_rook, b);
				*rook_ptr = get_bitboard(A, last_move.src.rank);
				update_square_table(A, last_move.src.rank, moved_rook, b);
				update_square_table(D, last_move.src.rank, EMPTY_SQUARE, b);
			}

			b->castle_rights = last_move.castle_rights;
			b->attack_tables[0] = last_move.attack_tables[0];
			b->attack_tables[1] = last_move.attack_tables[1];
			break;
		}

		/*
			flow for undoing promotion move:
			1. restore the pawn bitboard to the bitboard of the source square
			2. update the square table to reflect it
			3. remove the promoted piece from the board // this will involve freeing the memory allocated for the promoted piece
			4. update the square table to reflect it
			5. update the count of the promoted piece type
			5. (optional) restore castle rights and attack tables
		
			PENDING: IF THE MOVE IS PROMOTION + CAPTURE THEN WE NEED TO RESTORE THE CAPTURED PIECE AS WELL
		*/
	
		case WHITE_PROMOTES_TO_KNIGHT:
		case WHITE_PROMOTES_TO_BISHOP:
		case WHITE_PROMOTES_TO_ROOK:
		case WHITE_PROMOTES_TO_QUEEN:
		case BLACK_PROMOTES_TO_KNIGHT:
		case BLACK_PROMOTES_TO_BISHOP:
		case BLACK_PROMOTES_TO_ROOK:
		case BLACK_PROMOTES_TO_QUEEN: {
			undo_promotion(last_move, b);
			break;
		}

	}
}



/*
	SCRAP:
	case WHITE_PROMOTES_TO_KNIGHT: {
			uint64_t *pawn_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *promoted_piece_ptr = get_pointer_to_piece(last_move.promoted_piece, b);

			// if missing piece then return
			if(!pawn_ptr || !promoted_piece_ptr)
				return;

			*pawn_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);

			*promoted_piece_ptr = 0ULL;
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			b->white->count.knights--;

			b->castle_rights = last_move.castle_rights;
			b->attack_tables[0] = last_move.attack_tables[0];
			b->attack_tables[1] = last_move.attack_tables[1];
			break;
		}
		case WHITE_PROMOTES_TO_BISHOP: {
			uint64_t *pawn_ptr = get_pointer_to_piece(last_move.piece, b);
			uint64_t *promoted_piece_ptr = get_pointer_to_piece(last_move.promoted_piece, b);

			// if missing piece then return
			if(!pawn_ptr || !promoted_piece_ptr)
				return;

			*pawn_ptr = get_bitboard(last_move.src.file, last_move.src.rank);
			update_square_table(last_move.src.file, last_move.src.rank, last_move.piece, b);

			*promoted_piece_ptr = 0ULL;
			update_square_table(last_move.dest.file, last_move.dest.rank, EMPTY_SQUARE, b);

			b->white->count.bishops--;

			b->castle_rights = last_move.castle_rights;
			b->attack_tables[0] = last_move.attack_tables[0];
			b->attack_tables[1] = last_move.attack_tables[1];
			break;
		}
		case WHITE_PROMOTES_TO_ROOK: {
			
			break;
		}
		case WHITE_PROMOTES_TO_QUEEN: {
			
			break;
		}
*/