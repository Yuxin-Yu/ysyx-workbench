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
#include <stdbool.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static uint32_t buf_index=0;

static void gen_num(){
  uint32_t val;
  val = rand()%100+1; //1-100
  if(val>99){
    buf[buf_index++] = (char)((val/100)+0x30);
  }
  else if(val >9 && val<=99){
    buf[buf_index++] = (char)((val/10)+0x30);
    buf[buf_index++] = (char)((val%10)+0x30);
  }
  else {
    buf[buf_index++] = (char)((val%10)+0x30);
  }
}
static void gen(char input){
  buf[buf_index++] = input;
}
static void gen_rand_op() {
  if(buf_index < 65536){
    switch (rand()%4) {
      case 0: buf[buf_index++]='+';break;
      case 1: buf[buf_index++]='-'; break;
      case 2: buf[buf_index++]='*'; break;
      case 3: buf[buf_index++]='/'; break;
      default: break;
    } 
  }
  
}
static void gen_rand_expr() {
  
  switch (rand()%3) {
    case 0: gen_num();break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i,j;
  for (i = 0; i < loop; i ++) {
    for(j=0;j<65536;j++){buf[j]=NULL;}

    buf[0] = '(';
    buf_index = 1;
    gen_rand_expr();
    buf[buf_index] = ')';

    if(buf_index > 31){continue;}

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    // int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    unsigned result;
    fscanf(fp, "%u", &result);
    pclose(fp);

    printf("%u %s \n", result, buf);
  }
  return 0;
}
