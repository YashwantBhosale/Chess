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
    switch(color) {
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

uint64_t *get_pointer_to_piece(uint8_t piece_id, board *b) {
    uint8_t type, color, index;
    type = piece_id & 7; // last 3 bits represent type
    color = (piece_id & 8) >> 3; // 4th bit represents color
    index = (piece_id & 112) >> 4; // 5th, 6th and 7th bits represent index

    switch(type) {
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
    return color == WHITE ? move << 8 : move >> 8;
}

// function to generate backward direction move
uint64_t move_south(uint64_t move, short color) {
    return color == WHITE ? move >> 8 : move << 8;
}

// function to generate left direction move
uint64_t move_west(uint64_t move, short color) {
    return color == WHITE ? move << 1 : move >> 1;
}

// function to generate right direction move
uint64_t move_east(uint64_t move, short color) {
    return color == WHITE ? move >> 1 : move << 1;
}

// function to generate north-east direction move
uint64_t move_north_east(uint64_t move, short color) {
    return color == WHITE ? move << 9 : move >> 9;
}

// function to generate north-west direction move
uint64_t move_north_west(uint64_t move, short color) {
    return color == WHITE ? move << 7 : move >> 7;
}

// function to generate south-east direction move
uint64_t move_south_east(uint64_t move, short color) {
    return color == WHITE ? move >> 7 : move << 7;
}

// function to generate south-west direction move
uint64_t move_south_west(uint64_t move, short color) {
    return color == WHITE ? move >> 9 : move << 9;
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

    /* 1. check if pawn can move forward */
    move = move_north(pawn, color);
    
    /* IMPORTANT: AS WE CONSIDER PIECES OF BOTH COLORS AS NON-CAPTURABLE PIECES IN CASE OF FORWARD MOVES OF PAWN, WE PASS COMBINED BOARD OF PLAYER AND OPPONENT AS PLAYER BOARD */
    move = validate_move(move, player_board | opp_board, 0ULL); 
    lookup |= move;

    /* 2. check if its pawn's first move */
    move = move_north(move, color);
    move = validate_move(move, player_board | opp_board, 0ULL);
    lookup |= move;

    /* 3. check if pawn can capture enemy piece on two diagonals */
    move = move_north_east(pawn, color);
    move = validate_capture(move, player_board, opp_board);
    lookup |= move;

    move = move_north_west(pawn, color);
    move = validate_capture(move, player_board, opp_board);
    lookup |= move;

    /*
        PENDING: 1. Enpassant, 2. Promotion
    */

    return lookup;
}

uint64_t knight_lookup(uint64_t knight, short color, board *b){
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
    while(cursor) {
        move = move_north_east(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if bishop can capture enemy piece then break
            break;
        
    }

    /* 2. check if bishop can move north-west */
    cursor = bishop;
    while(cursor) {
        move = move_north_west(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if bishop can capture enemy piece then break
            break; 
    }

    /* 3. check if bishop can move south-east */
    cursor = bishop;
    while(cursor) {
        move = move_south_east(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if bishop can capture enemy piece then break
            break;
        
    }

    /* 4. check if bishop can move south-west */
    cursor = bishop;
    while(cursor) {
        move = move_south_west(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if bishop can capture enemy piece then break
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
    while(cursor) {
        move = move_north(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if rook can capture enemy piece then break
            break;
        
    }

    /* 2. check if rook can move south */
    cursor = rook;
    while(cursor) {
        move = move_south(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if rook can capture enemy piece then break
            break;
        
    }

    /* 3. check if rook can move east */
    cursor = rook;
    while(cursor) {
        move = move_east(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if rook can capture enemy piece then break
            break;
        
    }

    /* 4. check if rook can move west */
    cursor = rook;
    while(cursor) {
        move = move_west(cursor, color);
        move = validate_move(move, player_board, opp_board);
        lookup |= move; // update the lookup table with the new move
        cursor = move; // update the cursor to the new position

        if(move & opp_board) // if rook can capture enemy piece then break
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
    color = (piece_id & 8) >> 3;
    
    switch (type){
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

/* Function to make a move */
short make_move(square src, square dest, board *board) {
    uint64_t piece_bitboard, move, lookup_table, *piece_ptr;
    uint8_t piece;
    piece = board->square_table[src.file-1][src.rank-1];
    piece_ptr = get_pointer_to_piece(piece, board);

    piece_bitboard = get_bitboard(src.file, src.rank);
    lookup_table = generate_lookup_table(piece_bitboard, piece, board);

    move = get_bitboard(dest.file, dest.rank);
    if(move & lookup_table) {
        *piece_ptr = move;
        update_square_table(dest.file, dest.rank, piece, board);
        update_square_table(src.file, src.rank, EMPTY_SQUARE, board);
        Move m = {src, dest, piece, board->square_table[dest.file-1][dest.rank-1], 0};
        push(&moves, m);
        return 1;
    }

    return 0;
}