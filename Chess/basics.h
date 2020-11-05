#ifndef __CHESS_BASICS__
#define __CHESS_BASICS__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

// static unsigned int _movecounter_ = 0;

// A piece = 4 bits
// black = 0, white = 1

#define BLANK    0b0000
#define PAWN_B   0b0010
#define KNIGHT_B 0b0100
#define BISHOP_B 0b0110
#define ROOK_B   0b1000
#define QUEEN_B  0b1010
#define KING_B   0b1100
#define PAWN_W   0b0011
#define KNIGHT_W 0b0101
#define BISHOP_W 0b0111
#define ROOK_W   0b1001
#define QUEEN_W  0b1011
#define KING_W   0b1101
#define UNKNOWN  0b1110

struct node {
	unsigned long len;
	uint64_t *pos;
	char brkrwrkr00;
	struct node *branches;
	char *move;
	char color;
	char isnotleafnode;
};

struct stringandweight {
	char *move;
	int weight;
};

static inline char absol(char);
static inline char isWhite(char);
static inline char isBlack(char);
static inline unsigned char accessBoardAt(uint64_t const *, unsigned char);
static inline void setBoardAt(uint64_t *, unsigned char, unsigned char);
static inline char makeMove(uint64_t *, uint64_t const *, char *, char *); // char * (3nd arg) -> {from, to, promotionPiece or 0}
uint64_t *newChessBoard();
void makeForcedMove(uint64_t *, char *, char const *); // make move without validating
char validateMove(uint64_t const *, uint64_t const *, char, char const *);
static inline char pieceToNotation(unsigned char);
static inline unsigned char notationToPiece(char);
static inline char pieceToLowerNotation(unsigned char);
static inline unsigned char notationToWhitePiece(char);
static inline unsigned char notationToBlackPiece(char);
void freeNodes(struct node);
void printBoard(uint64_t const *);
void printBoardToFile(FILE *, uint64_t const *);
void printValidMoves(uint64_t const *, uint64_t const *, char, char);
unsigned long validMoves(uint64_t const *, uint64_t const *, char, char, char **);
unsigned long generateNodes(uint64_t const *, uint64_t const *, char, char, struct node **, int);
char *theBestMove(uint64_t const *, uint64_t const *, char, char, int);
static inline char *uciToIndices(uint64_t const *, char const *);
static inline char *indicesToUci(char const *);
static inline unsigned char chessPosToIndex(char const *);
char isCheckOnKing(uint64_t const *, char /*king color: 0 - black, 1 - white*/);
char isCheckOnXY(uint64_t const *, char, char, char);
int evaluateNode(struct node);
struct stringandweight minimax(struct node, int, char);

static inline char absol(char x) {
	return x * -(x < 0);
}

static inline char isWhite(char p) {
	return p & 1;
}

static inline char isBlack(char p) {
	return p && p != UNKNOWN && !(p & 1);
}

void printBoard(uint64_t const *board) {
	unsigned char i;
	for (i = 0; i < 64; i++) {
		printf("%c", pieceToNotation(accessBoardAt(board, i)));
		if ((i + 1) % 8 == 0) printf("\n");
	}
	fflush(stdout);
}

// void printBoardToFile(FILE *fp, uint64_t const *board) {
// 	unsigned char i;
// 	for (i = 0; i < 64; i++) {
// 		fprintf(fp, "%c", pieceToNotation(accessBoardAt(board, i)));
// 		if ((i + 1) % 8 == 0) fprintf(fp, "\n");
// 	}
// }

static inline unsigned char accessBoardAt(uint64_t const *board, unsigned char ind) {
	if (ind >= 64)
		return UNKNOWN;
	return (board[ind / 16] >> ((15 - (ind % 16)) * 4)) & 15;
}

