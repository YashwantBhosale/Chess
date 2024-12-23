#ifndef MOVE_GEN_H
#define MOVE_GEN_H

Bitboard generate_pawn_attacks(Board *b, MoveList *list, Byte color);
Bitboard generate_bishop_attacks(Board *b, MoveList *list, Byte color);
Bitboard generate_knight_attacks(Board *b, MoveList *list, Byte color);
Bitboard generate_rook_attacks(Board *b, MoveList *list, Byte color);
Bitboard generate_queen_attacks(Board *b, MoveList *list, Byte color);
Bitboard generate_king_attacks(Board *b, MoveList *list, Byte color);

#endif