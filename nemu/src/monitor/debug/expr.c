#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
//#include <debug.h>
enum {
  TK_NOTYPE = 256, TK_EQ=255,
  TK_16=257,TK_10=258,TK_reg=259,TK_UEQ=254,TK_AND=253,TK_OR=252,TK_NOT=251,DEREF=250,TK_LBA=249,TK_RBA=248

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"0x[a-fA-F0-9]+",TK_16}, //16jinzhi
  {"[0-9]+",TK_10}    ,  // 10jinzhi
  {"$[a-z]{3}",TK_reg},       //register
  {"\\(",TK_LBA},           //left bracket
  {"\\)",TK_RBA},           //right bracket
  {"-",'-'},           //subtract
  {"\\*",'*'},           //multiply
  {"/",'/'},            //divide
  {"!=",TK_UEQ},        //not equal
  {"&&",TK_AND},        //and
  {"\\|\\|",TK_OR},         //or
  {"\\!",TK_NOT},       //not
 // {"\\*",DEREF}        //deref
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];
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

Token tokens[32];
int nr_token;

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
        for(int j=0;j<32;j++){
               tokens[nr_token].str[j]='\0';}
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
           strncpy(tokens[nr_token].str,e+position,substr_len);  
           position+=substr_len;
       // int num=strlen(rules[i].regex);
       // if(num>31) assert(0);
        switch (rules[i].token_type) {
          case '+': tokens[nr_token].type='+';
                    nr_token++;
                    break;
          case '-': tokens[nr_token].type='-';
                    nr_token++;
                    break;
          case TK_EQ: tokens[nr_token].type=TK_EQ;
                    nr_token++;
                    break;
          case TK_16: tokens[nr_token].type=TK_16;
                    nr_token++;
                  //  strcpy(tokens[nr_token].str,rules[i].regex);
                    break;
          case TK_10: tokens[nr_token].type=TK_10;
                    nr_token++;
                   // strncpy(tokens[nr_token].str,e+position,substr_len);
                    break;
          case TK_reg: tokens[nr_token].type=TK_reg;
                       nr_token++;
                    //   strcpy(tokens[nr_token].str,rules[i].regex);
                       break;
          case TK_LBA: tokens[nr_token].type='(';
                    nr_token++;
                    break;
          case TK_RBA: tokens[nr_token].type=')';
                    nr_token++;
                    break; 
          case '*': tokens[nr_token].type='*';
                    nr_token++;
                    break;
          case '/': tokens[nr_token].type='/';
                    nr_token++;
                    break;
          case TK_UEQ: tokens[nr_token].type=TK_UEQ;
                       nr_token++;
                       break;
          case TK_AND: tokens[nr_token].type=TK_AND;
                       nr_token++;
                       break;
          case TK_OR: tokens[nr_token].type=TK_OR;
                      nr_token++;
                      break;
          case TK_NOT: tokens[nr_token].type=TK_NOT;
                       nr_token++;
                       break;
        //  case DERERF: token[i].type=DERERF;
        //               nr_token++; 
        //  default:
        //          assert(0);
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
bool check_parentheses(int p,int q){                  //match bracket
   if(tokens[p].type==TK_LBA&&tokens[q].type==TK_RBA)
  /* {
     int k;//
     int a,b;
     a=p;
     b=q;
     k=0;
     a++;
     b--;
       while(a!=b&&k>=0)
      {
        if(tokens[a].type==TK_LBA){
              k++;}
        else if(tokens[a].type==TK_RBA){
              k--;}
        a++;     
      }
    if(k!=0) return false;
    else return true; 
   }*/
   return true;
   else return false;
}
int getvalue_operation(Token a){
    if(a.type==DEREF||a.type==TK_NOT) return 6;
    else if(a.type=='*'||a.type=='/') return 5;
    else if(a.type=='+'||a.type=='-') return 4;
    else if(a.type==TK_EQ||a.type==TK_UEQ) return 3;
    else if(a.type==TK_AND) return 2;
    else if(a.type==TK_OR) return 1;
    else return 0;
}
int find_dominated_op(int p,int q){
    int i,j;
    int k;
    k=0;
    i=p;
    while(i!=q){
       if(tokens[i].type==TK_LBA){
             k++;}
       else if(tokens[i].type==TK_RBA){
             k--;}
       else if(tokens[i].type<TK_NOTYPE){
             if(k==0){  break;}}
       i++; 
    }
    k=0;  
    j=p;
    while(j<=q){
       if(tokens[j].type==TK_LBA){
             k++;}
       else if(tokens[j].type==TK_RBA){
             k--;}
       else if(tokens[j].type<TK_NOTYPE){
             if(k==0) {  
                 int x=getvalue_operation(tokens[j]);
                 int y=getvalue_operation(tokens[i]);
                 if(x!=y&&x<=y) i=j;}}
       j++;
      }        
      if(i==p) assert(0);
      else return i;            
}
uint32_t eval(int p,int q){
    if(p>q){
      assert(0); 
    }
    else if(p==q){
      int a;
      sscanf(tokens[p].str,"%d",&a);
      return a;
       }
    else if(check_parentheses(p,q)==true){
         printf("12345");
          return eval(p+1,q-1);
    }
    else{
    int op=find_dominated_op(p,q);
      int val1=eval(p,op-1);
      int val2=eval(op+1,q);
    //  printf("%d %d\n",val1,val2); 
      switch(tokens[op].type){
           case '+':return val1+val2;
           case '-':return val1-val2;
           case '*':return val1*val2;
           case '/':{ 
                if(val2==0) assert(0);
                else return val1/val2;
             }
           case TK_EQ:if(val1==val2) return 1;
                      else return 0;
           case TK_UEQ:if(val1!=val2) return 1;
                      else return 0;
           case TK_AND:return val1&&val2;
           case TK_OR:return val1||val2;
           case TK_NOT:return !val2;
           case DEREF:return 1;
           default:assert(0);
         }   
    }
  // return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;//assert(0)?
  }
  else {
          *success=true;
         for(int i=0;i<nr_token;i++){
           if(tokens[i].type=='*'&&(i==0||(tokens[i-1].type!=TK_16&&tokens[i-1].type!=TK_10))){
                tokens[i].type=DEREF;}
           else continue;
         }     
  //    printf("%d\n",nr_token);
      return eval(0,nr_token-1); }
}
