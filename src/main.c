#include <wchar.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include "chessboard.h"
#include "move_gen.h"

#define STARTING_FEN "r1bqkb1r/pppp1ppp/2n2n2/4p3/3PP3/5N2/PPP2PPP/RNBQKB1R w KQkq - 3 4"

int main() {
    setlocale(LC_ALL, "");
    Board b;
    load_fen(&b, STARTING_FEN);
    print_board(&b, b.turn);
    wprintf(L"\n\n");
    return 0;
}