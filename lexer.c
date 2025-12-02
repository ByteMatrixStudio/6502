#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
  TOKEN_ADC, TOKEN_AND, TOKEN_ASL, TOKEN_BCC, TOKEN_BCS, TOKEN_BEQ, TOKEN_BIT,
  TOKEN_BMI, TOKEN_BNE, TOKEN_BPL, TOKEN_BRK, TOKEN_BVC, TOKEN_BVS, TOKEN_CLC,
  TOKEN_CLD, TOKEN_CLI, TOKEN_CLV, TOKEN_CMP, TOKEN_CPX, TOKEN_CPY, TOKEN_DEC,
  TOKEN_DEX, TOKEN_DEY, TOKEN_EOR, TOKEN_INC, TOKEN_INX, TOKEN_INY, TOKEN_JMP,
  TOKEN_JSR, TOKEN_LDA, TOKEN_LDX, TOKEN_LDY, TOKEN_LSR, TOKEN_NOP, TOKEN_ORA,
  TOKEN_PHA, TOKEN_PHP, TOKEN_PLA, TOKEN_PLP, TOKEN_ROL, TOKEN_ROR, TOKEN_RTI,
  TOKEN_RTS, TOKEN_SBC, TOKEN_SEC, TOKEN_SED, TOKEN_SEI, TOKEN_STA, TOKEN_STX,
  TOKEN_STY, TOKEN_TAX, TOKEN_TAY, TOKEN_TSX, TOKEN_TXA, TOKEN_TXS, TOKEN_TYA,
  TOKEN_LABEL, TOKEN_IMMEDIATE, TOKEN_NUMBER, TOKEN_IDENTIFIER, TOKEN_COMMA,
  TOKEN_COLON, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_NEWLINE, TOKEN_COMMENT,
  TOKEN_EOF, TOKEN_UNKNOWN
} symbols;

typedef enum {
  BHV_STACK, BHV_UNDEFINED, BHV_NUMBER, BHV_STRING, BHV_FLOAT, BHV_IDENT
} symbol_bhv;

