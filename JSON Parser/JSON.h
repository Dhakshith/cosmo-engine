#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define isnumeric(c) (c >= '0' && c <= '9')

struct JSON;
struct Array;

enum TYPE_T {STRING, NUMBER, OBJECT, ARRAY, TRUEORFALSE, NONE};

typedef struct ArrayContent {
	enum TYPE_T type;
	union {
		char *str;
		double number;
		struct JSON *json;
		struct Array *array;
		char trueorfalse;
		char none;
	};
} ArrayContent;

typedef struct JSONContent {
	char *name;
	enum TYPE_T type;
	union {
		char *str;
		double number;
		struct JSON *json;
		struct Array *array;
		char trueorfalse;
		char none;
	};
} JSONContent;

typedef struct Array {
	unsigned int length;
	struct ArrayContent* contents;
} Array;

typedef struct JSON {
	unsigned int length;
	struct JSONContent* contents;
} JSON;

unsigned int printArray(Array const *, unsigned int, unsigned int);
unsigned int printJSON(JSON const *, unsigned int, unsigned int);
unsigned int parseArray(char const *, Array **);
unsigned int parseJSON(char const *, JSON **);
static inline unsigned int parseString(char const *, char **);
static inline int JSONIndexOf(char const *, JSON const *);
static inline char *appendStringToString(char **, char const *);
static inline char *appendToString(char **, char);
static inline void freeJSON(JSON *);
static inline JSONContent JSONGetValueForKey(char const *, JSON const *);

static inline int JSONIndexOf(char const *str, JSON const *json) {
	unsigned int i;

	for (i = 0; i < json->length; i++)
		if (!strcmp(json->contents[i].name, str)) return i;

	return -1;
}

static inline JSONContent JSONGetValueForKey(char const *str, JSON const *json) {
	// No error checking implemented
	return json->contents[JSONIndexOf(str, json)];
}

unsigned int printArray(Array const *arr, unsigned int indent, unsigned int depth) {
	if (!arr) return 0;

	printf("[\n");

	unsigned int i;
	for (i = 0; i < arr->length; i++) {
		if (!arr->contents) return 0;
		unsigned int j;
		for (j = 0; j < indent * (depth + 1); j++)
			printf(" ");
		switch (arr->contents[i].type) {
			case STRING: {
				printf("\"%s\"", arr->contents[i].str);
			} break;
			case NUMBER: {
				printf("%lg", arr->contents[i].number);
			} break;
			case OBJECT: {
				printJSON(arr->contents[i].json, indent, depth + 1);
			} break;
			case ARRAY: {
				printArray(arr->contents[i].array, indent, depth + 1);
			} break;
			case TRUEORFALSE: {
				printf(arr->contents[i].trueorfalse ? "true" : "false");
			} break;
			case NONE: {
				printf("null");
			} break;
		}

		if (i < arr->length - 1) printf(",");
		printf("\n");
	}

	unsigned int j;
	for (j = 0; j < indent * depth; j++)
		printf(" ");
	printf("]");

	return 1;
}

unsigned int printJSON(JSON const *json, unsigned int indent, unsigned int depth) {
	if (!json) return 0;

	printf("{\n");

	unsigned int i;
	for (i = 0; i < json->length; i++) {
		if (!json->contents) return 0;
		unsigned int j;
		for (j = 0; j < indent * (depth + 1); j++)
			printf(" ");
		printf("\"%s\": ", json->contents[i].name);
		switch (json->contents[i].type) {
			case STRING: {
				printf("\"%s\"", json->contents[i].str);
			} break;
			case NUMBER: {
				printf("%lg", json->contents[i].number);
			} break;
			case OBJECT: {
				printJSON(json->contents[i].json, indent, depth + 1);
			} break;
			case ARRAY: {
				printArray(json->contents[i].array, indent, depth + 1);
			} break;
			case TRUEORFALSE: {
				printf(json->contents[i].trueorfalse ? "true" : "false");
			} break;
			case NONE: {
				printf("null");
			} break;
		}

		if (i < json->length - 1) printf(",");
		printf("\n");
	}

	unsigned int j;
	for (j = 0; j < indent * depth; j++)
		printf(" ");
	printf("}");

	return 1;
}

