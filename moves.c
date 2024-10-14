#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "chessboard.h"
#include "moves.h"
#include "move_stack.h"

/* Piece types */
enum { PAWN = 1,
	   KNIGHT = 2,
	   BISHOP = 3,
	   ROOK = 4,
	   QUEEN = 5,
	   KING = 6 };

/* Helper functions */
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

uint64_t rankmask(int rank) {
    return 0xFFULL << (8 * (rank - 1));
}

uint64_t filemask(int file) {
    return 0x0101010101010101ULL << (file - 1);
}

/* This function returns pointer to the piece bitboard with argument as piece is */
uint64_t *get_pointer_to_piece(uint8_t piece_id, board *b) {
	uint8_t type, color, index;
	type = piece_id & 7;            // last 3 bits represent type
	color = (piece_id & 8) >> 3;    // 4th bit represents color
	index = (piece_id & 112) >> 4;  // 5th, 6th and 7th bits represent index

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
    Make sure that your piece is not stepping on a friendly piece
    PENDING : Simulate the move and check if the king is in check
*/

// Checks if king of the given color is in check
bool in_check(short color, board *b) {
	uint64_t king, attack_table;

	king = color == WHITE ? b->white->king : b->black->king;

	// Get opponents attack table
	attack_table = color == WHITE ? b->attack_tables[BLACK] : b->attack_tables[WHITE];

	return king & attack_table;
}

short simulate_move(square src, square dest, board *b) {
    uint8_t piece_id, dest_piece_id;
    uint64_t *piece, *dest_piece, dest_bitboard;
    short color;
    bool is_valid_move = false;
    
    // Backup current board state and attack tables
    uint64_t old_white_at = b->attack_tables[WHITE];
    uint64_t old_black_at = b->attack_tables[BLACK];

    // Backup pieces
    piece_id = b->square_table[src.file - 1][src.rank - 1];
    piece = get_pointer_to_piece(piece_id, b);
    color = piece_id & 8 ? BLACK : WHITE;
    
    dest_piece_id = b->square_table[dest.file - 1][dest.rank - 1];
    dest_piece = dest_piece_id != EMPTY_SQUARE ? get_pointer_to_piece(dest_piece_id, b) : NULL;
    dest_bitboard = get_bitboard(dest.file, dest.rank);
    
    // Make the move on the board
    *piece = dest_bitboard;
    update_square_table(dest.file, dest.rank, piece_id, b);
    update_square_table(src.file, src.rank, EMPTY_SQUARE, b);
    
    if (dest_piece) {
        *dest_piece = 0ULL;
        // Manually update the attack tables to reflect the capture
        b->attack_tables[color == WHITE ? BLACK : WHITE] |= dest_bitboard;
        b->attack_tables[color] &= ~dest_bitboard;
    }
    
    // Push the move to stack
    Move m = {src, dest, piece_id, dest_piece_id, 0};
    push(&moves, m);
    
    // Check if this move results in check
    if (!in_check(color, b)) {
        is_valid_move = true;
    }

    // Undo the move to restore the board state
    *piece = get_bitboard(src.file, src.rank);
    update_square_table(src.file, src.rank, piece_id, b);
    update_square_table(dest.file, dest.rank, dest_piece_id, b);
    
    if (dest_piece) {
        *dest_piece = dest_bitboard;
    }
    
    // Restore the attack tables
    b->attack_tables[WHITE] = old_white_at;
    b->attack_tables[BLACK] = old_black_at;
    
    // Pop the move stack
    pop(&moves);
    
    return is_valid_move;
}


uint64_t validate_move(uint64_t move, uint64_t pb, uint64_t ob) {
	return move & ~pb;
}

/* Make sure that your piece is capturing an enemy piece */
uint64_t validate_capture(uint64_t move, uint64_t pb, uint64_t ob) {
	return move & ob;
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
    if(move & filemask(1)){
        return 0ULL;
    }

	return move >> 1;
}

// function to generate right direction move
uint64_t move_east(uint64_t move, short color) {
    // if the move is on the rightmost file then it should not be allowed to move right
    if(move & filemask(8)){
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
            if(pawn & rankmask(2)){
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
			break;
		}
        case BLACK: {
            move = move_south(pawn, color);
            move = validate_move(move, player_board | opp_board, 0ULL);
            lookup |= move;
            
            if(pawn & rankmask(7)){
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

	return lookup;
}

/* Function to generate lookup tables */
uint64_t generate_lookup_table(uint64_t piece_bitboard, uint8_t piece_id, board *b) {
	uint64_t lookup_table = 0ULL;
	uint8_t type, color;
	type = piece_id & 7;
	color = (piece_id & 8) ? BLACK : WHITE;
    
    // wprintf(L"============================================================\n");
    // wprintf(L"This is lookup table for %s\n", color ? "BLACK" : "WHITE");
	switch (type) {
		case PAWN:
			lookup_table = pawn_lookup(piece_bitboard, color, b);
            // wprintf(L"pawn lookup table = \n");
            // print_legal_moves(lookup_table);
			break;
		case KNIGHT:
			lookup_table = knight_lookup(piece_bitboard, color, b);
            // wprintf(L"knight lookup table = %llu\n", lookup_table); 
            // print_legal_moves(lookup_table);          
			break;
		case BISHOP:
			lookup_table = bishop_lookup(piece_bitboard, color, b);
            // wprintf(L"bishop lookup table = %llu\n", lookup_table);
            // print_legal_moves(lookup_table);
			break;
		case ROOK:
			lookup_table = rook_lookup(piece_bitboard, color, b);
            // wprintf(L"rook lookup table = %llu\n", lookup_table);
            // print_legal_moves(lookup_table);
			break;
		case QUEEN:
			lookup_table = queen_lookup(piece_bitboard, color, b);
            // wprintf(L"queen lookup table = %llu\n", lookup_table);
            // print_legal_moves(lookup_table);
			break;
		case KING:
			lookup_table = king_lookup(piece_bitboard, color, b);
            // wprintf(L"king lookup table = %llu\n", lookup_table);
            // print_legal_moves(lookup_table);
			break;

		default:
			break;
	}

	return lookup_table;
}

/* Function to generate legal moves */
uint64_t generate_legal_moves(uint64_t piece_bitboard, uint8_t piece_id, board *b) {
    uint64_t legal_moves;
    square src, dest;
    
    legal_moves = generate_lookup_table(piece_bitboard, piece_id, b);
    src = get_square(piece_bitboard);

    // Check each potential move to ensure it doesn't leave the king in check
    for (int i = 0; i < 64; i++) {
        uint64_t dest_bitboard = 1ULL << i;
        if (legal_moves & dest_bitboard) {
            dest = get_square(dest_bitboard);
            if (!simulate_move(src, dest, b)) {
                legal_moves &= ~dest_bitboard; // Mask out invalid move
            }
        }
    }

    return legal_moves;
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
        for (int i = 0; i < 8; i++) {
            attack_table |= generate_legal_moves(b->white->pawns[i], cursor, b);
            cursor += 16;
        }

        cursor = WHITE_KNIGHT_1;
        for (int i = 0; i < 2; i++) {
            attack_table |= generate_legal_moves(b->white->knights[i], cursor, b);
            cursor += 16;
        }

        cursor = WHITE_BISHOP_1;
        for (int i = 0; i < 2; i++) {
            attack_table |= generate_legal_moves(b->white->bishops[i], cursor, b);
            cursor += 16;
        }

        cursor = WHITE_ROOK_1;
        for (int i = 0; i < 2; i++) {
            attack_table |= generate_legal_moves(b->white->rooks[i], cursor, b);
            cursor += 16;
        }

        cursor = WHITE_QUEEN;
        attack_table |= generate_legal_moves(*b->white->queen, cursor, b);

        cursor = WHITE_KING;
        attack_table |= generate_legal_moves(b->white->king, cursor, b);
    } else {
        cursor = BLACK_PAWN_1;
        for (int i = 0; i < 8; i++) {
            attack_table |= generate_legal_moves(b->black->pawns[i], cursor, b);
            cursor += 16;
        }

        cursor = BLACK_KNIGHT_1;
        for (int i = 0; i < 2; i++) {
            attack_table |= generate_legal_moves(b->black->knights[i], cursor, b);
            cursor += 16;
        }

        cursor = BLACK_BISHOP_1;
        for (int i = 0; i < 2; i++) {
            attack_table |= generate_legal_moves(b->black->bishops[i], cursor, b);
            cursor += 16;
        }

        cursor = BLACK_ROOK_1;
        for (int i = 0; i < 2; i++) {
            attack_table |= generate_legal_moves(b->black->rooks[i], cursor, b);
            cursor += 16;
        }

        cursor = BLACK_QUEEN;
        attack_table |= generate_legal_moves(*b->black->queen, cursor, b);

        cursor = BLACK_KING;
        attack_table |= generate_legal_moves(b->black->king, cursor, b);
    }

    return attack_table;
}

void update_attack_tables(board *b, short turn) {
	if(turn == WHITE) {
		b->attack_tables[WHITE] = generate_attack_tables_for_color(WHITE, b);
		b->attack_tables[BLACK] = generate_attack_tables_for_color(BLACK, b);
	} else {
		b->attack_tables[BLACK] = generate_attack_tables_for_color(BLACK, b);
		b->attack_tables[WHITE] = generate_attack_tables_for_color(WHITE, b);
	}
	/*
	uint64_t old_white_at, old_black_at, new_white_at, new_black_at;
	old_white_at = b->attack_tables[WHITE];
	old_black_at = b->attack_tables[BLACK];

	wprintf(L"is white in check? %d\n", in_check(WHITE, b));
	wprintf(L"is black in check? %d\n", in_check(BLACK, b));

	wprintf(L"Attack tables updated: \n");
	wprintf(L"white : ");
	new_white_at = generate_attack_tables_for_color(WHITE, b);
	print_legal_moves(new_white_at);

	wprintf(L"black : ");
	new_black_at = generate_attack_tables_for_color(BLACK, b);
	print_legal_moves(new_black_at);

	b->attack_tables[WHITE] = new_white_at;
	b->attack_tables[BLACK] = new_black_at;
	wprintf(L"============================================================\n");	
	*/
}


/* Function to make a move */
short make_move(square src, square dest, board *board) {
	if (board->square_table[src.file - 1][src.rank - 1] == EMPTY_SQUARE) {
		wprintf(L"Function returned because source square is empty\n");
		return INVALID_MOVE;
	}

	if (board->attack_tables[WHITE] == 0ULL || board->attack_tables[BLACK] == 0ULL) {
		wprintf(L"Function returned from make move\n");
		return CHECKMATE_MOVE;
	}

	uint64_t piece_bitboard, move, legal_moves, *piece_ptr, *dest_piece_ptr;  // lookup_table,
	uint8_t piece, dest_piece;
	short status, color;
	status = INVALID_MOVE;

	piece = board->square_table[src.file - 1][src.rank - 1];
	piece_ptr = get_pointer_to_piece(piece, board);

	piece_bitboard = get_bitboard(src.file, src.rank);
	// lookup_table = generate_lookup_table(piece_bitboard, piece, board);
	legal_moves = generate_legal_moves(piece_bitboard, piece, board);

	move = get_bitboard(dest.file, dest.rank);
	if (move & legal_moves) {
		dest_piece = board->square_table[dest.file - 1][dest.rank - 1];
		if (dest_piece != EMPTY_SQUARE) {
			dest_piece_ptr = get_pointer_to_piece(dest_piece, board);
			*dest_piece_ptr = 0ULL;
			status = CAPTURE_MOVE;
		} else {
			status = NORMAL_MOVE;
		}

		*piece_ptr = move;
		update_square_table(dest.file, dest.rank, piece, board);
		update_square_table(src.file, src.rank, EMPTY_SQUARE, board);
		Move m = {src, dest, piece, board->square_table[dest.file - 1][dest.rank - 1], 0};
		push(&moves, m);

		color = piece & 8 ? BLACK : WHITE;
		wprintf(L"Color in make move: %d\n", color);
		if (in_check((color ? WHITE : BLACK), board)) {
			status = CHECK_MOVE;
		}

		// if(legal_moves == 0ULL) {
		//     status = in_check(!color, board) ? CHECKMATE_MOVE : STALEMATE_MOVE;
		// }
		// return status;
	} else {
		status = INVALID_MOVE;
	}

	return status;
}
