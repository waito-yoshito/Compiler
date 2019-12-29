#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef enum{
  TK_RESERVED, //記号 0
  TK_NUM, //整数トークン 1
  TK_EOF, //入力の終わりを表すトークン 2
} TokenKind;

typedef struct Token Token;

// 構造体　トークンを連結リストで管理
struct Token{
  TokenKind kind; //トークンの型
  Token *next; //次の入力トークン
  int val; //kindがTK_NUMの場合、その数値
  char *str; //トークン文字列
};

//現在着目しているトークン
Token *token;

//入力プログラム
char *user_input;

void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char op){
  if(token -> kind != TK_RESERVED || token -> str[0] != op){
    return false;
  }
  token = token -> next;
  return true;
}

void expect(char op){
  if(token -> kind != TK_RESERVED || token -> str[0] != op){
    error_at(token->str, "'%c'ではありません", op);
  }
  token = token -> next;
}

int expect_number(){
  if(token -> kind != TK_NUM){
    error_at(token->str, "数ではありません");
  }
  int val = token -> val;
  token = token -> next;
  return val;
}

bool at_eof(){
  return token -> kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok -> kind = kind;
  tok -> str = str;
  cur -> next = tok;
  return tok;
}

Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if(isxdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur -> val = strtol(p, &p, 10);
      continue;
    }

    error_at(user_input,"トーカナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", expect_number());

  while(!at_eof()){
    if(consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}