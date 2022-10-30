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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
// #define NDEBUG
// #include <assert.h>
enum {
  TK_NOTYPE = 256, TK_EQ,
  /* TODO: Add more token types */
  TK_NUM,TK_ADD,TK_SUB,TK_MUL,TK_DIV,
  TK_BRA_L,TK_BRA_R,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"\\+{1}", TK_ADD},      // plus
  {"\\-{1}", TK_SUB},      // plus
  {"\\*{1}", TK_MUL},      // plus
  {"\\/{1}", TK_DIV},      // plus
  {"\\({1}", TK_BRA_L}, // bracket
  {"\\){1}", TK_BRA_R}, // bracket
  {"^[1-9][0-9]*", TK_NUM},// Num
  {"==", TK_EQ},        // equal
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
static bool check_parentheses(int start_index, int end_index);
static word_t get_main_cal(int p,int q);
static word_t eval(int p,int q);

static void display_token(){
  int i;
  printf("-----------------------------------------Tokens table-----------------------------------------\n");
  for(i=0;i<32;i=i+8){
    printf("%4d/%4s | %4d/%4s | %4d/%4s | %4d/%4s | %4d/%4s | %4d/%4s | %4d/%4s | %4d/%4s | \n",\
    tokens[i].type  ,tokens[i].str,   tokens[i+1].type,tokens[i+1].str, tokens[i+2].type,tokens[i+2].str, tokens[i+3].type,tokens[i+3].str, \
    tokens[i+4].type,tokens[i+4].str, tokens[i+5].type,tokens[i+5].str, tokens[i+6].type,tokens[i+6].str, tokens[i+7].type,tokens[i+7].str );
  }
  printf("----------------------------------------------------------------------------------------------\n");
  printf("nr_token:%d \n",nr_token);
  printf("----------------------------------------------------------------------------------------------\n");
  
}
static void clear_token(){
  int i;
  // printf("Clear token .\n");
  for(i=0;i<32;i=i+1){
    tokens[i].type=0;
    strcpy(tokens[i].str,"");
  }
}
static bool make_token(char *e) {
  int position = 0;
  int i,j,k;
  j=0;
  regmatch_t pmatch;

  nr_token = 0;
  clear_token();

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NUM:
            tokens[j].type = TK_NUM;
            for(k=0;k<substr_len;k++){
                tokens[j].str[k] =e[position-substr_len+k];
            }
            j=j+1;nr_token=nr_token+1;
            break;
          case TK_ADD:
          case TK_SUB:
          case TK_MUL:
          case TK_DIV:
          case TK_BRA_L:
          case TK_BRA_R:
            tokens[j].type = rules[i].token_type;
            tokens[j].str[0] =e[position-substr_len];
            j=j+1;nr_token=nr_token+1;
            break;
          default:
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("i=%d \n",i);
      printf("NR_REGEX=%d \n",NR_REGEX);
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  // display_token();
  
  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  // printf("Express Vale:%d \n",eval(0,nr_token-1));
  return eval(0,nr_token-1);

}
static bool check_parentheses(int start_index, int end_index){
  int i;
  int j=0;
  bool first_parenth_flag = false;
  // printf("start_index=%d,end_index=%d \n",start_index,end_index);
  for(i=start_index;i<=end_index && tokens[i].type>0;i++){
    if(tokens[i].type==TK_BRA_L){
      j++;
      first_parenth_flag = (i==start_index)? true:first_parenth_flag;
      // printf("i=%d,j=%d,parenth_flag=%d,Get a left parenthese \n",i,j,first_parenth_flag);
    }
    else if(tokens[i].type==TK_BRA_R){
      j--;
      //assert(j>=0); //右括号缺少匹配的左括号报错
      // printf("i=%d,j=%d,parenth_flag=%d,Get a right parenthese \n",i,j,first_parenth_flag);
      if(i==end_index && j==0 && first_parenth_flag==true){
        // printf("Return parenthese true. \n");
        return true;
      }
      first_parenth_flag = (j==0)? false:first_parenth_flag;
    }
  }
  if(j != 0){
    printf("Bad expression!!! \n");
    assert(0);
  }
  return false;
}

static word_t get_main_cal(int p,int q){
  int i;
  bool parenthese_flag=false;
  word_t position = 0;
  for(i=p;i<=q;i++){
    if(tokens[i].type == TK_BRA_L){ parenthese_flag = true;}
    else if(tokens[i].type == TK_BRA_R){parenthese_flag = false;}
    if((tokens[i].type==TK_ADD || tokens[i].type==TK_SUB) && parenthese_flag==false){  //当运算符为"+"或"-”,且不在括号中
      position = i;
    }else if((tokens[i].type==TK_MUL || tokens[i].type==TK_DIV) && parenthese_flag==false && position==0){//当运算符为"*"或"/”,不在括号中，且没有加减符号出现过
      position = i;
    }
  }
  return position;
}
static word_t eval(int p,int q){
  if (p > q) {
    /* Bad expression */
    assert(0);
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    return (word_t)atoi(tokens[p].str);
  }
  else if(check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    // printf("Check parentheses,remove parenthese\n");
    return eval(p + 1, q - 1);
  }
  else {
    word_t op,val1,val2;
    //op = the position of 主运算符 in the token expression;
    op = get_main_cal(p,q);
    // printf("p=%d,q=%d \n",p,q);
    // printf("op=%lu,Main cal:%s \n",op,tokens[op].str);
    // printf("Sub express1:%s..%s \n",tokens[p].str,tokens[op-1].str);
    // printf("Sub express2:%s..%s \n",tokens[op+1].str,tokens[q].str);
    val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);
   
    switch (tokens[op].str[0]) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
  return 0;
}
