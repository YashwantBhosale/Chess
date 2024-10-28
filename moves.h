#include <stdint.h>

#define NORMAL_MOVE 0
#define CAPTURE_MOVE 1
#define CASTLE_MOVE 2
#define EN_PASSANT_MOVE 3
#define PROMOTION_MOVE 4
#define CHECK_MOVE 5
#define CHECKMATE_MOVE 6
#define STALEMATE_MOVE 7
#define INVALID_MOVE -1

/* Piece types */
enum { PAWN = 1,
	   KNIGHT = 2,
	   BISHOP = 3,
	   ROOK = 4,
	   QUEEN = 5,
	   KING = 6 };

uint64_t generate_lookup_table(uint64_t piece_bitboard, uint8_t piece_id, board *b);
void update_attack_tables(board *b, short turn);
short make_move(square src, square dest, short turn, board *board);
board* copy_board(board *b);
