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

#include <common.h>
#include "/home/yyx/riscv/ysyx-workbench/nemu/src/monitor/sdb/sdb.h"

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
static void expr_test();
static int get_line(char **str,int *buff_size,FILE *fp);
static void get_line_free(char **str);

static int get_line(char **str,int *buff_size,FILE *fp){
    if(NULL == *str){
        //默认大小120
        *buff_size = 120;
        *str = malloc(120);
    }else{
        //清空字符串
        memset(*str, 0, strlen(*str));
    }
    //记录每行的字符数
    int line_ch_size = 0; 
    //每次读取的字符
    int ch;
    while (1)
    {
        if((ch = fgetc(fp)) != EOF){ 
            if(ch == '\n'){
                //读取一行完毕
                break;
            }else{
                if(line_ch_size == (*buff_size) - 1){
                    //需要扩容，一次加120
                    *buff_size = (*buff_size) + 120; 
                    *str = realloc(*str,*buff_size);
                }
                sprintf(*str,"%s%c",*str,ch);
                line_ch_size++;
            }
        }else{
            break;
        }
    }
    return line_ch_size;
}

static void get_line_free(char **str){
    if(NULL != *str){
        free(*str);
    }
}
static void expr_test(){
  FILE* fp = fopen("tools/gen-expr/input","r");
  //用于存储每行字符串
  char *str_line = NULL;
  //存储字符串的内存大小
  int buff_size;
  //每行的字符数
  int line_ch_size;

  word_t expr_value = NULL;
  word_t cal_value = NULL;
  char *cal_ptr = NULL;
  bool* success;
  char* expr_ptr;
  word_t cnt = 0;

  while((line_ch_size = get_line(&str_line,&buff_size,fp)) != 0){
      // printf("缓冲区大小：%d\n",buff_size);
      // printf("行字符数：%d\n",line_ch_size);
      // printf("行字符串：%s\n",str_line);
      printf("Start Check %ld Line  \n",++cnt);
      char* expr_ptr = strtok(str_line," ");
      expr_value = (word_t)atoi(expr_ptr);
      cal_ptr = expr_ptr + strlen(expr_ptr) + 1;
      cal_value = expr(cal_ptr,&success);
      
  }
  get_line_free(&str_line);
}

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  expr_test();
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
