
#include "sdb.h"

#define NR_WP 32


//head 使用中 free_ 空闲
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].exp="";
    wp_pool[i].result=0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void new_wp(char* exp){
  //exp此处为局部变量 不能直接赋值 会被回收 cao

  WP *findWP=free_;

  if (findWP==NULL){     
    printf("\n监视点不够\n");
    assert(0);
  }
  free_=free_->next;

  bool success;
  findWP->exp=(char *)malloc(strlen(exp)+1);
  strcpy(findWP->exp,exp);
  findWP->result=expr(exp,&success);
  
  if(head==NULL){
    head=findWP;
    head->next=NULL;
  }else{
    findWP->next=head;
    head=findWP;
  }

}


bool free_wp(int NO){
  WP* findWp=head,*prior=NULL;
  while(findWp!=NULL &&  findWp->NO != NO){      //可以改进 很有可能wp是一个 
    prior=findWp;
    findWp=findWp->next;
  }
  if(findWp==NULL){
    printf("\n监视点NO错误\n");
    return false;
  }
  if(prior!=NULL)
    prior->next=findWp->next;
  else
    head=head->next;


  if(free_==NULL){
    free_=findWp;
    free_->next=NULL;
  }
  else{
    findWp->next=free_;
    free_=findWp;
    free_->exp="";
    free_->result=0;
  }
  printf("\n监视点:%d 释放成功\n",findWp->NO);

  return true;
}

void print_changed_watch(WP *wp){
  printf("\nOld value:%x\n",wp->result);
  printf("New value:%x\n",wp->changed_result);
  printf("In %s\n\n",wp->exp);

}


bool watch_changed(){   
  WP* findWp=head;
  WP* wp;
  bool success=true;
  bool changed=false;

  while(findWp){
    if(expr(findWp->exp,&success)!=findWp->result){
      wp=findWp;
      wp->changed_result=expr(findWp->exp,&success);
      print_changed_watch(wp);

      changed=true;
    }
    findWp=findWp->next;
  }

  return changed;
}


void watch_display(){
    WP* findWp=head;

    printf("\nNum  exp        result");
    while(findWp){
      bool success=false;

      uint32_t  result=expr(findWp->exp,&success);
      findWp->result=result;
      printf("\n%-4d %s %14x",findWp->NO,findWp->exp,findWp->result);

      findWp=findWp->next;
    }
    printf("\n");

}