typedef struct {
    double evaluation;
    Move best_move;
} evaluated_move;

/*
typedef struct {
	square src;
	square dest;
	uint8_t piece;

	uint8_t captured_piece;
	uint8_t promoted_piece;
	uint64_t en_passant_square;

	uint8_t castle_rights;
	uint8_t type;
} Move;
*/

#define PLACEHOLDER_MOVE (Move) {{0, 0}, {0, 0}, 0, 0, 0, 0, 0, 0}

evaluated_move minimax(board *b, int depth, short maximizing_player, double alpha, double beta); 