static inline unsigned int parseString(char const *jsonStr, char **str) {
	int i = 0;
	*str = NULL;
	char escape = 0, lastEscape = 0;

	while (jsonStr[i++] != '"');
	while (1) {
		lastEscape = escape;
		escape = jsonStr[i] == '\\' && !lastEscape;
		if (jsonStr[i] == '"' && !lastEscape)
			break;
		appendToString(str, jsonStr[i++]);
	}

	return i;
}

unsigned int parseArray(char const *arrStr, Array **arr) {
	unsigned int i = 0;

	while (arrStr[i++] != '[');
	
	*arr = malloc(sizeof(Array));
	memset(*arr, 0, sizeof(Array));
	while (1) {
		while (arrStr[i] != '"' && arrStr[i] != 'n' && arrStr[i] != 't' && arrStr[i] != 'f' && arrStr[i] != '.' && arrStr[i] != '-' && !isnumeric(arrStr[i]) && arrStr[i] != '[' && arrStr[i] != '{' && arrStr[i] != ']') i++;
		if (arrStr[i] == ']')
			break;

		(*arr)->length++;
		(*arr)->contents = realloc((*arr)->contents, (*arr)->length * sizeof(ArrayContent));
		
		memset((*arr)->contents + ((*arr)->length - 1), 0, sizeof(JSONContent));

		switch (arrStr[i]) {
			case '"': {
				(*arr)->contents[(*arr)->length - 1].type = STRING;
				i += parseString(arrStr + i, &(*arr)->contents[(*arr)->length - 1].str) + 1;
			} break;
			case '{': {
				(*arr)->contents[(*arr)->length - 1].type = OBJECT;
				i += parseJSON(arrStr + i, &((*arr)->contents[(*arr)->length - 1].json)) + 1;
			} break;
			case '[': {
				(*arr)->contents[(*arr)->length - 1].type = ARRAY;
				i += parseArray(arrStr + i, &((*arr)->contents[(*arr)->length - 1].array)) + 1;
			} break;
			case 't': {
				i += 4;
				(*arr)->contents[(*arr)->length - 1].type = TRUEORFALSE;
				(*arr)->contents[(*arr)->length - 1].trueorfalse = 1;
			} break;
			case 'f': {
				i += 5;
				(*arr)->contents[(*arr)->length - 1].type = TRUEORFALSE;
				(*arr)->contents[(*arr)->length - 1].trueorfalse = 0;
			} break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0': case '.': case '-': {
				(*arr)->contents[(*arr)->length - 1].type = NUMBER;
				int num = 0;
				char minus = (arrStr[i] == '-');
				i += minus;
				if (arrStr[i] != '.')
					while (isnumeric(arrStr[i]))
						num = num * 10 + arrStr[i++] - '0';
				if (arrStr[i] == '.') {
					i++;
					int exp = 1;
					while (isnumeric(arrStr[i]))
						num += (arrStr[i++] - '0') / (double) (exp *= 10);
				}
				num *= ((signed char) minus) * -2 + 1; // negate if required
				(*arr)->contents[(*arr)->length - 1].number = num;
			} break;
			case 'n': {
				i += 4;
				(*arr)->contents[(*arr)->length - 1].type = NONE;
				(*arr)->contents[(*arr)->length - 1].none = 1;
			} break;
		}
	}

	return i;
}

