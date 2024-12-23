#include <wchar.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "chessboard.h"
#include "move_gen.h"

#define STARTING_FEN "r1bqk2r/pppp1ppp/2n2n2/2b1p1B1/3PP3/2N2Q2/PPP2PPP/R3KBNR b KQkq - 4 5"

int main() {
    setlocale(LC_ALL, "");
    Board b;
    load_fen(&b, STARTING_FEN);
    print_board(&b, b.turn);
    wprintf(L"\n\n");

    MoveList list;
    init_movelist(&list);
    generate_king_attacks(&b, &list, b.turn);
    print_movelist(&list);
    return 0;
}