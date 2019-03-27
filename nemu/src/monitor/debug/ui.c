#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

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
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "step few steps",  cmd_si },
  { "info","printf the register states", cmd_info },
  { "x","scanning memory",cmd_x}
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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
static int cmd_si(char *args) {
   char *arg = strtok(NULL," ");
   int n=0;
   sscanf(arg,"%d",&n);
   for(int j=0;j<n;++j){
   cpu_exec(n);
   }
   return 1;
}
//
void print_reg(){
   for (int i=0;i<8;i++){
       printf("%s 0x%08x %d\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
     }
   for (int i=0;i<8;i++){
       printf("%s 0x%08x %d\n",regsw[i],cpu.gpr[i]._16,cpu.gpr[i]._16);
     }
   for(int i=0;i<8;i++){
       for(int j=0;j<2;j++){
           printf("%s 0x%08x %d\n",regsl[i],cpu.gpr[i]._8[j],cpu.gpr[i]._8[j]);
       }
    }
}
static int cmd_info(char *args){
    char *arg=strtok(NULL," ");
    if(strcmp(arg,"r")==0){
    print_reg(); 
    }
    return 1;
}
//
/*static int cmd_x(char *args){
   char *arg1=strtok(NULL," ");
   char *arg2=strtok(NULL," ");
   int len;
   vaddr_t addr;
   sscanf(arg1,"%d",&len);
   sscanf(arg2,"%x",&addr);
   printf("0x%x:",addr);
   for(int j=0;j<len;j++){
       printf("0x%08x ",vaddr_read(addr,4));
       addr+=4;
   }
   printf("\n");
   return 1;
}*/
static int cmd_x(char *args){
   char *arg1=strtok(NULL," ");
   char *arg2=strtok(NULL," ");
   int len;
   vaddr_t addr;
   sscanf(arg1,"%d",&len);
   sscanf(arg2,"%d",&addr);
   for(int j=0;j<len;j++)
   {
     printf("0x%x ",addr);
     printf("0x%08x ",vaddr_read(addr,4));
     for(int k=0;k<4;k++)
     {
       printf("%08x ",vaddr_read(addr,1));
       addr++;
     }
     printf("\n");
   }
     return 1;
}
void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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
