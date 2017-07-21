#ifndef JSON_H
#define JSON_H

#include "hashtable.h"
#include "list.h"

typedef enum {
	jLIST,
	jOBJ,
	jSTRING,
	jNUM,
	jBOOL,
	jNULL
} Json_Type;

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
void _destroy_token(Token *token);

char *_substring(const char *src, int begin, int end);
void print_token(Token *t);
void _tokenize_json(List *list, const char *json_str);
Value _parse_val(List *tokens, int *cur);
Keyval *_parse_pair(List *tokens, int *cur);
List *_parse_list(List *tokens, int *cur);
Json *_parse_obj(List *tokens, int *cur);
Json *parse_json(const char *json_str);

void _destroy_val(Value val);
void _destroy_pair(Keyval *pair);
void _destroy_list(List *list);
void _destroy_obj(Json *json);
void destroy_json(Json *json);

char *_stringify_val(const Value v);
char *_stringify_pair(const Keyval *pair);
char *_stringify_list(const List *l);
char *_stringify_obj(const Json *json);
char *json_stringify(const Json *json);

int is_type_json(const Json *json, const char *name, Json_Type type);
Json *retrieve_obj_json(const Json *json, const char *name);
List *retrieve_list_json(const Json *json, const char *name);
const char *retrieve_str_json(const Json *json, const char *name);
double retrieve_num_json(const Json *json, const char *name);
int retrieve_bool_json(const Json *json, const char *name);

#endif // JSON_H
