#ifndef MOVE_GEN_H
#define MOVE_GEN_H

void generate_pawn_attacks(Board *b, MoveList *list, Byte color);
void generate_bishop_attacks(Board *b, MoveList *list, Byte color);
void generate_knight_attacks(Board *b, MoveList *list, Byte color);
void generate_rook_attacks(Board *b, MoveList *list, Byte color);
void generate_queen_attacks(Board *b, MoveList *list, Byte color);
void generate_king_attacks(Board *b, MoveList *list, Byte color);

#endif