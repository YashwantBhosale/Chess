#define FILEMASK_A 0b0000000100000001000000010000000100000001000000010000000100000001ULL

short num_attacked_pawns(board* board, short turn);
short num_attacked_knights(board* board, short turn);
short num_attacked_bishops(board* board, short turn);
short num_attacked_rooks(board* board, short turn);
short num_attacked_queens(board* board, short turn);
short num_doubled_blocked_pawns(board* board, short turn);
short num_isolated_pawns(board* board, short turn);
double get_evaluation_of_board(board* board);
void display_evaluation(double eval);