#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "chessboard.h"

void init_pieces(Pieces *type) {
	type->pawns = 0ULL;
	type->knights = 0ULL;
	type->bishops = 0ULL;
	type->rooks = 0ULL;
	type->queens = 0ULL;
	type->king = 0ULL;

	memset(&type->count, 0, sizeof(PieceCount));
}

void init_board(Board *b) {
	if (!b) return;

	b->white_board = NULL_BITBOARD;
	b->black_board = NULL_BITBOARD;
	b->en_passant_square = NULL_BITBOARD;
	b->castle_rights = WHITE_CASTLE_RIGHTS | BLACK_CASTLE_RIGHTS;

	memset(b->square_table, 0, sizeof(b->square_table));
	memset(b->captured_pieces[0], 0, sizeof(Byte) * 16);
	memset(b->captured_pieces[1], 0, sizeof(Byte) * 16);

	init_movelist(&b->moves);
	init_pieces(&b->white);
	init_pieces(&b->black);
}

void update_square_table(int file, int rank, Byte piece, Board *b) {
	b->square_table[file - 1][rank - 1] = piece;
}

static inline Byte get_id(Byte piece_type, Byte color) {
	return (piece_type | (color == WHITE ? 0 : 8));
}

static inline Bitboard place_piece(int file, int rank, Byte piece, Board *b) {
	Bitboard bb = get_bitboard(file, rank);
	update_square_table(file, rank, piece, b);
	return bb;
}

void load_fen(Board *b, const char *fen) {
	if (!b || !fen) return;

	// assuming board is not initialized
	init_board(b);

	int file = A, rank = 8;
	for (int i = 0; fen[i] != ' '; i++) {
		if (fen[i] == '/') {
			rank--;
			file = A;
			continue;
		}

		// empty files
		if (fen[i] >= '1' && fen[i] <= '8') {
			file += fen[i] - '0';
			continue;
		}

		Byte piece = 0;

		switch (fen[i]) {
			case 'P':
				b->white.count.pawns++;
				piece = get_id(PAWN, WHITE);
                b->white.pawns |= place_piece(file, rank, piece, b);
				b->white_board |= b->white.pawns;
				break;
			case 'p':
				b->black.count.pawns++;
				piece = get_id(PAWN, BLACK);
                b->black.pawns |= place_piece(file, rank, piece, b);
				b->black_board |= b->black.pawns;
				break;
			case 'N':
				b->white.count.knights++;
				piece = get_id(KNIGHT, WHITE);
                b->white.knights |= place_piece(file, rank, piece, b);
				b->white_board |= b->white.knights;
				break;
			case 'n':
				b->black.count.knights++;
				piece = get_id(KNIGHT, BLACK);
                b->black.knights |= place_piece(file, rank, piece, b);
				b->black_board |= b->black.knights;
				break;
			case 'B':
				b->white.count.bishops++;
				piece = get_id(BISHOP, WHITE);
                b->white.bishops |= place_piece(file, rank, piece, b);
				b->white_board |= b->white.bishops;
				break;
			case 'b':
				b->black.count.bishops++;
				piece = get_id(BISHOP, BLACK);
                b->black.bishops |= place_piece(file, rank, piece, b);
				b->black_board |= b->black.bishops;
				break;
			case 'R':
				b->white.count.rooks++;
				piece = get_id(ROOK, WHITE);
                b->white.rooks |= place_piece(file, rank, piece, b);
				b->white_board |= b->white.rooks;
				break;
			case 'r':
				b->black.count.rooks++;
				piece = get_id(ROOK, BLACK);
                b->black.rooks |= place_piece(file, rank, piece, b);
				b->black_board |= b->black.rooks;
				break;
            case 'Q':
                b->white.count.queens++;
                piece = get_id(QUEEN, WHITE);
                b->white.queens = place_piece(file, rank, piece, b);
                b->white_board |= b->white.queens;
                break;
            case 'q':
                b->black.count.queens++;
                piece = get_id(QUEEN, BLACK);
                b->black.queens |= place_piece(file, rank, piece, b);
                b->black_board |= b->black.queens;
                break;
			case 'K':
				piece = get_id(KING, WHITE);
                b->white.king = place_piece(file, rank, piece, b);
				b->white_board |= b->white.king;
				break;
			case 'k':
				piece = get_id(KING, BLACK);
                b->black.king = place_piece(file, rank, piece, b);
				b->black_board |= b->black.king;
				break;
			default:
				break;
		}
		file++;
	}

	char *turn = strchr(fen, ' ') + 1;
	if (*turn == 'w') {
		b->turn = WHITE;
	} else {
		b->turn = BLACK;
	}

	char *castle_rights = strchr(turn, ' ') + 1;
	if (*castle_rights == '-') {
		b->castle_rights = 0;
	} else {
		char *p = castle_rights;
		while (*p != ' ') {
			switch (*p) {
				case 'K':
					b->castle_rights |= 0b00000010;
					break;
				case 'Q':
					b->castle_rights |= 0b00000001;
					break;
				case 'k':
					b->castle_rights |= 0b00100000;
					break;
				case 'q':
					b->castle_rights |= 0b00010000;
					break;
				default:
					break;
			}
			p++;
		}
	}

	char *en_passant = strchr(castle_rights, ' ') + 1;
	if (*en_passant == '-') {
		b->en_passant_square = 0ULL;
	} else {
		char file_char = en_passant[0];  // File character (e.g., 'c')
		char rank_char = en_passant[1];  // Rank character (e.g., '6')

		int en_passant_file = file_char - 'a' + 1;
		int en_passant_rank = rank_char - '0';

        wprintf(L"%d%d", en_passant_file, en_passant_rank);

		b->en_passant_square = get_bitboard(en_passant_file, en_passant_rank);
	}

	// pending captured pieces
}