unsigned int parseJSON(char const *jsonStr, JSON **json) {
	unsigned int i = 0;

	while (jsonStr[i++] != '{');

	*json = malloc(sizeof(JSON));
	memset(*json, 0, sizeof(JSON));
	while (1) {
		while (jsonStr[i] != '"' && jsonStr[i] != '}') i++;
		if (jsonStr[i] == '}')
			break;
		(*json)->length++;
		(*json)->contents = realloc((*json)->contents, (*json)->length * sizeof(JSONContent));
		
		memset((*json)->contents + ((*json)->length - 1), 0, sizeof(JSONContent));

		i += parseString(jsonStr + i, &((*json)->contents[(*json)->length - 1].name)) + 1;

		while (jsonStr[i] != ':') i++;
		while (jsonStr[i] != '{' && jsonStr[i] != '[' && jsonStr[i] != '"' && jsonStr[i] != 't' && jsonStr[i] != 'f' && jsonStr[i] != 'n' && !isnumeric(jsonStr[i]) && jsonStr[i] != '.' && jsonStr[i] != '-') i++;

		switch (jsonStr[i]) {
			case '"': {
				(*json)->contents[(*json)->length - 1].type = STRING;
				i += parseString(jsonStr + i, &(*json)->contents[(*json)->length - 1].str) + 1;
			} break;
			case '{': {
				(*json)->contents[(*json)->length - 1].type = OBJECT;
				i += parseJSON(jsonStr + i, &((*json)->contents[(*json)->length - 1].json)) + 1;
			} break;
			case '[': {
				(*json)->contents[(*json)->length - 1].type = ARRAY;
				i += parseArray(jsonStr + i, &((*json)->contents[(*json)->length - 1].array)) + 1;
			} break;
			case 't': {
				i += 4;
				(*json)->contents[(*json)->length - 1].type = TRUEORFALSE;
				(*json)->contents[(*json)->length - 1].trueorfalse = 1;
			} break;
			case 'f': {
				i += 5;
				(*json)->contents[(*json)->length - 1].type = TRUEORFALSE;
				(*json)->contents[(*json)->length - 1].trueorfalse = 0;
			} break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0': case '.': case '-': {
				(*json)->contents[(*json)->length - 1].type = NUMBER;
				double num = 0;
				char minus = (jsonStr[i] == '-');
				i += minus;
				if (jsonStr[i] != '.')
					while (isnumeric(jsonStr[i]))
						num = num * 10 + jsonStr[i++] - '0';
				if (jsonStr[i] == '.') {
					i++;
					int exp = 1;
					while (isnumeric(jsonStr[i]))
						num += (jsonStr[i++] - '0') / (double) (exp *= 10);
				}
				num *= ((signed char) minus) * -2 + 1; // negate if required
				(*json)->contents[(*json)->length - 1].number = num;
			} break;
			case 'n': {
				i += 4;
				(*json)->contents[(*json)->length - 1].type = NONE;
				(*json)->contents[(*json)->length - 1].none = 1;
			} break;
		}

		while (jsonStr[i] != ',' && jsonStr[i] != '}') i++;

		if (jsonStr[i] == '}')
			break;

		i++;
	}

	return i;
}

static inline char *appendStringToString(char **str, const char *s) {
	unsigned int len2 = strlen(s);
	if (!*str) {
		 *str = malloc(1);
		**str = 0;
	}

	unsigned int len = strlen(*str);
	*str = realloc(*str, len + len2 + 1);

	unsigned int i;
	for (i = len; i < len + len2; i++)
		(*str)[i] = s[i - len];

	*str[i] = 0;

	return *str;
}

static inline char *appendToString(char **str, char c) {
	if (!*str) {
		 *str = malloc(1);
		**str = 0;
	}

	unsigned len = strlen(*str);
	
	*str = realloc(*str, len + 2);
	(*str)[len] = c;
	(*str)[len + 1] = 0;
	
	return *str;
}

static inline void freeArray(Array *arr) {
	for (unsigned int i = 0; i < arr->length; i++)
		switch (arr->contents[i].type) {
			case STRING:
				free(arr->contents[i].str);
			break;
			case OBJECT:
				freeJSON(arr->contents[i].json);
			break;
			case ARRAY:
				freeArray(arr->contents[i].array);
			break;
		}

	free(arr);
}

static inline void freeJSON(JSON *json) {
	for (unsigned int i = 0; i < json->length; i++) {
		free(json->contents[i].name);
		switch (json->contents[i].type) {
			case STRING:
				free(json->contents[i].str);
			break;
			case OBJECT:
				freeJSON(json->contents[i].json);
			break;
			case ARRAY:
				freeArray(json->contents[i].array);
			break;
		}
	}

	free(json);
}