char validateMove(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char const *move) {
	if (move[0] == move[1])
		return 0;
	if (move[0] < 0 || move[0] > 64 || move[1] < 0 || move[1] > 64)
		return 0;

	unsigned char fromPiece = accessBoardAt(board, move[0]), toPiece = accessBoardAt(board, move[1]);
	unsigned char isBlackFrom = isBlack(fromPiece), isBlackTo = isBlack(toPiece);
	unsigned char isWhiteFrom = isWhite(fromPiece), isWhiteTo = isWhite(toPiece);
	
	if (!fromPiece)
		return 0;
	if ((isBlackFrom && isBlackTo) || (isWhiteFrom && isWhiteTo))
		return 0;
	if (toPiece == KING_B || toPiece == KING_W)
		return 0;
	
	char valid;
	switch (fromPiece) {
		case PAWN_W: case PAWN_B: {
			if (move[1] == (move[0] + (isBlackFrom ? 8 : -8))) {
				if ((isBlackFrom && isWhiteTo) || (isWhiteFrom && isBlackTo))
					valid = 0;
				else if (isBlackFrom ? (move[1] >= 56 && move[1] <= 63) : (move[1] >= 0 && move[1] <= 7))
					valid = move[2] == (isBlackFrom ? QUEEN_B : QUEEN_W) || move[2] == (isBlackFrom ? KNIGHT_B : KNIGHT_W) || move[2] == (isBlackFrom ? BISHOP_B : BISHOP_W) || move[2] == (isBlackFrom ? ROOK_B : ROOK_W);
				else if (!move[2])
					valid = 1;
				else
					valid = 0;
			} else if (move[1] == (move[0] + (isBlackFrom ? 16 : -16)))
				valid = !move[2] && !accessBoardAt(board, move[1]) && (isBlackFrom ? (move[0] >= 8 && move[0] <= 15 && !accessBoardAt(board, move[0] + 8)) : (move[0] >= 48 && move[0] <= 55 && !accessBoardAt(board, move[0] - 8)));
			else if ((move[1] == (move[0] + (isBlackFrom ? 7 : -7)) || move[1] == (move[0] + (isBlackFrom ? 9 : -9))) && absol(move[0] / 8 - move[1] / 8) == 1) {
				if (isBlackFrom ? (move[1] >= 56 && move[1] <= 63) : (move[1] >= 0 && move[1] <= 7))
					valid = move[2] == (isBlackFrom ? QUEEN_B : QUEEN_W) || move[2] == (isBlackFrom ? KNIGHT_B : KNIGHT_W) || move[2] == (isBlackFrom ? BISHOP_B : BISHOP_W) || move[2] == (isBlackFrom ? ROOK_B : ROOK_W);
				else if (!move[2] && ((isBlackFrom && isWhiteTo) || (isWhiteFrom && isBlackTo)))
					valid = 1;
				else if (!move[2] && (isBlackFrom ? (move[0] >= 32 && move[0] <= 39) : (move[0] >= 24 && move[0] <= 31)) && accessBoardAt(board, move[1] + (isBlackFrom ? -8 : 8)) == (isWhiteFrom ? PAWN_B : PAWN_W) && !accessBoardAt(prevBoard, move[1] + (isBlackFrom ? -8 : 8)) && !accessBoardAt(prevBoard, move[1]))
					valid = 1;
				else
					valid = 0;
			} else valid = 0;
		} break;
		case KNIGHT_W: case KNIGHT_B: {
			valid = move[2] == 0 && ((move[0] > 15 ? (((move[0] % 8 > 0) ? (move[1] == move[0] - 17) : 0) || ((move[0] % 8 < 7) ? (move[1] == move[0] - 15) : 0)) : 0) || (move[0] < 48 ? ((move[0] % 8 > 0 ? (move[1] == move[0] + 15) : 0) || (move[0] % 8 < 7 ? (move[1] == move[0] + 17) : 0)) : 0) || (move[0] > 7 ? ((move[0] % 8 > 1 ? (move[1] == move[0] - 10) : 0) || (move[0] % 8 < 6 ? (move[1] == move[0] - 6) : 0)) : 0) || (move[0] < 56 ? ((move[0] % 8 < 6 ? (move[1] == move[0] + 10) : 0) || (move[0] % 8 > 1 ? (move[1] == move[1] + 6) : 0)) : 0));
		} break;
		case BISHOP_W: case BISHOP_B: {
			char x1 = move[0] % 8;
			char y1 = move[0] / 8;
			char x2 = move[1] % 8;
			char y2 = move[1] / 8;

			if (move[2] != 0 || absol(x2 - x1) != absol(y2 - y1))
				valid = 0;
			else {
				valid = 1;

				char x2gtx1 = (x2 > x1 ? 1 : -1);
				char y2gty1 = (y2 > y1 ? 1 : -1);
				for (char X = x1 + x2gtx1, Y = y1 + y2gty1; (x2 > x1 ? X < x2 : X > x2) && (y2 > y1 ? Y < y2 : Y > y2); X += x2gtx1, Y += y2gty1)
					if (accessBoardAt(board, X + Y * 8)) {
						valid = 0;
						break;
					}
			}
		} break;
		case ROOK_W: case ROOK_B: {
			char x1 = move[0] % 8;
			char y1 = move[0] / 8;
			char x2 = move[1] % 8;
			char y2 = move[1] / 8;

			valid = 1;

			if (move[2] != 0 || (x1 != x2 && y1 != y2))
				valid = 0;
			else if (x1 == x2) {
				char y2gty1 = y2 > y1 ? 1 : -1;
				for (char Y = y1 + y2gty1; y2 > y1 ? Y < y2 : Y > y2; Y += y2gty1)
					if (accessBoardAt(board, x1 + Y * 8)) {
						valid = 0;
						break;
					}
			} else {
				char x2gtx1 = x2 > x1 ? 1 : -1;
				for (char X = x1 + x2gtx1; x2 > x1 ? X < x2 : X > x2; X += x2gtx1)
					if (accessBoardAt(board, X + y1 * 8)) {
						valid = 0;
						break;
					}
			}
		} break;
		case QUEEN_W: case QUEEN_B: {
			char x1 = move[0] % 8;
			char y1 = move[0] / 8;
			char x2 = move[1] % 8;
			char y2 = move[1] / 8;
			
			valid = 1;

			if (move[2] != 0 || (absol(x2 - x1) != absol(y2 - y1) && x1 != x2 && y1 != y2))
				valid = 0;
			else if (absol(x2 - x1) == absol(y2 - y1)) {
				char x2gtx1 = x2 > x1 ? 1 : -1;
				char y2gty1 = y2 > y1 ? 1 : -1;
				for (char X = x1 + x2gtx1, Y = y1 + y2gty1; (x2 > x1 ? X < x2 : X > x2) && (y2 > y1 ? Y < y2 : Y > y2); X += x2gtx1, Y += y2gty1)
					if (accessBoardAt(board, X + Y * 8)) {
						valid = 0;
						break;
					}
			} else if (x1 == x2) {
				char y2gty1 = y2 > y1 ? 1 : -1;
				for (char Y = y1 + y2gty1; y2 > y1 ? Y < y2 : Y > y2; Y += y2gty1)
					if (accessBoardAt(board, x1 + Y * 8)) {
						valid = 0;
						break;
					}
			} else {
				char x2gtx1 = x2 > x1 ? 1 : -1;
				for (char X = x1 + x2gtx1; x2 > x1 ? X < x2 : X > x2; X += x2gtx1)
					if (accessBoardAt(board, X + y1 * 8)) {
						valid = 0;
						break;
					}
			}
		} break;
		case KING_W: case KING_B: {
			char x1 = move[0] % 8;
			char y1 = move[0] / 8;
			char x2 = move[1] % 8;
			char y2 = move[1] / 8;

			valid = move[2] == 0 && ((absol(x2 - x1) == 1 || x2 - x1 == 0) && (absol(y2 - y1) == 1 || y2 - y1 == 0));

			if (!valid && y1 == (isWhiteFrom ? 7 : 0) && x1 == 4 && y2 == y1 && absol(x2 - x1) == 2 && ((brkrwrkr00 >> (isBlackFrom ? 6 : 3)) & 1) && ((brkrwrkr00 >> (x2 > x1 ? (isBlackFrom ? 5 : 2) : (isBlackFrom ? 7 : 4))) & 1) && !isCheckOnXY(board, isWhiteFrom, x1, y1) && !accessBoardAt(board, (move[0] + move[1]) / 2) && !accessBoardAt(board, move[1]) && accessBoardAt(board, (isBlackFrom && x2 < x1) ? 0 : isBlackFrom ? 7 : (isWhiteFrom && x2 < x1) ? 56 : 63) == (isBlackFrom ? ROOK_B : ROOK_W) && (x2 < x1 ? !accessBoardAt(board, isBlackFrom ? 1 : 57) : 1) && !isCheckOnXY(board, isWhiteFrom, (x1 + x2) / 2, y1) && !isCheckOnXY(board, isWhiteFrom, x2, y2))
				valid = 1;
		} break;
		default: {
			valid = 0;
		}
	}

	if (valid) {
		uint64_t *board_cpy = malloc(4 * sizeof(uint64_t));
		memcpy(board_cpy, board, 32);
		makeForcedMove(board_cpy, &brkrwrkr00, move);
		if (isCheckOnKing(board_cpy, isWhiteFrom))
			valid = 0;
		free(board_cpy);
	}

	return valid;
}

