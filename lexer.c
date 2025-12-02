#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_INTEGER,
  TOKEN_FLOAT,
  TOKEN_SPACE,
  TOKEN_STRING, 
  TOKEN_IDENTIFIER,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_UNKNOWN, 
  TOKEN_EOF,
  TOKEN_NEWLINE,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_COMMA,
  TOKEN_LCURLY,
  TOKEN_RCURLY,
  TOKEN_COLON,
  TOKEN_SEMI,
  TOKEN_FN,
  TOKEN_LET,
  TOKEN_IDENT_INT, 
  TOKEN_EQU,
  TOKEN_RETURN
} symbols;

typedef enum {
  BHV_STACK,
  BHV_UNDEFINED,
  BHV_NUMBER,
  BHV_STRING,
  BHV_FLOAT,
  BHV_IDENT,
} symbol_bhv;

char *token_type_to_string(symbols type) {
  switch (type) {
    case TOKEN_PLUS: return "TOKEN_PLUS";
    case TOKEN_MINUS: return "TOKEN_MINUS";
    case TOKEN_INTEGER: return "TOKEN_INTEGER";
    case TOKEN_FLOAT: return "TOKEN_FLOAT";
    case TOKEN_SPACE: return "TOKEN_SPACE";
    case TOKEN_STRING: return "TOKEN_STRING";
    case TOKEN_MUL: return "TOKEN_MUL";
    case TOKEN_DIV: return "TOKEN_DIV";
    case TOKEN_LPAREN: return "TOKEN_LPAREN";
    case TOKEN_RPAREN: return "TOKEN_RPAREN";
    case TOKEN_COMMA: return "TOKEN_COMMA";
    case TOKEN_EOF: return "TOKEN_EOF";
    case TOKEN_NEWLINE: return "TOKEN_NEWLINE";
    case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
    case TOKEN_LCURLY: return "TOKEN_LCURLY";
    case TOKEN_RCURLY: return "TOKEN_RCURLY";
    case TOKEN_SEMI: return "TOKEN_SEMI";
    case TOKEN_COLON: return "TOKEN_COLON";
    case TOKEN_UNKNOWN: return "TOKEN_UNKNOWN";
    case TOKEN_FN: return "TOKEN_FN";
    case TOKEN_LET: return "TOKEN_LET";
    case TOKEN_IDENT_INT: return "TOKEN_IDENT_INT";
    case TOKEN_EQU: return "TOKEN_EQU";
    case TOKEN_RETURN: return "TOKEN_RETURN";

                    // default: return "UNKNOWN_SYMBOL";
  }
}

typedef struct {
  symbols *type;
  char **text;
  char **tktype;
  size_t *text_len;
  symbol_bhv *behaviour;
  unsigned int *cursor_skip;
  symbols *previous_token;
  size_t capacity;
  size_t size;
} Token;

void token_init(Token *tok, size_t capacity) {
  tok->capacity = capacity;
  tok->size = 0;

  tok->type = malloc(sizeof(symbols) * capacity);
  tok->text = malloc(sizeof(char *) * capacity);
  tok->text_len = malloc(sizeof(size_t) * capacity);
  tok->behaviour = malloc(sizeof(symbol_bhv) * capacity);
  tok->cursor_skip = malloc(sizeof(unsigned int) * capacity);
  tok->previous_token = malloc(sizeof(symbols) * capacity);
  tok->tktype = malloc(sizeof(char*) * capacity);
  assert(tok->type && tok->text && tok->text_len &&
         tok->behaviour && tok->cursor_skip && tok->previous_token);
}

void token_grow(Token *tok) {
  size_t new_capacity = (tok->capacity == 0 ? 8 : tok->capacity * 2);

  tok->type = realloc(tok->type, new_capacity * sizeof(symbols));
  tok->text = realloc(tok->text, new_capacity * sizeof(char *));
  tok->text_len = realloc(tok->text_len, new_capacity * sizeof(size_t));
  tok->behaviour = realloc(tok->behaviour, new_capacity * sizeof(symbol_bhv));
  tok->cursor_skip = realloc(tok->cursor_skip, new_capacity * sizeof(unsigned int));
  tok->previous_token = realloc(tok->previous_token, new_capacity * sizeof(symbols));
  tok->tktype = realloc(tok->tktype, new_capacity*sizeof(char*));
  assert(tok->type && tok->text && tok->text_len &&
         tok->behaviour && tok->cursor_skip && tok->previous_token);

  tok->capacity = new_capacity;
}

void token_push(Token *tok, symbols type, const char *text,
                symbol_bhv behaviour, size_t cursor_skip) {
  if (tok->size >= tok->capacity) {
    token_grow(tok);
  }

  size_t i = tok->size;

  tok->type[i] = type;
  tok->text[i] = strdup(text);
  tok->text_len[i] = strlen(text);
  tok->behaviour[i] = behaviour;
  tok->cursor_skip[i] = cursor_skip;
  tok->tktype[i] = token_type_to_string(tok->type[i]);
  
  if (i > 0)
    tok->previous_token[i] = tok->type[i - 1];
  else
    tok->previous_token[i] = TOKEN_UNKNOWN;

  tok->size++;
}

