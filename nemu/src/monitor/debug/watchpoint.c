#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static int NO_=-1;
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
     p->old_val=n;
     p->next=head;
     head=p;
     NO_++;
     head->NO=NO_;
     return NO_;  
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
            p->NO=-1;
            p->old_val=p->new_val=0;
            p->next=free_;
            free_=p;
            return true;
         } 
    else {
            q->next=p->next;
            strcpy(p->expr,"");
            p->NO=-1;
            p->old_val=p->new_val=0;
            p->next=free_;
            free_=p;
            return true;
         }  
}
void list_watchpoint()
{
    printf("NO Expr    Old Value\n");
    WP *p;
    for(int i=0;i<=NO_;i++)
    {
        p=head;
        printf("%d  ",NO_);
        while(p!=NULL)
        {
           if(p->NO==i)
           {
              printf("%s    0x%08x",p->expr,p->old_val);
              break;
           }
           p=p->next;
        }
        printf("\n");
    }  
}
WP* scan_watchpoint()
{
   WP *p;
   int flag=0;
   for(int i=0;i<NO_;i++)
  {
     p=head;
       while(p!=NULL)
     {
        if(p->NO==i)
        {
           if(p->new_val!=p->old_val)
           {
              flag=1; 
              break;
           }
        }
         p=p->next; 
     }
     if(flag==1) break;
  }
 if(flag==1) return p;
 else return NULL; 
}  