unsigned long validMoves(uint64_t const *boardcpy, uint64_t const *prevBoard, char brkrwrkr00, char isWhiteYourColor, char **retAddr) {
	uint64_t *board = memcpy(malloc(32), boardcpy, 32);
	char (*isMyC)(char) = isWhiteYourColor ? isWhite : isBlack;

	*retAddr = 0;
	unsigned long _len = 0;

	for (char i = 0; i < 64; i++) {
		char acbabi = accessBoardAt(board, i);

		if (!isMyC(acbabi))
			continue;

		char x = i % 8;
		char y = i / 8;

		switch (acbabi) {
			case PAWN_W: {
				char acbabim8ib = !accessBoardAt(board, i - 8);

				if (acbabim8ib && y != 1) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 8;
					(*retAddr)[_len - 1] = 0;
				}

				if (y == 6 && acbabim8ib && !accessBoardAt(board, i - 16)) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 16;
					(*retAddr)[_len - 1] = 0;
				}

				char im7 = accessBoardAt(board, i - 7);
				char im7ib = isBlack(im7);

				if (x <= 6 && ((y == 3 && accessBoardAt(board, i + 1) == PAWN_B && !im7 && !accessBoardAt(prevBoard, i + 1)) || (im7ib && y != 1))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 7;
					(*retAddr)[_len - 1] = 0;
				}
				
				char im9 = accessBoardAt(board, i - 9);
				char im9ib = isBlack(im9);

				if (x >= 1 && ((y == 3 && accessBoardAt(board, i - 1) == PAWN_B && !im9 && !accessBoardAt(prevBoard, i - 1)) || (im9ib && y != 1))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 9;
					(*retAddr)[_len - 1] = 0;
				}

				if (y == 1 && acbabim8ib) {
					*retAddr = realloc(*retAddr, _len += 12);
					(*retAddr)[_len - 1] = KNIGHT_W;
					(*retAddr)[_len - 2] = i - 8;
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 4] = BISHOP_W;
					(*retAddr)[_len - 5] = i - 8;
					(*retAddr)[_len - 6] = i;
					(*retAddr)[_len - 7] = ROOK_W;
					(*retAddr)[_len - 8] = i - 8;
					(*retAddr)[_len - 9] = i;
					(*retAddr)[_len - 10] = QUEEN_W;
					(*retAddr)[_len - 11] = i - 8;
					(*retAddr)[_len - 12] = i;
				}

				if (y == 1 && im9ib) {
					*retAddr = realloc(*retAddr, _len += 12);
					(*retAddr)[_len - 1] = KNIGHT_W;
					(*retAddr)[_len - 2] = i - 9;
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 4] = BISHOP_W;
					(*retAddr)[_len - 5] = i - 9;
					(*retAddr)[_len - 6] = i;
					(*retAddr)[_len - 7] = ROOK_W;
					(*retAddr)[_len - 8] = i - 9;
					(*retAddr)[_len - 9] = i;
					(*retAddr)[_len - 10] = QUEEN_W;
					(*retAddr)[_len - 11] = i - 9;
					(*retAddr)[_len - 12] = i;
				}

				if (y == 1 && im7ib) {
					*retAddr = realloc(*retAddr, _len += 12);
					(*retAddr)[_len - 1] = KNIGHT_W;
					(*retAddr)[_len - 2] = i - 7;
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 4] = BISHOP_W;
					(*retAddr)[_len - 5] = i - 7;
					(*retAddr)[_len - 6] = i;
					(*retAddr)[_len - 7] = ROOK_W;
					(*retAddr)[_len - 8] = i - 7;
					(*retAddr)[_len - 9] = i;
					(*retAddr)[_len - 10] = QUEEN_W;
					(*retAddr)[_len - 11] = i - 7;
					(*retAddr)[_len - 12] = i;
				}
			} break;
			case PAWN_B: {
				char acbabip8ib = !accessBoardAt(board, i + 8);

				if (acbabip8ib && y != 6) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 8;
					(*retAddr)[_len - 1] = 0;
				}

				if (y == 1 && acbabip8ib && !accessBoardAt(board, i + 16)) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 16;
					(*retAddr)[_len - 1] = 0;
				}

				char ip7 = accessBoardAt(board, i + 9);
				char ip7ib = isWhite(ip7);

				if (x <= 6 && ((y == 4 && accessBoardAt(board, i + 1) == PAWN_W && !ip7 && !accessBoardAt(prevBoard, i + 1)) || (ip7ib && y != 6))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 9;
					(*retAddr)[_len - 1] = 0;
				}
				
				char ip9 = accessBoardAt(board, i + 7);
				char ip9ib = isWhite(ip9);

				if (x >= 1 && ((y == 4 && accessBoardAt(board, i - 1) == PAWN_W && !ip9 && !accessBoardAt(prevBoard, i - 1)) || (ip9ib && y != 6))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 7;
					(*retAddr)[_len - 1] = 0;
				}

				if (y == 6 && acbabip8ib) {
					*retAddr = realloc(*retAddr, _len += 12);
					(*retAddr)[_len - 1] = KNIGHT_B;
					(*retAddr)[_len - 2] = i + 8;
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 4] = BISHOP_B;
					(*retAddr)[_len - 5] = i + 8;
					(*retAddr)[_len - 6] = i;
					(*retAddr)[_len - 7] = ROOK_B;
					(*retAddr)[_len - 8] = i + 8;
					(*retAddr)[_len - 9] = i;
					(*retAddr)[_len - 10] = QUEEN_B;
					(*retAddr)[_len - 11] = i + 8;
					(*retAddr)[_len - 12] = i;
				}

				if (y == 6 && ip9ib) {
					*retAddr = realloc(*retAddr, _len += 12);
					(*retAddr)[_len - 1] = KNIGHT_B;
					(*retAddr)[_len - 2] = i + 7;
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 4] = BISHOP_B;
					(*retAddr)[_len - 5] = i + 7;
					(*retAddr)[_len - 6] = i;
					(*retAddr)[_len - 7] = ROOK_B;
					(*retAddr)[_len - 8] = i + 7;
					(*retAddr)[_len - 9] = i;
					(*retAddr)[_len - 10] = QUEEN_B;
					(*retAddr)[_len - 11] = i + 7;
					(*retAddr)[_len - 12] = i;
				}

				if (y == 6 && ip7ib) {
					*retAddr = realloc(*retAddr, _len += 12);
					(*retAddr)[_len - 1] = KNIGHT_B;
					(*retAddr)[_len - 2] = i + 9;
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 4] = BISHOP_B;
					(*retAddr)[_len - 5] = i + 9;
					(*retAddr)[_len - 6] = i;
					(*retAddr)[_len - 7] = ROOK_B;
					(*retAddr)[_len - 8] = i + 9;
					(*retAddr)[_len - 9] = i;
					(*retAddr)[_len - 10] = QUEEN_B;
					(*retAddr)[_len - 11] = i + 9;
					(*retAddr)[_len - 12] = i;
				}
			} break;
			case KNIGHT_W: case KNIGHT_B: {
				if (x < 6 && y < 7 && !isMyC(accessBoardAt(board, i + 10))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 10;
					(*retAddr)[_len - 1] = 0;
				}

				if (x < 6 && y > 0 && !isMyC(accessBoardAt(board, i - 6))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 6;
					(*retAddr)[_len - 1] = 0;
				}

				if (x > 1 && y > 0 && !isMyC(accessBoardAt(board, i - 10))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 10;
					(*retAddr)[_len - 1] = 0;
				}

				if (x > 1 && y < 7 && !isMyC(accessBoardAt(board, i + 6))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 6;
					(*retAddr)[_len - 1] = 0;
				}

				if (x < 7 && y < 6 && !isMyC(accessBoardAt(board, i + 17))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 17;
					(*retAddr)[_len - 1] = 0;
				}

				if (x < 7 && y > 1 && !isMyC(accessBoardAt(board, i - 15))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 15;
					(*retAddr)[_len - 1] = 0;
				}

				if (x > 0 && y > 1 && !isMyC(accessBoardAt(board, i - 17))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 17;
					(*retAddr)[_len - 1] = 0;
				}

				if (x > 0 && y < 6 && !isMyC(accessBoardAt(board, i + 15))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 15;
					(*retAddr)[_len - 1] = 0;
				}
			} break;
			case BISHOP_W: case BISHOP_B: {
				for (char x2 = x + 1, y2 = y + 1; x2 < 8 && y2 < 8; x2++, y2++) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x - 1, y2 = y + 1; x2 >= 0 && y2 < 8; x2--, y2++) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x + 1, y2 = y - 1; x2 < 8 && y2 >= 0; x2++, y2--) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x - 1, y2 = y - 1; x2 >= 0 && y2 >= 0; x2--, y2--) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}
			} break;
			case ROOK_W: case ROOK_B: {
				for (char x2 = x + 1; x2 < 8; x2++) {
					char acbabx2y2 = accessBoardAt(board, x2 + y * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x - 1; x2 >= 0; x2--) {
					char acbabx2y2 = accessBoardAt(board, x2 + y * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char y2 = y + 1; y2 < 8; y2++) {
					char acbabx2y2 = accessBoardAt(board, x + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char y2 = y - 1; y2 >= 0; y2--) {
					char acbabx2y2 = accessBoardAt(board, x + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}
			} break;
			case QUEEN_W: case QUEEN_B: {
				for (char x2 = x + 1; x2 < 8; x2++) {
					char acbabx2y2 = accessBoardAt(board, x2 + y * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x - 1; x2 >= 0; x2--) {
					char acbabx2y2 = accessBoardAt(board, x2 + y * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char y2 = y + 1; y2 < 8; y2++) {
					char acbabx2y2 = accessBoardAt(board, x + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char y2 = y - 1; y2 >= 0; y2--) {
					char acbabx2y2 = accessBoardAt(board, x + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x + 1, y2 = y + 1; x2 < 8 && y2 < 8; x2++, y2++) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x - 1, y2 = y + 1; x2 >= 0 && y2 < 8; x2--, y2++) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x + 1, y2 = y - 1; x2 < 8 && y2 >= 0; x2++, y2--) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}

				for (char x2 = x - 1, y2 = y - 1; x2 >= 0 && y2 >= 0; x2--, y2--) {
					char acbabx2y2 = accessBoardAt(board, x2 + y2 * 8);
					if (!isMyC(acbabx2y2)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = x2 + y2 * 8;
						(*retAddr)[_len - 1] = 0;
					}

					if (acbabx2y2)
						break;
				}
			} break;
			case KING_W: case KING_B: {
				setBoardAt(board, i, BLANK);

				if (x > 0 && !isMyC(accessBoardAt(board, i - 1))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 1;
					(*retAddr)[_len - 1] = 0;
				}

				if (x < 7 && !isMyC(accessBoardAt(board, i + 1))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 1;
					(*retAddr)[_len - 1] = 0;
				}

				if (y > 0 && !isMyC(accessBoardAt(board, i - 8))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 8;
					(*retAddr)[_len - 1] = 0;
				}

				if (y < 7 && !isMyC(accessBoardAt(board, i + 8))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 8;
					(*retAddr)[_len - 1] = 0;
				}

				if (y > 0 && x > 0 && !isMyC(accessBoardAt(board, i - 9))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 9;
					(*retAddr)[_len - 1] = 0;
				}

				if (y > 0 && x < 7 && !isMyC(accessBoardAt(board, i - 7))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i - 7;
					(*retAddr)[_len - 1] = 0;
				}

				if (y < 7 && x > 0 && !isMyC(accessBoardAt(board, i + 7))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 7;
					(*retAddr)[_len - 1] = 0;
				}

				if (y < 7 && x < 7 && !isMyC(accessBoardAt(board, i + 9))) {
					*retAddr = realloc(*retAddr, _len += 3);
					(*retAddr)[_len - 3] = i;
					(*retAddr)[_len - 2] = i + 9;
					(*retAddr)[_len - 1] = 0;
				}

				if (y == (isWhiteYourColor * 7) && x == 4 && ((brkrwrkr00 >> (6 - isWhiteYourColor * 3)) & 1) && !isCheckOnXY(board, isWhiteYourColor, x, y)) {
					if (((brkrwrkr00 >> (5 - isWhiteYourColor * 3)) & 1) && !accessBoardAt(board, i + 1) && !accessBoardAt(board, i + 2) && !isCheckOnXY(board, isWhiteYourColor, x + 1, y) && !isCheckOnXY(board, isWhiteYourColor, x + 2, y)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = i + 2;
						(*retAddr)[_len - 1] = 0;
					}

					if (((brkrwrkr00 >> (7 - isWhiteYourColor * 3)) & 1) && !accessBoardAt(board, i - 1) && !accessBoardAt(board, i - 2) && !accessBoardAt(board, i - 3) && !isCheckOnXY(board, isWhiteYourColor, x - 1, y) && !isCheckOnXY(board, isWhiteYourColor, x - 2, y)) {
						*retAddr = realloc(*retAddr, _len += 3);
						(*retAddr)[_len - 3] = i;
						(*retAddr)[_len - 2] = i - 2;
						(*retAddr)[_len - 1] = 0;
					}
				}

				setBoardAt(board, i, acbabi /* KING_W or KING_B */ );
			} break;
		}
	}

	if (_len) {
		uint64_t *bbboard = memcpy(malloc(32), board, 32);
		for (long i = _len - 3; i >= 0; i -= 3) {
			char brkrwrkr00cpy = brkrwrkr00;
			makeForcedMove(board, &brkrwrkr00cpy, (char [3]) {(*retAddr)[i], (*retAddr)[i + 1], (*retAddr)[i + 2]});
			if (isCheckOnKing(board, isWhiteYourColor)) {
				char *ucii = indicesToUci((char [3]) {(*retAddr)[i], (*retAddr)[i + 1], (*retAddr)[i + 2]});
				free(ucii);
				if ((unsigned long) (i + 3) != _len) memmove((*retAddr) + i, (*retAddr) + i + 3, _len - i - 3);
				*retAddr = realloc((*retAddr), _len -= 3);
			}
			memcpy(board, bbboard, 32);
		}
		free(bbboard);
	}

	free(board);

	return _len;
}

// char *ntostr(unsigned int n) {
// 	char *ret = NULL;
// 	unsigned int len = 0;

// 	if (!n)
// 		return memcpy(malloc(2), "0", 2);

// 	while (n) {
// 		ret = realloc(ret, ++len);
// 		ret[len - 1] = (n % 10) + '0';
// 		n /= 10;
// 	}

// 	for (unsigned int i = 0; i < len / 2; i++) {
// 		char tmp = ret[i];
// 		ret[i] = ret[len - i - 1];
// 		ret[len - i - 1] = tmp;
// 	}

// 	ret = realloc(ret, ++len);
// 	ret[len - 1] = '\0';

// 	return ret;
// }

unsigned long generateNodes(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char isWhiteYourColor, struct node **ret, int depth) {
	if (!depth)
		return 0;

	char *valids = 0;
	unsigned long _len_ = validMoves(board, prevBoard, brkrwrkr00, isWhiteYourColor, &valids);
	
	if (!_len_)
		return 0;

	*ret = malloc(_len_ / 3 * sizeof **ret);

	// char *drname = NULL;
	
	// if (depth > 1)
	// 	drname = ntostr(_movecounter_);

	for (unsigned long i = 0, j = 0; i < _len_; i += 3, j++) {
		(*ret)[j].color = isWhiteYourColor;
		(*ret)[j].pos = memcpy(malloc(32), board, 32);
		(*ret)[j].brkrwrkr00 = brkrwrkr00;
		(*ret)[j].move = memcpy(malloc(3), valids + i, 3);
		makeForcedMove((*ret)[j].pos, &((*ret)[j].brkrwrkr00), (*ret)[j].move);
		(*ret)[j].len = generateNodes((*ret)[j].pos, board, (*ret)[j].brkrwrkr00, !isWhiteYourColor, &((*ret)[j].branches), depth - 1);
		(*ret)[j].isnotleafnode = depth != 1;
		// if (depth > 1) {
			// remove(drname);
		// 	mkdir(drname, 0777);
		// 	char *fname = ntostr(j);
		// 	char *newstr = malloc(2 + strlen(drname) + strlen(fname));
		// 	sprintf(newstr, "%s/%s", drname, fname);
		// 	FILE *fileopened = fopen(newstr, "w");
		// 	fprintf(fileopened, "%d\n", evaluateNode((*ret)[j]));
		// 	printBoardToFile(fileopened, (*ret)[j].pos);
		// 	fclose(fileopened);
		// 	free(newstr);
		// 	free(fname);
		// }
	}

	// free(drname);
	free(valids);

	return _len_ / 3;
};

void freeNodes(struct node n) {
	free(n.move);
	free(n.pos);

	for (unsigned long i = 0; i < n.len; i++)
		freeNodes(n.branches[i]);
}

struct stringandweight minimax(struct node n, int depth, char maximizer) {
	if (!depth || !n.len)
		return (struct stringandweight) {n.move, evaluateNode(n)};

	char *optimalmove;
	struct stringandweight curr;
	int eval = maximizer ? INT_MIN : INT_MAX;
	for (unsigned long i = 0; i < n.len; i++) {
		curr = minimax(n.branches[i], depth - 1, !maximizer);
		if (maximizer ? (curr.weight > eval) : (curr.weight < eval)) {
			eval = curr.weight;
			optimalmove = n.branches[i].move;
		}
	}

	return (struct stringandweight) {optimalmove, eval};
}

char *theBestMove(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char isWhiteYourColor, int depth) {
	// _movecounter_++;
	if (!depth)
		return NULL;

	struct node baseNode;
	baseNode.len = generateNodes(board, prevBoard, brkrwrkr00, isWhiteYourColor, &baseNode.branches, depth);
	
	printf("%ld\n", baseNode.len);

	if (!baseNode.len)
		return NULL;

	baseNode.pos = memcpy(malloc(32), board, 32);
	baseNode.brkrwrkr00 = brkrwrkr00;
	baseNode.move = NULL;
	baseNode.color = isWhiteYourColor;
	baseNode.isnotleafnode = depth != 1;

	char *bestmove = memcpy(malloc(3), minimax(baseNode, depth, isWhiteYourColor).move, 3);

	freeNodes(baseNode);

	return bestmove;
}

void printValidMoves(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char isWhiteYourColor) {
	char *ret = 0;
	unsigned long len = validMoves(board, prevBoard, brkrwrkr00, isWhiteYourColor, &ret);
	
	for (unsigned long i = 0; i < len; i += 3) {
		char *strr = indicesToUci((char [3]) {ret[i], ret[i + 1], ret[i + 2]});
		printf((i + 3 == len) ? "%s" : "%s ", strr);
		fflush(stdout);
		free(strr);
	}

	printf("\n");
	free(ret);
	fflush(stdout);
}

char isCheckOnKing(uint64_t const *board, char kingColor) {
	char kingX = -1, kingY = -1;

	for (char x = 0; x < 8; x++)
		for (char y = 0; y < 8; y++)
			if (accessBoardAt(board, x + y * 8) == (kingColor ? KING_W : KING_B)) {
				kingX = x;
				kingY = y;
				x = 8;
				break;
			}

	if (kingX == -1 || kingY == -1) {
		printf("No King on Board!\n");
		fflush(stdout);
		return 0;
	}

	return isCheckOnXY(board, kingColor, kingX, kingY);
}

char isCheckOnXY(uint64_t const *board, char kingColor, char kingX, char kingY) {
	if ((accessBoardAt(board, kingX + 8 * (kingY + (kingColor ? -1 : 1)) - 1) == (kingColor ? PAWN_B : PAWN_W) && kingX > 0) || (accessBoardAt(board, kingX + 8 * (kingY + (kingColor ? -1 : 1)) + 1) == (kingColor ? PAWN_B : PAWN_W) && kingX < 7))
		return 1;
	else {
		for (char i = -1; i <= 1; i++)
			for (char j = -1; j <= 1; j++)
				if ((i || j) && accessBoardAt(board, i + kingX + 8 * (kingY + j)) == (kingColor ? KING_B : KING_W))
					return 1;

		char knightt = kingColor ? KNIGHT_B : KNIGHT_W;
		if ((kingX > 1 && kingY > 0 && accessBoardAt(board, kingX - 2 + 8 * (kingY - 1)) == knightt) || (kingX > 0 && kingY > 1 && accessBoardAt(board, kingX - 1 + 8 * (kingY - 2)) == knightt) || (kingX < 7 && kingY > 1 && accessBoardAt(board, kingX + 1 + 8 * (kingY - 2)) == knightt) || (kingX < 6 && kingY > 0 && accessBoardAt(board, kingX + 2 + 8 * (kingY - 1)) == knightt) || (kingX < 6 && kingY < 7 && accessBoardAt(board, kingX + 2 + 8 * (kingY + 1)) == knightt) || (kingX < 7 && kingY < 6 && accessBoardAt(board, kingX + 1 + 8 * (kingY + 2)) == knightt) || (kingX > 0 && kingY < 6 && accessBoardAt(board, kingX - 1 + 8 * (kingY + 2)) == knightt) || (kingX > 1 && kingY < 7 && accessBoardAt(board, kingX - 2 + 8 * (kingY + 1)) == knightt))
			return 1;

		char bishopp = kingColor ? BISHOP_B : BISHOP_W;
		char queennn = kingColor ? QUEEN_B  :  QUEEN_W;

		char pppp;
		
		for (char X = kingX + 1, Y = kingY + 1; X < 8 && Y < 8; X += 1, Y += 1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		for (char X = kingX - 1, Y = kingY + 1; X >= 0 && Y < 8; X += -1, Y += 1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		for (char X = kingX + 1, Y = kingY - 1; X >= 0 && Y < 8; X += 1, Y += -1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		for (char X = kingX - 1, Y = kingY - 1; X >= 0 && Y < 8; X += -1, Y += -1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		char rookk = kingColor ? ROOK_B : ROOK_W;

		for (char Y = kingY + 1; Y < 8; Y += 1) {
			pppp = accessBoardAt(board, kingX + Y * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		for (char Y = kingY - 1; Y >= 0; Y += -1) {
			pppp = accessBoardAt(board, kingX + Y * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		for (char X = kingX + 1; X < 8; X += 1) {
			pppp = accessBoardAt(board, X + kingY * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		for (char X = kingX - 1; X >= 0; X += -1) {
			pppp = accessBoardAt(board, X + kingY * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp)
				break;
		}

		return 0;
	}
}

static inline char pieceToNotation(unsigned char p) {
	return !p ? ' ' : p == PAWN_W ? 'P' : p == PAWN_B ? 'p' : p == KNIGHT_W ? 'N' : p == KNIGHT_B ? 'n' : p == BISHOP_W ? 'B' : p == BISHOP_B ? 'b' : p == ROOK_W ? 'R' : p == ROOK_B ? 'r' : p == QUEEN_W ? 'Q' : p == QUEEN_B ? 'q' : p == KING_W ? 'K' : p == KING_B ? 'k' : 'U';
}

static inline char pieceToLowerNotation(unsigned char p) {
	return !p ? ' ' : p == PAWN_W ? 'p' : p == PAWN_B ? 'p' : p == KNIGHT_W ? 'n' : p == KNIGHT_B ? 'n' : p == BISHOP_W ? 'b' : p == BISHOP_B ? 'b' : p == ROOK_W ? 'r' : p == ROOK_B ? 'r' : p == QUEEN_W ? 'q' : p == QUEEN_B ? 'q' : p == KING_W ? 'k' : p == KING_B ? 'k' : 'u';
}

static inline unsigned char notationToPiece(char p) {
	return p == ' ' ? BLANK : p == 'P' ? PAWN_W : p == 'p' ? PAWN_B : p == 'N' ? KNIGHT_W : p == 'n' ? KNIGHT_B : p == 'B' ? BISHOP_W : p == 'b' ? BISHOP_B : p == 'R' ? ROOK_W : p == 'r' ? ROOK_B : p == 'Q' ? QUEEN_W : p == 'q' ? QUEEN_B : p == 'K' ? KING_W : p == 'k' ? KING_B : UNKNOWN;
}

static inline unsigned char notationToWhitePiece(char p) {
	return p == ' ' ? BLANK : (p == 'P' || p == 'p') ? PAWN_W : (p == 'N' || p == 'n') ? KNIGHT_W : (p == 'B' || p == 'b') ? BISHOP_W : (p == 'R' || p == 'r') ? ROOK_W : (p == 'Q' || p == 'q') ? QUEEN_W : (p == 'K' || p == 'k') ? KING_W : UNKNOWN;
}

static inline unsigned char notationToBlackPiece(char p) {
	return p == ' ' ? BLANK : (p == 'P' || p == 'p') ? PAWN_B : (p == 'N' || p == 'n') ? KNIGHT_B : (p == 'B' || p == 'b') ? BISHOP_B : (p == 'R' || p == 'r') ? ROOK_B : (p == 'Q' || p == 'q') ? QUEEN_B : (p == 'K' || p == 'k') ? KING_B : UNKNOWN;
}

void makeForcedMove(uint64_t *board, char *brkrwrkr00, char const *args) {
	char toPiece = accessBoardAt(board, args[1]);
	char fromPiece = accessBoardAt(board, args[0]);
	setBoardAt(board, args[1], fromPiece);
	setBoardAt(board, args[0], BLANK);

	char x1 = args[0] % 8;
	char x2 = args[1] % 8;
	char isWhiteFrom = isWhite(fromPiece);
	char castled = 0;

	// En Passant
	if ((fromPiece == PAWN_W || fromPiece == PAWN_B) && !toPiece && ((args[0] - args[1]) % 8))
		setBoardAt(board, args[1] + (isBlack(fromPiece) ? -8 : 8), BLANK);
	
	// Promotion
	else if (args[2])
		setBoardAt(board, args[1], args[2]);

	// Castle
	else if ((fromPiece == KING_W || fromPiece == KING_B) && absol(args[1] - args[0]) == 2) {
		setBoardAt(board, (args[1] + args[0]) / 2, isWhiteFrom ? ROOK_W : ROOK_B);
		setBoardAt(board, isWhiteFrom ? (x2 > x1 ? 63 : 56) : (x2 > x1 ? 7 : 0), BLANK);
		castled = 1;
	}

	// Ban right to castle for king
	if (fromPiece == KING_W)
		*brkrwrkr00 &= ~8;
	else if (fromPiece == KING_B)
		*brkrwrkr00 &= ~64;

	// Ban right to castle for white rook coz of castle or normally
	if ((fromPiece == ROOK_W && args[0] == 56) || (castled && x2 < x1 && fromPiece == KING_W))
		*brkrwrkr00 &= ~16;
	else if ((fromPiece == ROOK_W && args[0] == 63) || (castled && x2 > x1 && fromPiece == KING_W))
		*brkrwrkr00 &= ~4;

	// Ban right to castle for black rook coz of castle or normally
	if ((fromPiece == ROOK_B && args[0] == 0) || (castled && x2 < x1 && fromPiece == KING_B))
		*brkrwrkr00 &= ~128;
	else if ((fromPiece == ROOK_B && args[0] == 7) || (castled && x2 > x1 && fromPiece == KING_B))
		*brkrwrkr00 &= ~32;
}

static inline char makeMove(uint64_t *board, uint64_t const *prevBoard, char *brkrwrkr00, char *args) {
	if (!validateMove(board, prevBoard, *brkrwrkr00, args)) {
		printf("Invalid!\n");
		fflush(stdout);
		return 0;
	}

	makeForcedMove(board, brkrwrkr00, args);

	return 1;
}

static inline void setBoardAt(uint64_t *board, unsigned char ind, unsigned char p) {
	if (ind < 64)
		board[ind / 16] = (board[ind / 16] & ~((uint64_t) 15 << (60 - (ind % 16) * 4))) | (uint64_t) p << (60 - (ind % 16) * 4);
}

uint64_t *newChessBoard() {
	uint64_t *board = malloc(sizeof(uint64_t) * 4);
	memset(board, 0, 4 * sizeof(uint64_t));

	setBoardAt(board, 0, ROOK_B);
	setBoardAt(board, 1, KNIGHT_B);
	setBoardAt(board, 2, BISHOP_B);
	setBoardAt(board, 3, QUEEN_B);
	setBoardAt(board, 4, KING_B);
	setBoardAt(board, 5, BISHOP_B);
	setBoardAt(board, 6, KNIGHT_B);
	setBoardAt(board, 7, ROOK_B);

	int i;
	for (i = 8; i < 16; i++)
		setBoardAt(board, i, PAWN_B);
	for (i = 48; i < 56; i++)
		setBoardAt(board, i, PAWN_W);

	setBoardAt(board, 56, ROOK_W);
	setBoardAt(board, 57, KNIGHT_W);
	setBoardAt(board, 58, BISHOP_W);
	setBoardAt(board, 59, QUEEN_W);
	setBoardAt(board, 60, KING_W);
	setBoardAt(board, 61, BISHOP_W);
	setBoardAt(board, 62, KNIGHT_W);
	setBoardAt(board, 63, ROOK_W);

	return board;
}

static inline unsigned char chessPosToIndex(char const *p) {
	return (p[0] - 'a' + 1) + (8 - p[1] + '0') * 8 - 1;
}

static inline char *uciToIndices(uint64_t const *board, char const *uci) {
	char *ret = malloc(3);
	ret[0] = chessPosToIndex((char [2]) {uci[0], uci[1]});
	ret[1] = chessPosToIndex((char [2]) {uci[2], uci[3]});
	ret[2] = uci[4] ? (isWhite(accessBoardAt(board, ret[0])) ? notationToWhitePiece(uci[4]) : notationToBlackPiece(uci[4])) : 0;
	return ret;
}

static inline char *indicesToUci(char const *indices) {
	char *ret = malloc(6);
	ret[0] = (indices[0] % 8) + 'a';
	ret[1] = '8' - (indices[0] / 8);
	ret[2] = (indices[1] % 8) + 'a';
	ret[3] = '8' - (indices[1] / 8);
	ret[4] = indices[2] ? pieceToLowerNotation(indices[2]) : 0;
	ret[5] = 0;
	return ret;
}

int evaluateNode(struct node n) {
	if (!n.len && n.isnotleafnode) {
		if (isCheckOnKing(n.pos, !n.color))
			return (n.color - !n.color) * -64000;
		else if (isCheckOnKing(n.pos, n.color))
			return (n.color - !n.color) * 64000;
	}

	int posAdv = 0;

	for (char i = 0, p; i < 64; i++) {
		p = accessBoardAt(n.pos, i);
		posAdv += p == PAWN_B ? -10 : p == PAWN_W ? 10 : p == KNIGHT_B ? -29 : p == KNIGHT_W ? 29 : p == BISHOP_B ? -30 : p == BISHOP_W ? 30 : p == ROOK_B ? -50 : p == ROOK_W ? 50 : p == QUEEN_B ? -100 : p == QUEEN_W ? 100 : 0;
	}

	// +1 for castle
	return posAdv + ((n.move[0] == KING_W || n.move[0] == KING_B) && absol(n.move[1] - n.move[0]) == 2) * -(!n.color);
}

#endif /*__CHESS_BASICS__*/