void token_free(Token *tok) {
  for (size_t i = 0; i < tok->size; i++) {
    free(tok->text[i]);
  }
  free(tok->type);
  free(tok->text);
  free(tok->text_len);
  free(tok->behaviour);
  free(tok->cursor_skip);
  free(tok->previous_token);
}


int str_to_int(char *strint) { return atoi(strint); }
float str_to_float(char *strif) { return strtof(strif, NULL); }

size_t read_from_tok(Token *tok, const char *input, size_t cursor) {
  char buf[64];
  size_t start = cursor;
  size_t i = 0;

  if (isdigit((unsigned char)input[cursor])) {
    int dots_seen = 0;
    while (isdigit((unsigned char)input[cursor]) || input[cursor] == '.') {
      if (input[cursor] == '.') dots_seen++;
      buf[i++] = input[cursor++];
      if (i >= sizeof(buf) - 1) break;
    }
    buf[i] = '\0';
    token_push(tok, dots_seen == 0 ? TOKEN_INTEGER : TOKEN_FLOAT,
               buf, dots_seen == 0 ? BHV_NUMBER : BHV_FLOAT,
               cursor - start);
    return cursor - start; // all digits handled
  }

  else if (input[cursor] == '"') {
    cursor++; // skip opening quote
    while (input[cursor] != '"' && input[cursor] != '\0') {
      buf[i++] = input[cursor++];
      if (i >= sizeof(buf) - 1) break;
    }
    buf[i] = '\0';
    if (input[cursor] == '"') cursor++; // skip closing quote
    token_push(tok, TOKEN_STRING, buf, BHV_STRING, cursor - start);
    return cursor - start;
  }

  else if (isalpha((unsigned char)input[cursor])) {
    while (isalpha((unsigned char)input[cursor])) {
      buf[i++] = input[cursor++];
      if (i >= sizeof(buf) - 1) break;
    }
    buf[i] = '\0';
    
    if (strcmp(buf, "let") == 0) token_push(tok, TOKEN_LET, buf, BHV_UNDEFINED, cursor - start);
    else if (strcmp(buf, "fn") == 0) token_push(tok, TOKEN_FN, buf, BHV_UNDEFINED, cursor - start);
    else if (strcmp(buf, "return") == 0) token_push(tok, TOKEN_RETURN, buf, BHV_UNDEFINED, cursor - start);
    else if (strcmp(buf, "int") == 0) token_push(tok, TOKEN_IDENT_INT, buf, BHV_UNDEFINED, cursor - start); // TODO: unhardcode
    else token_push(tok, TOKEN_IDENTIFIER, buf, BHV_IDENT, cursor - start);

    return cursor - start;
  }

  // Single-character tokens and symbols
  switch (input[cursor]) {
    case '+': token_push(tok, TOKEN_PLUS, "+", BHV_STACK, 1); break;
    case '-': token_push(tok, TOKEN_MINUS, "-", BHV_STACK, 1); break;
    case '*': token_push(tok, TOKEN_MUL, "*", BHV_STACK, 1); break;
    case '/': token_push(tok, TOKEN_DIV, "/", BHV_STACK, 1); break;
    case '{': token_push(tok, TOKEN_LCURLY, "{", BHV_STACK, 1); break;
    case '}': token_push(tok, TOKEN_RCURLY, "}", BHV_STACK, 1); break;
    case ';': token_push(tok, TOKEN_SEMI, ";", BHV_STACK, 1); break;
    case ':': token_push(tok, TOKEN_COLON, ":", BHV_STACK, 1); break;
    case '=': token_push(tok, TOKEN_EQU, "=", BHV_STACK, 1); break;

    case '(':
      token_push(tok, TOKEN_LPAREN, "(", BHV_STACK, 1);
      break;
    case ')':
      token_push(tok, TOKEN_RPAREN, ")", BHV_STACK, 1);
      break;
    case ',':
      token_push(tok, TOKEN_COMMA, ",", BHV_STACK, 1);
      break;
    case ' ':
      // you can skip space tokens if you don't need them
      // token_push(tok, TOKEN_SPACE, " ", BHV_UNDEFINED, 1);
      break;
    case '\n':
      token_push(tok, TOKEN_NEWLINE, "\\n", BHV_UNDEFINED, 1);
      break;
    case '\0':
      return 0; // end of input
    default: {
      buf[0] = input[cursor];
      buf[1] = '\0';
      token_push(tok, TOKEN_UNKNOWN, buf, BHV_UNDEFINED, 1);
      break;
    }
  }

  cursor++; // move forward exactly one char for symbol cases
  return cursor - start;
}


Token tokenize_all(const char *input) {
  Token tok;
  token_init(&tok, 8);

  size_t i = 0;
  size_t length = strlen(input);

  while (i < length) {
    i += read_from_tok(&tok, input, i);
  }

  token_push(&tok, TOKEN_EOF, "EOF", BHV_UNDEFINED, 0);
  return tok;
}
