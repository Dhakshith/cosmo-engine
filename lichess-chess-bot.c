#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>
#include "JSON Parser/JSON.h"
#include "Chess/basics.h"

#define AUTHORIZATION "Authorization: Bearer KOdnd7Ny0eMQWWyx"

static char myColor = 0; // white = 0, black = 1
static char setMyColor = 0;
static uint64_t *board;
static uint64_t *prevBoard;
static char brkrwr00 = 0xfc;
static char *myLichessId;

static inline size_t emptycallback(char *t, size_t u, size_t v, void *w) {
	return v;
}

static inline unsigned int spaces(char *str) {
	unsigned int i = 0;

	while (*str != 0)
		i += *(str++) == ' ';

	return i;
}

unsigned int intlen(int i) {
	if (!i) return 1;
	unsigned int n = 0;
	unsigned int m = i;
	do {
		m = i;
		i /= 10;
		m -= i;
		n++;
	} while (m > 0);
	return n - 1;
};

size_t playGame(char *gameState, size_t size, size_t nmemb, void *gameId) {
	if (nmemb > 1) {
		char *str = malloc(nmemb + 1);
		unsigned int ndone;
		for (ndone = 0; ndone < nmemb; ndone++)
			str[ndone] = gameState[ndone];
		str[ndone] = 0;
		JSON *json = NULL;
		parseJSON(str, &json);
		printJSON(json, 4, 0);
		free(str);
		printf("\n");
		fflush(stdout);
		str = JSONGetValueForKey("type", json).str;
		char gameFull = !strcmp(str, "gameFull");
		char gameState = !strcmp(str, "gameState");
		if (!gameFull && !gameState)
			return nmemb;
		char *moves = gameFull ? JSONGetValueForKey("moves", JSONGetValueForKey("state", json).json).str : JSONGetValueForKey("moves", json).str;
		
		if (!setMyColor && gameFull) {
			setMyColor = 1;
			myColor = !strcmp(JSONGetValueForKey("id", JSONGetValueForKey("white", json).json).str, myLichessId);
		}

		// int btime, wtime, binc, winc;

		// if (gameFull) {
		// 	btime = wtime = JSONGetValueForKey("initial", JSONGetValueForKey("clock", json).json).number;
		// 	binc = winc = JSONGetValueForKey("increment", JSONGetValueForKey("clock", json).json).number;
		// } else {
		// 	btime = JSONGetValueForKey("btime", json).number;
		// 	wtime = JSONGetValueForKey("wtime", json).number;
		// 	binc = JSONGetValueForKey("binc", json).number;
		// 	winc = JSONGetValueForKey("winc", json).number;
		// }

		if (!moves)
			moves = " ";
		else {
			size_t strlenn = strlen(moves);
			char beforaf = strlenn < 5 || moves[strlenn - 5] == ' ';
			char *indicess = uciToIndices((char [6]) {moves[strlenn - (5 - beforaf)], moves[strlenn - 4 + beforaf], moves[strlenn - 3 + beforaf], moves[strlenn - 2 + beforaf], moves[strlenn - (!beforaf)], 0});
			memcpy(prevBoard, board, 32);
			makeForcedMove(board, &brkrwr00, indicess);
			printf("%s: Move from opp\n", (char [6]) {moves[strlenn - (5 - beforaf)], moves[strlenn - 4 + beforaf], moves[strlenn - 3 + beforaf], moves[strlenn - 2 + beforaf], moves[strlenn - (!beforaf)], 0});
			printBoard(board);
			free(indicess);
		}

		printf("myc: %d, spaces: %d, sp%%2: %d\n", myColor, spaces(moves), spaces(moves) % 2);
		fflush(stdout);
		if (spaces(moves) % 2 == !!myColor) {
			char *q; // = malloc(75 + intlen((int) btime) + intlen((int) wtime) + intlen((int) binc) + intlen((int) winc) + strlen(moves));
			// sprintf(q, "printf \"position startpos move %s\\ngo btime %d wtime %d binc %d winc %d\\n\"|./engine", moves, btime, wtime, binc, winc);
			// FILE *stockfish = popen(q, "r");
			// printf("%s\n", q);
			// free(q);
			// char *ss = NULL;
			// char c;
			// while (1) {
			// 	while ((c = fgetc(stockfish)) != '\n' && c != ' ') {
			// 		appendToString(&ss, c);
			// 	}
			// 	if (!strcmp(ss, "bestmove")) {
			// 		free(ss);
			// 		break;
			// 	} else {
			// 		printf("%s\n", ss);
			// 		free(ss);
			// 		ss = NULL;
			// 	}
			// }
			// char *bestmove = NULL;
			// char ccc;
			// while ((ccc = fgetc(stockfish)) != ' ' && ccc != '\n')
			// 	appendToString(&bestmove, ccc);
			// pclose(stockfish);
			char *valids = 0;
			unsigned long _len_ = validMoves(board, prevBoard, brkrwr00, myColor, &valids);
			if (!_len_) {
				printf("Checkmate\n");
				return nmemb;
			}
			q = malloc(45 + strlen((char *) gameId));
			int rnd = ((float) rand()) / RAND_MAX * (_len_ / 3 - 1);
			char indicess[3] = {valids[rnd * 3], valids[rnd * 3 + 1], valids[rnd * 3 + 2]};
			char *bestmove = indicesToUci(indicess);
			sprintf(q, "https://lichess.org/api/bot/game/%s/move/%s", (char *) gameId, bestmove);
			// memcpy(prevBoard, board, 32);
			// makeForcedMove(board, &brkrwr00, indicess);
			// printf("After doing the best move (%s):\n", bestmove);
			// printBoard(board);
			free(valids);
			free(bestmove);

			CURL *curl = curl_easy_init();
			CURLcode res;

			struct curl_slist *chunk = NULL;
			chunk = curl_slist_append(chunk, AUTHORIZATION);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

			#ifdef SKIP_PEER_VERIFICATION
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			#endif

			#ifdef SKIP_HOSTNAME_VERIFICATION
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			#endif

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, emptycallback);
			curl_easy_setopt(curl, CURLOPT_URL, q);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
			free(q);

			res = curl_easy_perform(curl);
			if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed (in playgame): %s\n", curl_easy_strerror(res));

			curl_easy_cleanup(curl);
			curl_slist_free_all(chunk);
		}
	}

	return nmemb;
}

