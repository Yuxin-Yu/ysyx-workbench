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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "memory/paddr.h"



static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("$(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  cpu_exec(-1);
  return -1;
}
static int cmd_si(char *args) {
  /* extract the first token as the parameter */
  char *param = strtok(args, " ");
  int step_v;

  if(param==NULL){
    // printf("param is NULL \n");
    step_v = 1;
  }else{
    step_v = atoi(param);
  }
  cpu_exec(step_v);
  return 0;
}
static int cmd_info(char *args) {
  /* extract the first token as the parameter */
  char *param = strtok(args, " ");
  if(strcmp(param,"r") == 0){
    isa_reg_display();
  }else if (strcmp(param,"w") == 0)
  {
    printf("get w\n");
  }else{
    printf("error SUBCMD\n");
  }
  
  return 0;
}
static int cmd_x(char *args) {
  /* extract the first token as the parameter */
  char *param = strtok(args, " ");
  int num = atoi(param);

  long unsigned int addr;
  char *param2 = strtok(param + strlen(param) + 1," ");
  char *str;
  addr = strtol(param2,&str,16);

  word_t value = paddr_read((uint64_t)addr,num);
  int i;
  for(i=0;i<num;i++){
    printf("0x%lx=0x%lx | ",addr,value&0xff);
    value=value>>8;
  }
  printf("\n");
  return 0;
}
static int cmd_p(char *args) {
  /* extract the first token as the parameter */
  char *expr_v = strtok(args, " ");
  bool success;

  word_t result = expr(expr_v,&success);
  printf("Result = %ld \n",result);
  return 0;
}
static int cmd_w(char *args) {
  
  return 0;
}
static int cmd_d(char *args) {
  
  return 0;
}
static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  { "si", "'si [N]'.Execute n step", cmd_si },
  { "info", "'info SUBCMD'.SUBCMD=r/w,Print some information of regs or watchpoint", cmd_info },
  { "x", "'x N EXPR'(N=1,2,4,8).Evaluate the expression EXPR, using the result as the starting memory address,\
  output N consecutive 4-bytes in hexadecimal", cmd_x },
  { "p", "'p EXPR'.Evaluate the expression EXPR", cmd_p },
  { "w", "'w EXPR'.Suspend program execution when the value of the expression EXPR changes", cmd_w },
  { "d", "'d [N]'.Delete the watchpoint with sequence number N", cmd_d },
  
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

// #define DEBUG 1
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

#ifndef DEBUG
  for (char *str; (str = rl_gets()) != NULL; ) {
#else
    // char *str = "p (3-1)*3";
    char *str = "info r";
#endif
    // printf("str cmd:%s \n",str);
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
  #ifndef DEBUG
    if (cmd == NULL) { continue; }
  #endif

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    // printf("cmd:%s\n",cmd);
    // printf("str:%s\n",str);
    // printf("*str:%d\n",*str);
    char *args = cmd + strlen(cmd) + 1;
    // printf("args:%s\n",args);
    // printf("*args:0x%x\n",*args);
    // printf("args len:%d\n",(int)strlen(strtok(args," ")));
    if (args >= str_end) {
      args = NULL;
    }
#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
#ifndef DEBUG
  }
#endif
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
