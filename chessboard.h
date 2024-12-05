#ifndef CHESSBOARD_H
#define CHESSBOARD_H
#include <stdint.h>


// #include "move_list.h"
/*
COLORS:
In general, for all functions white is enumerated as 0 and black is enumerated
as 1.
*/
enum colors {
	WHITE = 0,
	BLACK = 1
};

/*
FILES:
Throughout the code files are represented by numbers. To improve readability,
they are enumerated so that we can understand that we are referring to file
on the chessboard.
*/
enum Files {
	A = 1,
	B = 2,
	C = 3,
	D = 4,
	E = 5,
	F = 6,
	G = 7,
	H = 8
};

/*
    PIECE IDs:
    Why piece ids?
    The design of bitboards is piece centric, which means that the piece knows
    its position on the board. But whenever we place a piece on the board, we
    often need to know which piece is at which square. So a design which may
    not be most efficient but is most intuitive is to have a piece id which
    is a 8 bit number, with obviously special format which will allow us to
    keep track of what piece is at what square. Most significant use of this
    design is when a particular piece needs iformation about the piece on some
    other square.

    piece ids themselves aren't very very useful they are used with the square
    table to get the piece on a particular square.

    Potential issue:
    After pawn promotion, we may have more pieces whose ids are not defined here.
    so our code needs to be robust enough to handle that without creating any
    issues.

    format : PNNN CTTT
    C -> piece color : 0 -> white, 1 -> black, P-> promoted
    TTT -> piece type :
    000 -> no piece
    001 -> pawn
    010 -> knight
    011 -> bishop
    100 -> rook
    101 -> queen
    110 -> king
    NNN -> piece number : 000 -> 1, ..., 111 -> 8
    01101000
    white:               black:
    0000 0001 -> pawn    0000 1001 -> pawn
    0001 0001 -> pawn    0001 1001 -> pawn
    0010 0001 -> pawn    0010 1001 -> pawn
    0011 0001 -> pawn    0011 1001 -> pawn
    0100 0001 -> pawn    0100 1001 -> pawn
    0101 0001 -> pawn    0101 1001 -> pawn
    0110 0001 -> pawn    0110 1001 -> pawn
    0111 0001 -> pawn    0111 1001 -> pawn

    0000 0010 -> knight  0000 1010 -> black knight
    0001 0010 -> knight  0001 1010 -> black knight

    0000 0011 -> bishop  0000 1011 -> black bishop
    0001 0011 -> bishop  0001 1011 -> black bishop

    0000 0100 -> rook    0000 1100 -> black rook
    0001 0100 -> rook    0001 1100 -> black rook

    0000 0101 -> queen   0000 1101 -> black queen

    0000 0110 -> king    0000 1110 -> black king
*/

// piece ids
#define EMPTY_SQUARE 0
// White pieces
#define WHITE_PAWN_1 1
#define WHITE_PAWN_2 17
#define WHITE_PAWN_3 33
#define WHITE_PAWN_4 49
#define WHITE_PAWN_5 65
#define WHITE_PAWN_6 81
#define WHITE_PAWN_7 97
#define WHITE_PAWN_8 113

#define WHITE_KNIGHT_1 2
#define WHITE_KNIGHT_2 18

#define WHITE_BISHOP_1 3
#define WHITE_BISHOP_2 19

#define WHITE_ROOK_1 4
#define WHITE_ROOK_2 20

#define WHITE_QUEEN 5
#define WHITE_KING 6

// Black pieces
#define BLACK_PAWN_1 9
#define BLACK_PAWN_2 25
#define BLACK_PAWN_3 41
#define BLACK_PAWN_4 57
#define BLACK_PAWN_5 73
#define BLACK_PAWN_6 89
#define BLACK_PAWN_7 105
#define BLACK_PAWN_8 121

#define BLACK_KNIGHT_1 10
#define BLACK_KNIGHT_2 26

#define BLACK_BISHOP_1 11
#define BLACK_BISHOP_2 27

#define BLACK_ROOK_1 12
#define BLACK_ROOK_2 28

#define BLACK_QUEEN 13
#define BLACK_KING 14

