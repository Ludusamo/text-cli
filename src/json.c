#include "json.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void throw_error(const char *msg, int lineno) {
	fprintf(stderr, "[Line %d]: Message: %s\n", lineno, msg);
}

Token *_create_token(Token_Type type, const char *lexeme, Value val, int line) {
	Token *t = malloc(sizeof(Token));
	*t = (Token) { type, lexeme, val, line};
	return t;
}

char *_substring(const char *src, int begin, int end) {
	char *sub = malloc(sizeof(char) * end - begin);
	strncpy(sub, src + begin, end - begin);
	return sub;
}

void print_token(Token *t) {
	printf("Token_Type: %d, Lexeme: %s, Line: %d, Value: ",
	       t->type,
	       t->lexeme,
	       t->line);
	if (is_double(t->val)) {
		printf("%lf\n", t->val.as_double);
	} else if(is_int32(t->val)) {
		printf("%d\n", t->val.as_int32);
	} else if (is_ptr(t->val)) {
		printf("%p\n", get_ptr(t->val));
	}
}

void _tokenize_json(List *list, const char *json_str) {
	size_t in_len = strlen(json_str);
	int start = 0;
	int current = 0;
	int line = 1;
	while (current < in_len) {
		char c = json_str[current++];
		switch (c) {
		case '(':
			append_list(list, from_ptr(_create_token(kLCURLY,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case ')':
			append_list(list, from_ptr(_create_token(kRCURLY,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case '{':
			append_list(list, from_ptr(_create_token(kLBRACK,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case '}':
			append_list(list, from_ptr(_create_token(kRBRACK,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case ':':
			append_list(list, from_ptr(_create_token(kCOLON,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case ',':
			append_list(list, from_ptr(_create_token(kCOMMA,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case '\n':
			line++;
			break;
		case '"':
			while (json_str[current] != '"' && current < in_len) {
				if (json_str[current] == '\n') line++;
				current++;
			}
			current++;
			append_list(list, from_ptr(_create_token(kSTRING,
				_substring(json_str, start, current),
				from_ptr(_substring(json_str, start + 1, current - 1)),
				line)));
			break;
		case ' ':
		case '\r':
		case '\t':
			break;
		default:
			if (isdigit(c) || c == '-') {
				while (isdigit(json_str[current])) current++;
				if (json_str[current] == '.'
					&& isdigit(json_str[current + 1])) {
					current++;
					while (isdigit(json_str[current])) current++;
				}
				char *sub = _substring(json_str, start, current);
				append_list(list, from_ptr(_create_token(kNUMBER,
					sub,
					from_double(atof(sub)),
					line)));
			} else if (isalpha(c)) {
				while (isalnum(json_str[current])) current++;
				char *sub = _substring(json_str, start, current);
				Token_Type type = 0;
				if (strcmp(sub, "true") == 0) {
					type = kTRUE;
				} else if (strcmp(sub, "false") == 0) {
					type = kFALSE;
				} else if (strcmp(sub, "null") == 0) {
					type = kNULL;
				}
				if (type == 0) {
					throw_error("Invalid identifier", line);
				} else {
					append_list(list, from_ptr(
						_create_token(type, sub, nil_val, line)));
				}
			} else {
				throw_error("Unidentified Token", line);
			}
		}
		start = current;
	}
	append_list(list, from_ptr(_create_token(kEOF, 0, nil_val, line)));
}

void parse_json(Json *json, const char *json_str) {

}
