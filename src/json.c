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

void _destroy_token(Token *token) {
	free((void*) token->lexeme);
	free(token);
}

char *_substring(const char *src, int begin, int end) {
	char *sub = malloc(sizeof(char) * end - begin);
	strncpy(sub, src + begin, end - begin);
	return sub;
}

void print_token(Token *t) {
	printf("Token_Type: %s, Lexeme: %s, Line: %d, Value: ",
	       type_str(t->type),
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

Value _parse_val(List *tokens, int *cur) {
	Token *tok = get_ptr(access_list(tokens, (*cur)));
	switch(tok->type) {
	case kLCURLY: {
		Obj *obj = malloc(sizeof(Obj));
		*obj = (Obj) { OBJ, _parse_obj(tokens, cur) };
		return from_obj(obj);
	}
	case kLBRACK: {
		Obj *obj = malloc(sizeof(Obj));
		*obj = (Obj) { LIST, _parse_list(tokens, cur) };
		return from_obj(obj);
	}
	case kSTRING:
	case kNUMBER:
	case kNULL:
	case kTRUE:
	case kFALSE:
		(*cur)++;
		return tok->val;
	default: {
		Token *t = ((Token*) get_ptr(access_list(tokens, *cur)));
		throw_error("Not a valid value.", t->line);
	}
	}
	return nil_val;
}

Keyval *_parse_pair(List *tokens, int *cur) {
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

List *_parse_list(List *tokens, int *cur) {
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

Json *_parse_obj(List *tokens, int *cur) {
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
	Json *ret = _parse_obj(&tokens, &cur);
	Iter i;
	iter_list(&i, &tokens);
	foreach (i) {
		_destroy_token(get_ptr(i.val(&i)));
	}
	destroy_iter_list(&i);
	dtor_list(&tokens);
	return ret;
}

void _destroy_val(Value val) {
	if (is_obj(val)) {
		Obj *o = get_obj(val);
		if (o->type == LIST) _destroy_list(o->ptr);
		else if (o->type == OBJ) _destroy_obj(o->ptr);
		free(o);
	} else if (is_ptr(val)) {
		free(get_ptr(val));
	}
}

void _destroy_pair(Keyval *pair) {
	free((void*)pair->key);
	_destroy_val(pair->val);
	free(pair);
}

void _destroy_list(List *list) {
	Iter i;
	iter_list(&i, list);
	foreach (i) {
		_destroy_val(i.val(&i));
	}
	destroy_iter_list(&i);
	dtor_list(list);
	free(list);
}

void _destroy_obj(Json *json) {
	Iter i;
	iter_hashtable(&i, &json->h);
	foreach (i) {
		_destroy_pair(get_ptr(i.val(&i)));
	}
	destroy_iter_hashtable(&i);
	dtor_hashtable(&json->h);
	free(json);
}

void destroy_json(Json *json) {
	_destroy_obj(json);
}

static char *str_concat(char *dest, const char *src) {
	char *new = malloc(strlen(dest) + strlen(src) + 1);
	new[0] = '\0';
	strcpy(new, dest);
	strcat(new, src);
	free(dest);
	return new;
}

char *_stringify_val(const Value v) {
	char *str = malloc(1);
	str[0] = '\0';
	if (is_obj(v)) {
		Obj *o = get_obj(v);
		free(str);
		if (o->type == LIST) return _stringify_list(o->ptr);
		if (o->type == OBJ) return _stringify_obj(o->ptr);
	} else if (is_int32(v)) {
		char tmp[2000];
		sprintf(tmp, "%d", v.as_int32);
		return str_concat(str, tmp);
	} else if (nil_val.bits == v.bits) {
		return str_concat(str, "null");
	} else if (true_val.bits == v.bits) {
		return str_concat(str, "true");
	} else if (false_val.bits == v.bits) {
		return str_concat(str, "false");
	} else if (is_ptr(v)) {
		char tmp[strlen(get_ptr(v)) + 3];
		sprintf(tmp, "\"%s\"", get_ptr(v));
		return str_concat(str, tmp);
	}
	char tmp[2000];
	sprintf(tmp, "%f", v.as_double);
	return str_concat(str, tmp);
}

char *_stringify_pair(const Keyval *pair) {
	char *str = malloc(sizeof(1));
	str[0] = '\0';
	str = str_concat(str, pair->key);
	str = str_concat(str, ":");
	char *val = _stringify_val(pair->val);
	str = str_concat(str, val);
	free(val);
	return str;
}

char *_stringify_list(const List *l) {
	char *jstr = malloc(sizeof(2));
	strcpy(jstr, "[");
	Iter i;
	iter_list(&i, l);
	while (i.done(&i)) {
		char *val =_stringify_val(i.val(&i));
		jstr = str_concat(jstr, val);
		free(val);
		i.next(&i);
		if (i.done(&i)) strcat(jstr, ",");
	}
	destroy_iter_list(&i);
	jstr = str_concat(jstr, "]");
	return jstr;
}

char *_stringify_obj(const Json *json) {
	char *jstr = malloc(sizeof(2));
	strcpy(jstr, "{");
	Iter i;
	iter_hashtable(&i, &json->h);
	while (i.done(&i)) {
		char *pair = _stringify_pair(get_ptr(i.val(&i)));
		jstr = str_concat(jstr, pair);
		free(pair);
		i.next(&i);
		if (i.done(&i)) strcat(jstr, ",");
	}
	jstr = str_concat(jstr, "}");
	destroy_iter_hashtable(&i);
	return jstr;
}

char *json_stringify(const Json *json) {
	return _stringify_obj(json);
}
