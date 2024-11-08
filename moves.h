#include <stdint.h>
#include <stdbool.h>

#define NORMAL_MOVE 0
#define CAPTURE_MOVE 1
#define CASTLE_MOVE 2
#define EN_PASSANT_MOVE 3
#define WHITE_PROMOTION_MOVE 4 // 0000 0100
#define BLACK_PROMOTION1_MOVE 2 // 0000 1100

#define WHITE_PROMOTES_TO_KNIGHT 0b00010100
#define WHITE_PROMOTES_TO_BISHOP 0b00100100
#define WHITE_PROMOTES_TO_ROOK 0b00110100
#define WHITE_PROMOTES_TO_QUEEN 0b01000100

#define BLACK_PROMOTES_TO_KNIGHT 0b00011100
#define BLACK_PROMOTES_TO_BISHOP 0b00101100
#define BLACK_PROMOTES_TO_ROOK 0b00111100
#define BLACK_PROMOTES_TO_QUEEN 0b01001100

#define PROMOTION_MOVE_MASK 0b00001111

#define CHECK_MOVE 5
#define CHECKMATE_MOVE 6
#define STALEMATE_MOVE 7
#define INVALID_MOVE -1


/* Utility Functions */

uint64_t rankmask(int rank);
uint64_t filemask(int file);
unsigned int absolute(int x);
void get_player_board_and_opp_board(short color, board *b, uint64_t *pb, uint64_t *ob);
uint64_t *get_pointer_to_piece(uint8_t piece_id, board *b);
board *copy_board(board *b);
bool in_check(short color, board *b);


/* Move Generation Functions */

uint64_t move_north(uint64_t move, short color);
uint64_t move_south(uint64_t move, short color);
uint64_t move_west(uint64_t move, short color);
uint64_t move_east(uint64_t move, short color);
uint64_t move_north_east(uint64_t move, short color);
uint64_t move_north_west(uint64_t move, short color);
uint64_t move_south_east(uint64_t move, short color);
uint64_t move_south_west(uint64_t move, short color);
uint64_t validate_move(uint64_t move, uint64_t pb, uint64_t ob);
uint64_t validate_capture(uint64_t move, uint64_t pb, uint64_t ob);
uint64_t validate_en_passant(uint64_t piece_bitboard, short color, uint64_t en_passant, board *b);
uint64_t validate_castle(uint64_t king, short color, board *b);
void move(square src, square dest, uint64_t *src_piece_ptr, uint64_t *dest_piece_ptr, short move_type, board *b);
short simulate_move(square src, square dest, short turn, board *b);
short make_move(square src, square dest, short turn, board *b);


/* Lookup Functions */
uint64_t pawn_lookup(uint64_t pawn, short color, board *b);
uint64_t knight_lookup(uint64_t knight, short color, board *b);
uint64_t bishop_lookup(uint64_t bishop, short color, board *b);
uint64_t rook_lookup(uint64_t rook, short color, board *b);
uint64_t queen_lookup(uint64_t queen, short color, board *b);
uint64_t king_lookup(uint64_t king, short color, board *b);
uint64_t generate_lookup_table(uint64_t piece_bitboard, uint8_t piece_id, board *b);
uint64_t get_combined_lookup_table(board *b, short color);


/* Legal Move Functions */
uint64_t generate_legal_moves(uint64_t piece_bitboard, uint8_t piece_id, short turn, board *b);
int get_legal_moves_from_attack_table(uint64_t generated_move, uint64_t piece, short int legal_moves_array[MAX_LEGAL_MOVES][4], int legal_moves_array_count);
int get_all_legal_moves(uint8_t color, board* b, short int legal_moves_array[MAX_LEGAL_MOVES][4]);
uint64_t generate_attack_tables_for_color(uint8_t color, board *b);
void update_attack_tables(board *b, short turn);
void promotion_move_menu();