#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
COLORS:

In general, for all functions white is enumerated as 0
and black is enumerated as 1.
*/
enum colors {
	WHITE = 0,
	BLACK = 1
};

/*
FILES:

Throughout the code files are represented by numbers.
To improve readability, they are enumerated so that we can
understand that we are referring to file on the chessboard.
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

WHY PIECE IDs?
The design of bitboards is piece centric, which means that the piece knows
its position on the board. But whenever we place a piece on the board, we
often need to know which piece is at which square. So a design which may not
be most efficient but is most intuitive is to have a piece id which is a 8 bit
number, with obviously special format which will allow us to keep track of
what piece is at what square. Most significant use of this design is when a
particular piece needs information about the piece on some other square.

PIECE IDs themselves aren't very useful
They are used with the square table to get the piece on a particular square.

POTENTIAL ISSUE: After pawn promotion, we may have more pieces whose IDs are not defined
here. So our code needs to be robust enough to handle that without creating any issues.

FORMAT: XNNN CTTT
C -> PIECE COLOR
	0 -> white
	1 -> black

TTT -> PIECE TYPE
	000 -> NO PIECE
	001 -> PAWN
	010 -> KNIGHT
	011 -> BISHOP
	100 -> ROOK
	101 -> QUEEN
	110 -> KING

NNN -> PIECE NUMBER
	000 -> 1
	001 -> 2
	...
	111 -> 8

WHITE				 BLACK
0000 0001 -> PAWN	 0000 1001 -> PAWN
0001 0001 -> PAWN	 0001 1001 -> PAWN
0010 0001 -> PAWN	 0010 1001 -> PAWN
0011 0001 -> PAWN	 0011 1001 -> PAWN
0100 0001 -> PAWN	 0100 1001 -> PAWN
0101 0001 -> PAWN	 0101 1001 -> PAWN
0110 0001 -> PAWN	 0110 1001 -> PAWN
0111 0001 -> PAWN	 0111 1001 -> PAWN

0000 0010 -> KNIGHT	 0000 1010 -> KNIGHT
0001 0010 -> KNIGHT	 0001 1010 -> KNIGHT

0000 0011 -> BISHOP	 0000 1011 -> BISHOP
0001 0011 -> BISHOP	 0001 1011 -> BISHOP

0000 0100 -> ROOK	 0000 1100 -> ROOK
0001 0100 -> ROOK	 0001 1100 -> ROOK

0000 0101 -> QUEEN	 0000 1101 -> QUEEN

0000 0110 -> KING	 0000 1110 -> KING
*/

/* PIECE IDS */
#define EMPTY_SQUARE 0

/* WHITE PIECES */
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

/* BLACK PIECES */
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


/* Please refer to the "moves.c" file's "validate_castle()" function for the explanation of the castle rights */
#define WHITE_CASTLE_RIGHTS 0b00000111
#define BLACK_CASTLE_RIGHTS 0b01110000

#define WHITE_KING_SIDE_CASTLE_RIGHTS 0b00000101
#define WHITE_QUEEN_SIDE_CASTLE_RIGHTS 0b00000110

#define BLACK_KING_SIDE_CASTLE_RIGHTS 0b01010000
#define BLACK_QUEEN_SIDE_CASTLE_RIGHTS 0b01100000

/* Maximum number of legal moves from a given position */
#define MAX_LEGAL_MOVES 218

enum {
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

/*
Structure for Pieces:

- Design:
Each type of piece is mostly an array of bitboards.
This array is dynamically allocated so that we can
have a variable number of pieces of a particular type.
This is in alignment with the pawn promotion rule in chess.

- Purpose:
The purpose of defining structure for pieces is because
we have two types of pieces, black and white. So instead
of defining them seperately it is convenient to define them
in a single structure.
*/
typedef struct {
	/* keep count of the number of pieces of each type */
	short pawns, knights, bishops, rooks, queens;
} piece_count;

typedef struct pieces {
	uint64_t *pawns; /* array of 8 pawns */
	uint64_t *knights; /* array of 2 knights */
	uint64_t *bishops; /* array of 2 bishops */
	uint64_t *rooks; /* array of 2 rooks */
	uint64_t *queen; /* array of 1 queen */
	uint64_t king; /* 1 king */
	piece_count count;
} pieces;

/*
Structure for Global Board State:

- Design:
	1. there are two malloced pieces structures, one for WHITE and one for BLACK.
	2. there is square table to keep track of what piece is on a particular square.
	... More to be added as we progress. (like castling rights, en passant square etc.)
*/
typedef struct {
	pieces *white, *black;
	uint8_t square_table[8][8];
	short en_pass_pawn;
	uint64_t en_passant;
	uint8_t castle_rights;
	uint64_t attack_tables[2];
} board;

/* Structure for a Square */
typedef struct {
	uint8_t rank;
	uint8_t file;
} square;

/* Piece IDs Functions */

uint8_t piece_color(uint8_t piece_id);
uint8_t piece_type(uint8_t piece_id);
uint8_t piece_number(uint8_t piece_id);

/* Utility Functions */

uint64_t get_bitboard(uint8_t file, uint8_t rank);
void get_rank_and_file_from_bitboard(uint64_t bitboard, int *file, int *rank);
square get_square_from_bitboard(uint64_t bitboard);
uint64_t get_type_board(pieces *type, board *b);
uint64_t white_board(board *b);
uint64_t black_board(board *b);
void print_square_from_bitboard(uint64_t bitboard);
void print_moves(uint64_t moves);
short *get_pointer_to_piece_counter(board *b, uint8_t piece_id);
uint64_t *get_pointer_to_piece_type(short color, uint8_t piece_type, board *b);


/* Square Table Functions */

void init_square_table(board *b);
void update_square_table(int file, int rank, uint8_t piece, board *b);


/* Chessboard Functions */
void init_pieces(pieces *type);
void set_piece_counts(board *b);
void place_piece(uint64_t *piece_board, int file, int rank, uint8_t piece, board *b);
void set_pieces(board *b);
void init_board(board *board); // initializes the board
void print_board(board *board, short turn); // prints the board
uint8_t new_piece(short color, uint8_t piece_type, uint64_t position_bb, board *b); // for pawn promotion
void free_board(board* board); // free the memory allocated for the board