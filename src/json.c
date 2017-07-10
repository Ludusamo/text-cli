#include "json.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void throw_error(const char *msg, int lineno) {
	fprintf(stderr, "[Line %d]: Message: %s\n", lineno, msg);
}

#define DEFINE_TYPE_STRING(type) case k##type: return #type;
const char *type_str(Token_Type t) {
	switch (t) {
		TOKEN_LIST(DEFINE_TYPE_STRING)
	}
}
#undef DEFINE_TYPE_STRING

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
		case '{':
			append_list(list, from_ptr(_create_token(kLCURLY,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case '}':
			append_list(list, from_ptr(_create_token(kRCURLY,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case '[':
			append_list(list, from_ptr(_create_token(kLBRACK,
				_substring(json_str, start, current),
				nil_val,
				line)));
			break;
		case ']':
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
				Value val;
				if (strcmp(sub, "true") == 0) {
					type = kTRUE;
					val = true_val;
				} else if (strcmp(sub, "false") == 0) {
					type = kFALSE;
					val = false_val;
				} else if (strcmp(sub, "null") == 0) {
					type = kNULL;
					val = nil_val;
				}
				if (type == 0) {
					throw_error("Invalid identifier", line);
				} else {
					append_list(list, from_ptr(
						_create_token(type, sub, val, line)));
				}
			} else {
				throw_error("Unidentified Token", line);
			}
		}
		start = current;
	}
	append_list(list, from_ptr(_create_token(kEOF, 0, nil_val, line)));
}

static int _peek(List *tokens, int i, Token_Type type) {
	Token *tok = get_ptr(access_list(tokens, i));
	if (tok->type == type) {
		return 1;
	}
	return 0;
}

static int _expect(List *tokens, int i, Token_Type type) {
	Token *tok = get_ptr(access_list(tokens, i));
	if (_peek(tokens, i, type)) {
		return 1;
	}
	char err_msg[255];
	sprintf(err_msg, "Parse Error: Expected %s, Received %s",
	        type_str(type), type_str(tok->type));
	throw_error(err_msg, tok->line);
	return 0;
}

static Value _parse_val(List *tokens, int *cur) {
	Token *tok = get_ptr(access_list(tokens, (*cur)));
	switch(tok->type) {
	case kLCURLY:
		return from_ptr(_parse_obj(tokens, cur));
	case kLBRACK:
		return from_ptr(_parse_list(tokens, cur));
		break;
	case kSTRING:
	case kNUMBER:
	case kNULL:
	case kTRUE:
	case kFALSE:
		(*cur)++;
		return ((Token *) get_ptr(access_list(tokens, *cur)))->val;
	default: {
		Token *t = ((Token*) get_ptr(access_list(tokens, *cur)));
		throw_error("Not a valid value.", t->line);
	}
	}
	return nil_val;
}

static Keyval *_parse_pair(List *tokens, int *cur) {
	Keyval *pair = malloc(sizeof(Keyval));
	if (_expect(tokens, (*cur)++, kSTRING)) {
		pair->key = get_ptr(
			((Token *) get_ptr(access_list(tokens, *cur - 1)))->val);
		if (_expect(tokens, (*cur)++, kCOLON)) {
			pair->val = _parse_val(tokens, cur);
		}
	}
	return pair;
}

static List *_parse_list(List *tokens, int *cur) {
	List *l = malloc(sizeof(List));
	ctor_list(l);
	if (_expect(tokens, (*cur)++, kLBRACK)) {
		if (!_peek(tokens, (*cur), kRBRACK)) {
			do {
				append_list(l, _parse_val(tokens, cur));
			} while (_peek(tokens, (*cur)++, kCOMMA));
			(*cur)--;
		}
		_expect(tokens, (*cur)++, kRBRACK);
	}
	return l;
}

static Json *_parse_obj(List *tokens, int *cur) {
	Json *obj = malloc(sizeof(Json));
	ctor_hashtable(&obj->h);
	if (_expect(tokens, (*cur)++, kLCURLY)) {
		if (_peek(tokens, (*cur), kSTRING)) {
			do {
				Keyval *pair = _parse_pair(tokens, cur);
				set_hashtable(&obj->h, pair->key, pair->val);
				free(pair);
			} while (_peek(tokens, (*cur)++, kCOMMA));
			(*cur)--;
		}
		_expect(tokens, (*cur)++, kRCURLY);
	}
	return obj;
}

Json *parse_json(const char *json_str) {
	List tokens;
	ctor_list(&tokens);
	_tokenize_json(&tokens, json_str);
	int cur = 0;
	return _parse_obj(&tokens, &cur);
}

}
