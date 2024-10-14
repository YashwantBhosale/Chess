#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include "chessboard.h"
#include "moves.h"
#include "move_stack.h"


move_stack moves;

square read_square(){
    char file;
    int rank;
    square s;
    scanf(" %c%d", &file, &rank);
    s.file = file - 'a' + 1;
    s.rank = rank;

    return s;
}

void clrscr() {
    wprintf(L"\033[H\033[J");
}

void new_game(board *b) {
    short turn, status;
    
    init_board(b);
    init_move_stack(&moves);

    turn = WHITE;

    while(1){
  
        // update_attack_tables(b);
        //clrscr();
        if(b->attack_tables[WHITE] == 0ULL){
            wprintf(L"%s is in checkmate 1\n", "White");
            break;
        }

        else if(b->attack_tables[BLACK] == 0ULL){
            wprintf(L"%s is in checkmate\n", "Black");
            break;
        }


        print_board(b, turn);
        wprintf(L"%s's turn\n", turn == WHITE ? "White" : "Black");

        printf("Enter the source square: ");
        square src = read_square();
        printf("Enter the destination square: ");
        square dest = read_square();
        status = make_move(src, dest, b);

        switch (status){
        case NORMAL_MOVE:
            break;
        case CAPTURE_MOVE:
            wprintf(L"Captured piece\n");
            break;
        case CHECK_MOVE:
            wprintf(L"Check\n");
            break;
        case CHECKMATE_MOVE:
            wprintf(L"Checkmate\n");
            break;
        case STALEMATE_MOVE:
            wprintf(L"Stalemate\n");
            break;
        case INVALID_MOVE:
            wprintf(L"control comes here\n");
            wprintf(L"Invalid move\n");
            break;
        default:
            break;
        }

        // update_attack_tables(b);
        update_attack_tables(b, turn);
        
        // wprintf(L"attack table of white: ");
        // print_legal_moves(b->attack_tables[WHITE]);
        // wprintf(L"attack table of black: ");
        // print_legal_moves(b->attack_tables[BLACK]);
        

        // view_square_table(b);  
        if(b->attack_tables[WHITE] == 0ULL){
            wprintf(L"%s is in checkmate 2\n", "White");
            break;
        }

        else if(b->attack_tables[BLACK] == 0ULL){
            wprintf(L"%s is in checkmate\n", "Black");
            break;
        }

        if(status == INVALID_MOVE){
            continue;
        }


        if(status == CHECKMATE_MOVE || status == STALEMATE_MOVE){
            break;
        }
        turn = !turn;
    }

}

int main(){
    setlocale(LC_CTYPE, "");
    board *b = (board *)malloc(sizeof(board));
    new_game(b);
    free(b);
    return 0;
}