char *token_type_to_string(symbols type) {
  switch (type) {
    case TOKEN_ADC: return "TOKEN_ADC"; case TOKEN_AND: return "TOKEN_AND";
    case TOKEN_ASL: return "TOKEN_ASL"; case TOKEN_BCC: return "TOKEN_BCC";
    case TOKEN_BCS: return "TOKEN_BCS"; case TOKEN_BEQ: return "TOKEN_BEQ";
    case TOKEN_BIT: return "TOKEN_BIT"; case TOKEN_BMI: return "TOKEN_BMI";
    case TOKEN_BNE: return "TOKEN_BNE"; case TOKEN_BPL: return "TOKEN_BPL";
    case TOKEN_BRK: return "TOKEN_BRK"; case TOKEN_BVC: return "TOKEN_BVC";
    case TOKEN_BVS: return "TOKEN_BVS"; case TOKEN_CLC: return "TOKEN_CLC";
    case TOKEN_CLD: return "TOKEN_CLD"; case TOKEN_CLI: return "TOKEN_CLI";
    case TOKEN_CLV: return "TOKEN_CLV"; case TOKEN_CMP: return "TOKEN_CMP";
    case TOKEN_CPX: return "TOKEN_CPX"; case TOKEN_CPY: return "TOKEN_CPY";
    case TOKEN_DEC: return "TOKEN_DEC"; case TOKEN_DEX: return "TOKEN_DEX";
    case TOKEN_DEY: return "TOKEN_DEY"; case TOKEN_EOR: return "TOKEN_EOR";
    case TOKEN_INC: return "TOKEN_INC"; case TOKEN_INX: return "TOKEN_INX";
    case TOKEN_INY: return "TOKEN_INY"; case TOKEN_JMP: return "TOKEN_JMP";
    case TOKEN_JSR: return "TOKEN_JSR"; case TOKEN_LDA: return "TOKEN_LDA";
    case TOKEN_LDX: return "TOKEN_LDX"; case TOKEN_LDY: return "TOKEN_LDY";
    case TOKEN_LSR: return "TOKEN_LSR"; case TOKEN_NOP: return "TOKEN_NOP";
    case TOKEN_ORA: return "TOKEN_ORA"; case TOKEN_PHA: return "TOKEN_PHA";
    case TOKEN_PHP: return "TOKEN_PHP"; case TOKEN_PLA: return "TOKEN_PLA";
    case TOKEN_PLP: return "TOKEN_PLP"; case TOKEN_ROL: return "TOKEN_ROL";
    case TOKEN_ROR: return "TOKEN_ROR"; case TOKEN_RTI: return "TOKEN_RTI";
    case TOKEN_RTS: return "TOKEN_RTS"; case TOKEN_SBC: return "TOKEN_SBC";
    case TOKEN_SEC: return "TOKEN_SEC"; case TOKEN_SED: return "TOKEN_SED";
    case TOKEN_SEI: return "TOKEN_SEI"; case TOKEN_STA: return "TOKEN_STA";
    case TOKEN_STX: return "TOKEN_STX"; case TOKEN_STY: return "TOKEN_STY";
    case TOKEN_TAX: return "TOKEN_TAX"; case TOKEN_TAY: return "TOKEN_TAY";
    case TOKEN_TSX: return "TOKEN_TSX"; case TOKEN_TXA: return "TOKEN_TXA";
    case TOKEN_TXS: return "TOKEN_TXS"; case TOKEN_TYA: return "TOKEN_TYA";
    case TOKEN_LABEL: return "TOKEN_LABEL";
    case TOKEN_IMMEDIATE: return "TOKEN_IMMEDIATE";
    case TOKEN_NUMBER: return "TOKEN_NUMBER";
    case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
    case TOKEN_COMMENT: return "TOKEN_COMMENT";
    case TOKEN_EOF: return "TOKEN_EOF";
    default: return "TOKEN_UNKNOWN";
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
  assert(tok->type && tok->text && tok->text_len && tok->behaviour &&
         tok->cursor_skip && tok->previous_token);
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
  assert(tok->type && tok->text && tok->text_len && tok->behaviour &&
         tok->cursor_skip && tok->previous_token);
  tok->capacity = new_capacity;
}

void token_push(Token *tok, symbols type, const char *text,
                symbol_bhv behaviour, size_t cursor_skip) {
  if (tok->size >= tok->capacity) token_grow(tok);
  size_t i = tok->size;
  tok->type[i] = type;
  tok->text[i] = strdup(text);
  tok->text_len[i] = strlen(text);
  tok->behaviour[i] = behaviour;
  tok->cursor_skip[i] = cursor_skip;
  tok->tktype[i] = token_type_to_string(tok->type[i]);
  tok->previous_token[i] = (i > 0) ? tok->type[i - 1] : TOKEN_UNKNOWN;
  tok->size++;
}

void token_free(Token *tok) {
  for (size_t i = 0; i < tok->size; i++) free(tok->text[i]);
  free(tok->type);
  free(tok->text);
  free(tok->text_len);
  free(tok->behaviour);
  free(tok->cursor_skip);
  free(tok->previous_token);
}

bool is_mnemonic(const char *word) {
  static const char *mnemonics[] = {
    "ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI","BNE","BPL","BRK","BVC",
    "BVS","CLC","CLD","CLI","CLV","CMP","CPX","CPY","DEC","DEX","DEY","EOR",
    "INC","INX","INY","JMP","JSR","LDA","LDX","LDY","LSR","NOP","ORA","PHA",
    "PHP","PLA","PLP","ROL","ROR","RTI","RTS","SBC","SEC","SED","SEI","STA",
    "STX","STY","TAX","TAY","TSX","TXA","TXS","TYA", NULL
  };
  for (int i = 0; mnemonics[i]; i++)
    if (strcasecmp(word, mnemonics[i]) == 0) return true;
  return false;
}

size_t read_from_tok(Token *tok, const char *input, size_t cursor) {
  char buf[64*1024];
  size_t start = cursor, i = 0;
  if (input[cursor] == ' ' || input[cursor] == '\t') return 1;
  if (input[cursor] == ';') {
    while (input[cursor] && input[cursor] != '\n')
      buf[i++] = input[cursor++];
    buf[i] = '\0';
    token_push(tok, TOKEN_COMMENT, buf, BHV_UNDEFINED, cursor - start);
    return cursor - start;
  }
  if (isalpha((unsigned char)input[cursor]) || input[cursor] == '_') {
    while (isalnum((unsigned char)input[cursor]) || input[cursor] == '_')
      buf[i++] = input[cursor++];
    buf[i] = '\0';
    if (input[cursor] == ':') {
      token_push(tok, TOKEN_LABEL, buf, BHV_IDENT, cursor - start + 1);
      return (cursor - start) + 1;
    } else if (is_mnemonic(buf)) {
      token_push(tok, TOKEN_IDENTIFIER, buf, BHV_IDENT, cursor - start);
    } else {
      token_push(tok, TOKEN_IDENTIFIER, buf, BHV_IDENT, cursor - start);
    }
    return cursor - start;
  }
  if (input[cursor] == '#') {
    buf[i++] = input[cursor++];
    while (isalnum((unsigned char)input[cursor]) || input[cursor] == '$' ||
           input[cursor] == '%')
      buf[i++] = input[cursor++];
    buf[i] = '\0';
    token_push(tok, TOKEN_IMMEDIATE, buf, BHV_NUMBER, cursor - start);
    return cursor - start;
  }
  if (isdigit((unsigned char)input[cursor]) || input[cursor] == '$' ||
      input[cursor] == '%') {
    while (isalnum((unsigned char)input[cursor]) || input[cursor] == '$' ||
           input[cursor] == '%')
      buf[i++] = input[cursor++];
    buf[i] = '\0';
    token_push(tok, TOKEN_NUMBER, buf, BHV_NUMBER, cursor - start);
    return cursor - start;
  }
  switch (input[cursor]) {
    case ',': token_push(tok, TOKEN_COMMA, ",", BHV_UNDEFINED, 1); break;
    case '(': token_push(tok, TOKEN_LPAREN, "(", BHV_UNDEFINED, 1); break;
    case ')': token_push(tok, TOKEN_RPAREN, ")", BHV_UNDEFINED, 1); break;
    case '\n': token_push(tok, TOKEN_NEWLINE, "\\n", BHV_UNDEFINED, 1); break;
    case '\0': return 0;
    default:
      buf[0] = input[cursor];
      buf[1] = '\0';
      token_push(tok, TOKEN_UNKNOWN, buf, BHV_UNDEFINED, 1);
      break;
  }
  return 1;
}

Token tokenize_all(const char *input) {
  Token tok;
  token_init(&tok, 8);
  size_t i = 0, length = strlen(input);
  while (i < length) i += read_from_tok(&tok, input, i);
  token_push(&tok, TOKEN_EOF, "EOF", BHV_UNDEFINED, 0);
  return tok;
}
