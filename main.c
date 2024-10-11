#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include "chessboard.h"
#include "moves.h"
#include "move_stack.h"

move_stack moves;

square get_square(){
    char file;
    int rank;
    square s;
    scanf(" %c%d", &file, &rank);
    s.file = file - 'a' + 1;
    s.rank = rank;

    return s;
}

int main(){
    short status;
    setlocale(LC_CTYPE, "");
    board *b = (board *)malloc(sizeof(board));
    init_board(b);
    // print_board(b, WHITE);

    while(1){
        print_board(b, WHITE);
        printf("Enter the source square: ");
        square src = get_square();
        printf("Enter the destination square: ");
        square dest = get_square();
        status = make_move(src, dest, b);
    }

    free(b);
    return 0;
}