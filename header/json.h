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
	code(COLON) \
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

const char *type_str(Token_Type t);
Token *_create_token(Token_Type type, const char *lexeme, Value val, int line);
char *_substring(const char *src, int begin, int end);
void print_token(Token *t);
void _tokenize_json(List *list, const char *json_str);
static Value _parse_val(List *tokens, int *cur);
static Keyval *_parse_pair(List *tokens, int *cur);
static List *_parse_list(List *tokens, int *cur);
static Json *_parse_obj(List *tokens, int *cur);
Json *parse_json(const char *json_str);

char *_stringify_val(const Value v);
char *_stringify_pair(const Keyval *pair);
char *_stringify_list(const List *l);
char *_stringify_obj(const Json *json);
char *json_stringify(const Json *json);

#endif // JSON_H
