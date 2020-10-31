#ifndef __CHESS_BASICS__
#define __CHESS_BASICS__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define abs(x) ((x) >= 0 ? (x) : -(x))

// A piece = 4 bits
// black = 0, white = 1

typedef enum piece {
	BLANK,
	PAWN_W,
	KNIGHT_W,
	BISHOP_W,
	ROOK_W,
	QUEEN_W,
	KING_W,
	PAWN_B,
	KNIGHT_B,
	BISHOP_B,
	ROOK_B,
	QUEEN_B,
	KING_B,
	UNKNOWN
} piece;

uint64_t *newChessBoard();
static inline unsigned char accessBoardAt(uint64_t const *, unsigned char);
static inline void setBoardAt(uint64_t *, unsigned char, unsigned char);
static inline void makeMove(uint64_t *, uint64_t const *, char *, char *); // char * (3nd arg) -> {from, to, promotionPiece or 0}
static inline void makeForcedMove(uint64_t *, char *, char const *); // make move without validating
char validateMove(uint64_t const *, uint64_t const *, char, char const *);
static inline char pieceToNotation(unsigned char);
static inline unsigned char notationToPiece(char);
void printBoard(uint64_t const *);
void printValidMoves(uint64_t const *, uint64_t const *, char, char);
unsigned long validMoves(uint64_t const *, uint64_t const *, char, char, char **);
static inline char *uciToIndices(char const *);
static inline char *indicesToUci(char const *);
static inline char isBlack(unsigned char);
static inline char isWhite(unsigned char);
static inline unsigned char chessPosToIndex(char const *);
char isCheckOnKing(uint64_t const *, char /*king color: 0 - black, 1 - white*/);
char isCheckOnXY(uint64_t const *, char, char, char);

void printBoard(uint64_t const *board) {
	unsigned char i;
	for (i = 0; i < 64; i++) {
		printf("%c", pieceToNotation(accessBoardAt(board, i)));
		if ((i + 1) % 8 == 0) printf("\n");
	}
	fflush(stdout);
}

static inline unsigned char accessBoardAt(uint64_t const *board, unsigned char ind) {
	if (ind >= 64)
		return UNKNOWN;
	return (board[ind / 16] >> ((15 - (ind % 16)) * 4)) & 15;
}

static inline char isBlack(unsigned char p) {
	return p >= PAWN_B && p <= KING_B;
}

static inline char isWhite(unsigned char p) {
	return p >= PAWN_W && p <= KING_W;
}

