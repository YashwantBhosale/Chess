#include <stdbool.h>
#include <stdint.h>
#include "chessboard.h"

uint64_t generate_pawn_attacks(uint8_t pawn_id, uint64_t pawn_position, board *b);
uint64_t generate_bishop_attacks(uint8_t bishop_id, uint64_t bishop_position, board *b);
short make_move(square src, square dest, short turn, board *b, bool is_engine);
short unmake_move(board *b);
void update_attacks(board *b);
void update_attacks_for_color(board *b, short color);
void update_type_board(board *b, short turn);
void filter_legal_moves(board *b, short turn);
int lookup_index(uint8_t id);
void adjust_type_board_for_make_move(Move move, board *b);
void adjust_type_board_for_unmake_move(Move move, board *b);
uint8_t piece_type_from_promotion_flag(uint8_t flag);
uint8_t get_id_of_promoted_piece(uint8_t piece_type, short color, short piece_number);
bool in_check(short color, board *b);
bool in_check_alt(short color, board *b);
void filter_legal_moves_alt(board *b, short turn);