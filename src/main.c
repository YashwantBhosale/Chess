#include <wchar.h>
#include <locale.h>
#include "chessboard.h"

#define STARTING_FEN "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6"

int main() {
    setlocale(LC_ALL, "");
    Board b;
    load_fen(&b, STARTING_FEN);
    print_board(&b, b.turn);
    return 0;
}