#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
//static int NO_=-1;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp()
{
    if(free_==NULL) assert(0);
    WP *p;
    p=free_;
    if(free_->next==NULL) free_=NULL;
    else free_=free_->next;
    p->next=NULL;
    return p;
}
void free_wp(WP *wp)
{
    wp->next=NULL;
    if(free_==NULL)
    {
       free_=wp;
       free_->next=NULL;
    }
    else{
      WP *q;
      q=free_;
      free_=wp;
      free_->next=q;
      }
}
int set_watchpoint(char *e)
{
     bool success=false;
     uint32_t n=expr(e,&success);
     WP *p=new_wp();
     strcpy(p->expr,e);
     p->new_val=p->old_val=n;
     p->next=head;
     head=p;
     printf("Set watchpoint #%d\n",p->NO);
     printf("expr      = %s\n",p->expr);
     printf("old value = 0x%08x\n",p->old_val);
    // NO_++;
    // head->NO=NO_;
     return p->NO;  
} 
bool delete_watchpoint(int NO)
{
     if(head==NULL) return false;
     WP *p=head;
     WP *q=NULL;
     while(p!=NULL)
     {
         if(p->NO==NO) break;
         else{
            q=p;
            p=p->next;
          } 
     }
     if(p==NULL)  return false;
     else if(p==head)
         {
            head=head->next;
            strcpy(p->expr,"");
            p->old_val=p->new_val=0;
            free_wp(p);
            return true;
         } 
    else {
            q->next=p->next;
            strcpy(p->expr,"");
            p->old_val=p->new_val=0;
            free_wp(p);
            return true;
         }  
}
void list_watchpoint()
{
    printf("NO Expr    Old Value\n");
    WP *p;
    p=head;
     while(p!=NULL)
     {
          
         printf("%d  %s    0x%08x\n",p->NO,p->expr,p->old_val);
         p=p->next;
     }  
}
WP* scan_watchpoint()
{
   WP *p;
   int flag=0;
   uint32_t n;
   p=head;
   bool success=false;
   while(p!=NULL)
   {
      n=expr(p->expr,&success);
      p->new_val=n;
      if(p->new_val!=p->old_val)
      {
         flag=1;
         break;
      }
      p=p->next;
   }
 if(flag==1) return p;
 else return NULL; 
}  