char validateMove(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char const *move) {
	if (move[0] == move[1])
		return 0;
	if (move[0] < 0 || move[0] > 64 || move[1] < 0 || move[1] > 64)
		return 0;

	unsigned char fromPiece = accessBoardAt(board, move[0]), toPiece = accessBoardAt(board, move[1]);
	unsigned char isBlackFrom = isBlack(fromPiece), isBlackTo = isBlack(toPiece);
	unsigned char isWhiteFrom = isWhite(fromPiece), isWhiteTo = isWhite(toPiece);
	
	if (fromPiece == BLANK)
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
				valid = !move[2] && accessBoardAt(board, move[1]) == BLANK && (isBlackFrom ? (move[0] >= 8 && move[0] <= 15 && accessBoardAt(board, move[0] + 8) == BLANK) : (move[0] >= 48 && move[0] <= 55 && accessBoardAt(board, move[0] - 8) == BLANK));
			else if ((move[1] == (move[0] + (isBlackFrom ? 7 : -7)) || move[1] == (move[0] + (isBlackFrom ? 9 : -9))) && abs(move[0] / 8 - move[1] / 8) == 1) {
				if (isBlackFrom ? (move[1] >= 56 && move[1] <= 63) : (move[1] >= 0 && move[1] <= 7))
					valid = move[2] == (isBlackFrom ? QUEEN_B : QUEEN_W) || move[2] == (isBlackFrom ? KNIGHT_B : KNIGHT_W) || move[2] == (isBlackFrom ? BISHOP_B : BISHOP_W) || move[2] == (isBlackFrom ? ROOK_B : ROOK_W);
				else if (!move[2] && ((isBlackFrom && isWhiteTo) || (isWhiteFrom && isBlackTo)))
					valid = 1;
				else if (!move[2] && (isBlackFrom ? (move[0] >= 32 && move[0] <= 39) : (move[0] >= 24 && move[0] <= 31)) && accessBoardAt(board, move[1] + (isBlackFrom ? -8 : 8)) == (isWhiteFrom ? PAWN_B : PAWN_W) && accessBoardAt(prevBoard, move[1] + (isBlackFrom ? -8 : 8)) == BLANK && accessBoardAt(prevBoard, move[1]) == BLANK)
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

			if (move[2] != 0 || abs(x2 - x1) != abs(y2 - y1))
				valid = 0;
			else {
				valid = 1;

				char x2gtx1 = (x2 > x1 ? 1 : -1);
				char y2gty1 = (y2 > y1 ? 1 : -1);
				for (char X = x1 + x2gtx1, Y = y1 + y2gty1; (x2 > x1 ? X < x2 : X > x2) && (y2 > y1 ? Y < y2 : Y > y2); X += x2gtx1, Y += y2gty1)
					if (accessBoardAt(board, X + Y * 8) != BLANK) {
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
					if (accessBoardAt(board, x1 + Y * 8) != BLANK) {
						valid = 0;
						break;
					}
			} else {
				char x2gtx1 = x2 > x1 ? 1 : -1;
				for (char X = x1 + x2gtx1; x2 > x1 ? X < x2 : X > x2; X += x2gtx1)
					if (accessBoardAt(board, X + y1 * 8) != BLANK) {
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

			if (move[2] != 0 || (abs(x2 - x1) != abs(y2 - y1) && x1 != x2 && y1 != y2))
				valid = 0;
			else if (abs(x2 - x1) == abs(y2 - y1)) {
				char x2gtx1 = x2 > x1 ? 1 : -1;
				char y2gty1 = y2 > y1 ? 1 : -1;
				for (char X = x1 + x2gtx1, Y = y1 + y2gty1; (x2 > x1 ? X < x2 : X > x2) && (y2 > y1 ? Y < y2 : Y > y2); X += x2gtx1, Y += y2gty1)
					if (accessBoardAt(board, X + Y * 8) != BLANK) {
						valid = 0;
						break;
					}
			} else if (x1 == x2) {
				char y2gty1 = y2 > y1 ? 1 : -1;
				for (char Y = y1 + y2gty1; y2 > y1 ? Y < y2 : Y > y2; Y += y2gty1)
					if (accessBoardAt(board, x1 + Y * 8) != BLANK) {
						valid = 0;
						break;
					}
			} else {
				char x2gtx1 = x2 > x1 ? 1 : -1;
				for (char X = x1 + x2gtx1; x2 > x1 ? X < x2 : X > x2; X += x2gtx1)
					if (accessBoardAt(board, X + y1 * 8) != BLANK) {
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

			valid = move[2] == 0 && ((abs(x2 - x1) == 1 || x2 - x1 == 0) && (abs(y2 - y1) == 1 || y2 - y1 == 0));

			if (!valid && y1 == (isWhiteFrom ? 7 : 0) && y2 == y1 && abs(x2 - x1) == 2 && ((brkrwrkr00 >> (isBlackFrom ? 6 : 3)) & 1) && ((brkrwrkr00 >> (x2 > x1 ? (isBlackFrom ? 5 : 2) : (isBlackFrom ? 7 : 4))) & 1) && !isCheckOnXY(board, isWhiteFrom, x1, y1) && accessBoardAt(board, (move[0] + move[1]) / 2) == BLANK && accessBoardAt(board, move[1]) == BLANK && accessBoardAt(board, (isBlackFrom && x2 < x1) ? 0 : isBlackFrom ? 7 : (isWhiteFrom && x2 < x1) ? 56 : 63) == (isBlackFrom ? ROOK_B : ROOK_W) && (x2 < x1 ? (accessBoardAt(board, isBlackFrom ? 1 : 57) == BLANK) : 1) && !isCheckOnXY(board, isWhiteFrom, (x1 + x2) / 2, y1) && !isCheckOnXY(board, isWhiteFrom, x2, y2))
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

unsigned long validMoves(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char isWhiteYourColor, char **retAddr) {
	if (!isWhiteYourColor)
		isWhiteYourColor = -1;
	else
		isWhiteYourColor = 1;

	*retAddr = 0;
	unsigned long _len = 0;
	
	for (char i = 0; i < 64; i++)
		for (char j = 0; j < 64; j++) {
			char piese = accessBoardAt(board, i);
			char colorOfPiece = isWhite(piese) ? 1 : isBlack(piese) ? -1 : 0;

			if (!piese || isWhiteYourColor != colorOfPiece)
				break;
			
			#define PRINTIFVALID() ({if (validateMove(board,prevBoard,brkrwrkr00,mv)){*retAddr=realloc(*retAddr, _len += 3);(*retAddr)[_len-3]=mv[0];(*retAddr)[_len-2]=mv[1];(*retAddr)[_len-1]=mv[2];}})
			char mv[3] = {i, j, 0};
			PRINTIFVALID();
			mv[2] = QUEEN_W;
			PRINTIFVALID();
			mv[2] = QUEEN_B;
			PRINTIFVALID();
			mv[2] = KNIGHT_W;
			PRINTIFVALID();
			mv[2] = KNIGHT_B;
			PRINTIFVALID();
			mv[2] = BISHOP_W;
			PRINTIFVALID();
			mv[2] = BISHOP_B;
			PRINTIFVALID();
		}

	return _len;
}

void printValidMoves(uint64_t const *board, uint64_t const *prevBoard, char brkrwrkr00, char isWhiteYourColor) {
	char *ret = 0;
	unsigned long len = validMoves(board, prevBoard, brkrwrkr00, isWhiteYourColor, &ret);
	for (unsigned long i = 0; i < len; i += 3) {
		char *strr = indicesToUci((char [3]) {ret[i], ret[i + 1], ret[i + 2]});
		printf("%s\n", strr);
		free(strr);
	}
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
			else if (pppp != BLANK)
				break;
		}

		for (char X = kingX - 1, Y = kingY + 1; X >= 0 && Y < 8; X += -1, Y += 1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		for (char X = kingX + 1, Y = kingY - 1; X >= 0 && Y < 8; X += 1, Y += -1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		for (char X = kingX - 1, Y = kingY - 1; X >= 0 && Y < 8; X += -1, Y += -1) {
			pppp = accessBoardAt(board, X + Y * 8);
			if (pppp == bishopp || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		char rookk = kingColor ? ROOK_B : ROOK_W;

		for (char Y = kingY + 1; Y < 8; Y += 1) {
			pppp = accessBoardAt(board, kingX + Y * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		for (char Y = kingY - 1; Y >= 0; Y += -1) {
			pppp = accessBoardAt(board, kingX + Y * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		for (char X = kingX + 1; X < 8; X += 1) {
			pppp = accessBoardAt(board, X + kingY * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		for (char X = kingX - 1; X >= 0; X += -1) {
			pppp = accessBoardAt(board, X + kingY * 8);
			if (pppp == rookk || pppp == queennn)
				return 1;
			else if (pppp != BLANK)
				break;
		}

		return 0;
	}
}

static inline char pieceToNotation(unsigned char p) {
	return p == BLANK ? ' ' : p == PAWN_W ? 'P' : p == PAWN_B ? 'p' : p == KNIGHT_W ? 'N' : p == KNIGHT_B ? 'n' : p == BISHOP_W ? 'B' : p == BISHOP_B ? 'b' : p == ROOK_W ? 'R' : p == ROOK_B ? 'r' : p == QUEEN_W ? 'Q' : p == QUEEN_B ? 'q' : p == KING_W ? 'K' : p == KING_B ? 'k' : 'U';
}

static inline unsigned char notationToPiece(char p) {
	return p == ' ' ? BLANK : p == 'P' ? PAWN_W : p == 'p' ? PAWN_B : p == 'N' ? KNIGHT_W : p == 'n' ? KNIGHT_B : p == 'B' ? BISHOP_W : p == 'b' ? BISHOP_B : p == 'R' ? ROOK_W : p == 'r' ? ROOK_B : p == 'Q' ? QUEEN_W : p == 'q' ? QUEEN_B : p == 'K' ? KING_W : p == 'k' ? KING_B : UNKNOWN;
}

static inline void makeForcedMove(uint64_t *board, char *brkrwrkr00, char const *args) {
	char toPiece = accessBoardAt(board, args[1]);
	char fromPiece = accessBoardAt(board, args[0]);
	setBoardAt(board, args[1], fromPiece);
	setBoardAt(board, args[0], BLANK);

	char x1 = args[0] % 8;
	char y1 = args[0] / 8;
	char x2 = args[1] % 8;
	char y2 = args[1] / 8;
	char isBlackFrom = isBlack(fromPiece);
	char isWhiteFrom = isWhite(fromPiece);
	char castled = 0;

	// En Passant
	if ((fromPiece == PAWN_W || fromPiece == PAWN_B) && toPiece == BLANK && ((args[0] - args[1]) % 8))
		setBoardAt(board, args[1] + (isBlack(fromPiece) ? -8 : 8), BLANK);
	
	// Promotion
	else if (args[2])
		setBoardAt(board, args[1], args[2]);

	// Castle
	else if ((fromPiece == KING_W || fromPiece == KING_B) && abs(x2 - x1) == 2 && y1 == y2 && y1 == (isBlackFrom ? 0 : 7)) {
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

static inline void makeMove(uint64_t *board, uint64_t const *prevBoard, char *brkrwrkr00, char *args) {
	if (!validateMove(board, prevBoard, *brkrwrkr00, args)) {
		printf("Inv\n");
		return;
	}
	makeForcedMove(board, brkrwrkr00, args);
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

static inline char *uciToIndices(char const *uci) {
	char *ret = malloc(3);
	ret[0] = chessPosToIndex((char [2]) {uci[0], uci[1]});
	ret[1] = chessPosToIndex((char [2]) {uci[2], uci[3]});
	ret[2] = uci[4] ? notationToPiece(uci[4]) : 0;
	return ret;
}

static inline char *indicesToUci(char const *indices) {
	char *ret = malloc(6);
	ret[0] = (indices[0] % 8) + 'a';
	ret[1] = '8' - (indices[0] / 8);
	ret[2] = (indices[1] % 8) + 'a';
	ret[3] = '8' - (indices[1] / 8);
	ret[4] = indices[2] ? pieceToNotation(indices[2]) : 0;
	ret[5] = 0;
	return ret;
}

#endif /*__CHESS_BASICS__*/
