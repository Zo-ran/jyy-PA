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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough 
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static int depth = 0;
static int max_depth = 100;
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static inline uint32_t choose(uint32_t n) {
  return rand() % n;
}

static void append(const char *format, int data) {
  char tmp[65536 + 128] = "\0";
  snprintf(tmp, sizeof(tmp), format, buf, data);
  strncpy(buf, tmp, strlen(tmp) < 65536 ? strlen(tmp) : 65536);
}

static inline void gen_rand_num() {
  append("%s%d", rand());
}

static inline void gen(char c) {
  append("%s%c", c);
}

static inline void gen_rand_op() {
  switch (choose(4)) {
    case 0: gen('+'); break;
    case 1: gen('-'); break;
    case 2: gen('*'); break;
    case 3: gen('/'); break;
  }
}

static void gen_rand_expr() {
  int c = (depth >= max_depth) ? 0 : choose(3);
  switch (c) {
    case 0: gen_rand_num(); break;
    case 1: depth += 1; gen('('); gen_rand_expr(); gen(')'); break;
    case 2: depth += 1; gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    memset(buf, 0, sizeof(buf));
    depth = 0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