size_t callback(char *actualStr, size_t size, size_t nmemb, void *usrdata) {
	if (nmemb > 1) {
		char *str = malloc(nmemb + 1);
		unsigned int ndone;
		for (ndone = 0; ndone < nmemb; ndone++)
			str[ndone] = actualStr[ndone];
		str[ndone] = 0;
		JSON *json = NULL;
		parseJSON(str, &json);
		printJSON(json, 4, 0);
		printf("\n");
		fflush(stdout);
		free(str);
		int ind = JSONIndexOf("type", json);
		if (ind == -1) {
			freeJSON(json);
			return nmemb;
		} else {
			CURL *curl = curl_easy_init();
			CURLcode res;
			if (curl) {
				struct curl_slist *chunk = NULL;
				chunk = curl_slist_append(chunk, AUTHORIZATION);
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

				#ifdef SKIP_PEER_VERIFICATION
					curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
				#endif

				#ifdef SKIP_HOSTNAME_VERIFICATION
					curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
				#endif

				char *type = json->contents[ind].str;
				char challenge = !strcmp(type, "challenge");
				char gameStart = !strcmp(type, "gameStart");

				if (challenge) {
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, emptycallback);
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

					setMyColor = 0;
					char *gameId = JSONGetValueForKey("id", JSONGetValueForKey("challenge", json).json).str;
					char *s = malloc(42 + strlen(gameId));
					sprintf(s, "https://lichess.org/api/challenge/%s/accept", gameId);
					curl_easy_setopt(curl, CURLOPT_URL, s);
					free(s);
					res = curl_easy_perform(curl);
					if(res != CURLE_OK)
						fprintf(stderr, "curl_easy_perform() failed (in callback[0]): %s\n", curl_easy_strerror(res));
				} else if (gameStart) {
					char *gameId = JSONGetValueForKey("id", JSONGetValueForKey("game", json).json).str;

					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, playGame);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, gameId);

					char *s = malloc(41 + strlen(gameId));
					sprintf(s, "https://lichess.org/api/bot/game/stream/%s", gameId);
					
					curl_easy_setopt(curl, CURLOPT_URL, s);
					free(s);

					res = curl_easy_perform(curl);
					if(res != CURLE_OK)
						fprintf(stderr, "curl_easy_perform() failed (in callback[1]): %s\n", curl_easy_strerror(res));
				}

				curl_easy_cleanup(curl);
				curl_slist_free_all(chunk);
			}
		}
		freeJSON(json);
	}

	return nmemb;
}

size_t setMyLichessId(char *actualStr, size_t size, size_t nmemb, void *usrdata) {
	JSON *json = NULL;
	char *str = malloc(nmemb + 1);
	unsigned int ndone;
	for (ndone = 0; ndone < nmemb; ndone++)
		str[ndone] = actualStr[ndone];
	str[ndone] = 0;
	parseJSON(str, &json);
	char *tmpPointer = JSONGetValueForKey("id", json).str;
	myLichessId = malloc(strlen(tmpPointer) + 1);
	strcpy(myLichessId, tmpPointer);
	freeJSON(json);
	return nmemb;
}

int main(void) {
	srand(time(0));

	board = newChessBoard();
	prevBoard = malloc(4 * sizeof(uint64_t));

	CURL *curl = curl_easy_init();
	CURLcode res;

	if (!curl) {
		fprintf(stderr, "curl_easy_init failed\n");
		return -1;
	}

	struct curl_slist *chunk = NULL;

	chunk = curl_slist_append(chunk, AUTHORIZATION);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	curl_easy_setopt(curl, CURLOPT_URL, "https://lichess.org/api/account");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, setMyLichessId);

	#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	#endif

	#ifdef SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	#endif

	res = curl_easy_perform(curl);

	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed (in main): %s\n", curl_easy_strerror(res));

	curl_easy_setopt(curl, CURLOPT_URL, "https://lichess.org/api/stream/event");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

	res = curl_easy_perform(curl);

	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed (in main 2nd part): %s\n", curl_easy_strerror(res));

	curl_easy_cleanup(curl);
	curl_slist_free_all(chunk);

	return 0;
}
