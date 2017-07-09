#ifndef JSON_H
#define JSON_H

#include "hashtable.h"
#include "list.h"
typedef struct {
	Hashtable h;
} Json;

#define TOKEN_LIST(code) \
	code(LBRACK) \
	code(RBRACK) \
	code(LCURLY) \
	code(RCURLY) \
	code(COMMA) \
	code(STRING) \
	code(NUMBER) \
	code(TRUE) \
	code(FALSE) \
	code(NULL) \
	code(EOF)

#define DEFINE_ENUM_TYPE(type) k##type,
typedef enum {
	TOKEN_LIST(DEFINE_ENUM_TYPE)
} Token_Type;
#undef DEFINE_ENUM_TYPE

typedef struct {
	Token_Type type;
	const char *lexeme;
	Value val;
	int line;
} Token;

Token *_create_token(Token_Type type, const char *lexeme, Value val, int line);
char *_substring(const char *src, int begin, int end);
void print_token(Token *t);
void _tokenize_json(List *list, const char *json_str);
void parse_json(Json *json, const char *json_str);

#endif // JSON_H