static inline Byte piece_type(Byte id) {
    return id & 0b00000111;
}

static inline Byte piece_color(Byte id) {
    return id & 0b00001000;
}

static void print_piece(Byte id) {
	switch (piece_type(id)) {
		case PAWN:
			wprintf(!piece_color(id) ? L" ♟ |" : L" ♙ |");
			break;
		case KNIGHT:
			wprintf(!piece_color(id) ? L" ♞ |" : L" ♘ |");
			break;
		case BISHOP:
			wprintf(!piece_color(id) ? L" ♝ |" : L" ♗ |");
			break;
		case ROOK:
			wprintf(!piece_color(id) ? L" ♜ |" : L" ♖ |");
			break;
		case QUEEN:
			wprintf(!piece_color(id) ? L" ♛ |" : L" ♕ |");
			break;
		case KING:
			wprintf(!piece_color(id) ? L" ♚ |" : L" ♔ |");
			break;
		default:
			wprintf(L"   |");
			break;
	}
	return;
}

void print_board(Board *b, Byte turn) {
	Byte piece, start_rank, end_rank, start_file, end_file;

	switch (turn) {
		case WHITE:
			start_file = A;
			start_rank = 8;
			end_file = H;
			end_rank = 1;
			break;
		case BLACK:
			start_file = H;
			start_rank = 1;
			end_file = A;
			end_rank = 8;
			break;
		default:
			return;
	}

	for (int rank = start_rank; turn == WHITE ? rank >= end_rank : rank <= end_rank; turn == WHITE ? rank-- : rank++) {
		wprintf(L"\t\t+---+---+---+---+---+---+---+---+\n");
		wprintf(L"\t\t|");

		for (int file = start_file; turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--) {
			piece = b->square_table[file - 1][rank - 1];
			print_piece(piece);
		}

		wprintf(L" %d\n", rank);
	}

	wprintf(L"\t\t+---+---+---+---+---+---+---+---+\n\t\t");
	wprintf(L" ");
	char _files[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

	for (int file = start_file; turn == WHITE ? file <= end_file : file >= end_file; turn == WHITE ? file++ : file--) {
		wprintf(L" %c  ", _files[file - 1]);
	}
	wprintf(L"\n");
}