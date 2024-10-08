#include<stdio.h>
#include<stdlib.h>
#include<wchar.h>
#include<locale.h>
#include"chessboard.h"

int main(){
    setlocale(LC_CTYPE, "");
    board *b = (board *)malloc(sizeof(board));
    init_board(b);
    print_board(b, WHITE);
    free(b);
    return 0;
}