// piece types
enum { PAWN = 1,
	   KNIGHT = 2,
	   BISHOP = 3,
	   ROOK = 4,
	   QUEEN = 5,
	   KING = 6 };


// Please refer to the moves.c validate_castle function for the explanation of the castle rights
#define WHITE_CASTLE_RIGHTS 0b00000111
#define BLACK_CASTLE_RIGHTS 0b01110000

#define WHITE_KING_SIDE_CASTLE_RIGHTS 0b00000101
#define WHITE_QUEEN_SIDE_CASTLE_RIGHTS 0b00000110

#define BLACK_KING_SIDE_CASTLE_RIGHTS 0b01010000
#define BLACK_QUEEN_SIDE_CASTLE_RIGHTS 0b01100000
/*
    Structure for pieces:
    * Design:
    each type of piece is mostly an array of bitboards. This array is dynamically
    allocated so that we can have a variable number of pieces of a particular type.
    This is in alignment with the pawn promotion rule in chess.

    * purpose:
    The purpose of defining structure for pieces is because we have two types of
    pieces, black and white. So instead of defining them seperately it is convenient
    to define them in a single structure.
*/
typedef struct move_stack move_stack;

struct MoveList;

typedef struct {
	// keep count of the number of pieces of each type
	short pawns, knights, bishops, rooks, queens;
} piece_count;

typedef struct pieces {
	uint64_t *pawns;    // array of 8 pawns
	uint64_t *knights;  // array of 2 knights
	uint64_t *bishops;  // array of 2 bishops
	uint64_t *rooks;    // array of 2 rooks
	uint64_t *queen;    // array of 1 queen
	uint64_t king;      // 1 king
	piece_count count;
} pieces;

/*
    Structure for global board state:
    * Design:
    1. there are two malloced pieces structures, one for white and one for black.
    2. there is square table to keep track of what piece is on a particular square.
    ... More to be added as we progress. (like castling rights, en passant square etc.)
*/

// structure for a square
typedef struct square {
	uint8_t rank;
	uint8_t file;
} square;

typedef struct {
	pieces *white, *black;
	uint8_t square_table[8][8];

	uint8_t castle_rights; // XBBBXWWW

	uint8_t captured_pieces[2][16];  // captured_pieces[WHITE], captured_piece[BLACK]
	short captured_pieces_count[2];

    uint64_t en_passant_square;
    uint64_t white_board;
    uint64_t black_board;

    move_stack *moves;
    struct MoveList *white_attacks; // Pseudo legal moves
    struct MoveList *black_attacks; // Pseudo legal moves

    struct MoveList *white_legal_moves; // Legal moves
    struct MoveList *black_legal_moves; // Legal moves

    /*
        lookup table will contain the attacks of  particular color.
        the first index will wolways contain the combined attacks of all the pieces.
        the next 6 indices will contain the attacks of each piece type.

        The lookup table is used to check if a move is valid or not.
    */
    uint64_t white_lookup_table[97];
    uint64_t black_lookup_table[97];
} board;


// functions for chessboard
void init_board(board *b);
void init_pieces(pieces *type);
void print_board(board *b, short turn);
void load_fen(board *b, char *fen);


// Helper functions
uint8_t piece_color(uint8_t piece_id);
uint8_t piece_type(uint8_t piece_id);            
uint8_t piece_number(uint8_t piece_id);          
void get_rank_and_file_from_bitboard(uint64_t bitboard, int *file, int *rank);
square get_square_from_bitboard(uint64_t bitboard);
uint64_t get_bitboard(uint8_t file, uint8_t rank);
uint8_t generate_id_for_promoted_piece(uint8_t piece_type, short color, board *b);
short *get_pointer_to_piece_counter(board *b, uint8_t piece_id);
uint64_t **get_pointer_to_piece_type(short color, uint8_t piece_type, board *b);
void update_square_table(int file, int rank, uint8_t piece, board *b);
uint64_t white_board(board *b);
uint64_t black_board(board *b);
void print_square_from_bitboard(uint64_t bitboard);
void print_moves(uint64_t moves);

#endif
