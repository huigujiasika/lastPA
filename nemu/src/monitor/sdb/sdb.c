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

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;       //调用结束后保留其值

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);            //支持上下切换
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);

  return 0;
}


static int cmd_q(char *args) {
  set_quit();                //TODO:: 此处为了退出不报错加的，可能有问题

  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  int steps;

  char *args_steps = strtok(args, " ");
  if(args_steps==NULL)
    steps=1;
  else
    steps=atoi(args_steps);
  cpu_exec(steps);

  return 0;
}

static int cmd_info(char *args){
  char *subcmd=strtok(NULL," ");
  
  switch (subcmd[0])
  {
    case 'r':
      isa_reg_display();  break;
    case 'w':
      //watch_display();   break;
    default:
      printf("Unsupported subcommand: %s\n", subcmd);
  }

  return 0;
}



static int cmd_x(char *args){  //需要调整
  char* n =strtok(NULL," ");             
  char* exp=strtok(NULL," ");
  
  int num;
  uint32_t addr;   //无符号 16进制
  sscanf(n,"%d",&num);   //bool success=false;      //首先规定只能是16进制数
  sscanf(exp,"%x",&addr);   

  int i=0;
  #include <memory/vaddr.h>
  // for(int inst=0;inst<num;inst++){
  //   printf("0x%02x",vaddr_read( addr+(i++) ,4 )  );
  // }
  while(num--){
    printf("0x%08x\n",vaddr_read( addr+i ,4 )  );
    i+=4;
    //printf(" %02x",vaddr_read( addr+(i++) ,1 )  );
  }
  putchar('\n');
  
  return 0;
}




static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "choose steps to excute proogram", cmd_si },
  { "info", "print info about reg or watchpoint", cmd_info },
  { "x", "search memory", cmd_x },

  /* TODO: Add more commands */

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

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
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
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
