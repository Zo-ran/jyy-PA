/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <stdlib.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_INT,
  TK_LP,
  TK_RP
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},    
  {"\\+", '+'},         
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"==", TK_EQ},        
  {"[1-9][0-9]*|0", TK_INT},
  {"\\(", TK_LP},
  {"\\)", TK_RP}
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        if (rules[i].token_type != TK_NOTYPE) {
          assert(nr_token < 32);
          tokens[nr_token].type = rules[i].token_type;
          switch (rules[i].token_type) {
            case TK_INT:
              assert(substr_len < 32);
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              break;
          }
          nr_token += 1;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}	

bool check_parentheses(int p, int q, bool *success) {
  int prefix = 0;
  bool ret = true;
  for (int i = p; i <= q; ++i) {
    switch (tokens[i].type) {
      case TK_LP:
        prefix += 1;
        break;
      case TK_RP:
        prefix -= 1;
        break;
    }
    if (prefix == 0 && i != q) {
      ret = false;
    } else if (prefix < 0) {
      return *success = false;
    }
  }
  return (prefix != 0) ? (*success = false) : ret;
}

int get_major_op(char *e, int p, int q) {
  int prefix = 0;
  for (int i = p; i <= q; ++i) {  
    switch (tokens[i].type) {
      
    }
  }
}

word_t eval(char *e, int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  } else if (p == q) {
    return (tokens[p].type != TK_INT) ? (*success = false) : atoi(tokens[p].str);
  } else {
    bool parentheses = check_parentheses(p, q, success);
    if (!success)
      return 0;
    if (parentheses) {
      return eval(e, p + 1, q - 1, success);
    } else {
      int op = get_major_op(e, p, q);
      word_t val1 = eval(e, p, op - 1, success), val2 = eval(e, op + 1, q, success);
      if (!success)
        return 0;
      switch (tokens[op].type) {

      }
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(e, 0, nr_token - 1, success);
}
