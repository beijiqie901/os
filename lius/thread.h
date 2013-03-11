#ifndef LIUS_THREAD_H
#define LIUS_THREAD_H

#include<stdio.h>
#include<stdlib.h>    /*用于malloc*/
#include<string.h>    /*用于strcmp与strcpy*/

#define NTCB 10       /*TCB 所用的数量*/
#define NTEXT 50    /*在线程间传送时，信息上限*/

/*线程的五种状态，即开始，完成，运行，就绪和阻塞*/
#define START -1   
#define FINISH 0
#define RUNNING 1
#define READY 2
#define BLOCK 3

#define TIMEOUT 2    /*线程时间片轮转的时间*/

typedef int (far* codeptr)();   
typedef struct{
    int value ;
    struct TCB *wq ;
}semaphore ;    /*信号量结构体的定义*/

struct buffer{
    int sender ;
    int size ;
    char text[NTEXT];
    struct buffer *next ;
};

struct TCB{
    unsigned char *stack;    /*线程私有堆栈的始址*/
    unsigned ss ;            /*堆栈的段址*/
    unsigned sp ;            /*堆栈的栈顶指针*/
    int priority ;            /*线程的优先级*/
    char state ;            /*线程的状态标志*/
    char name[10] ;            /*线程的外部标识符*/
    struct buffer *mq ;        /*消息队列排队使用*/
    semaphore mutex ;        /*互斥信号量，用于对TCB消息队列处理时互斥*/
    semaphore sm ;            /*线程同步信号量*/
    struct TCB *next ;
}tcb[NTCB];        /*线程控制块TCB的结构体定义*/
typedef struct TCB *READYQUEUE;  /*就绪队列*/
READYQUEUE readyhead;

/*定义一个线程私有堆栈结构体*/
struct int_regs{
    unsigned BP, DI, SI, DS, ES, DX, CX, BX, AX, I P, CS, Flags, Off, Seg;
};   

int TimeCount = 0 ;                /*时间计数为0*/
int currentTcb = -1 ;            /*当前TCB初始为-1*/

void   interrupt (*old_int8)(void);    /*用于记录原时间中断函数的入口指针*/

void   InitTCB(void);                /*初始化线程控制块TCB*/

int    create(char *name, codeptr code, int stacklen, int priority );  /*创建线程*/
void   over(void);                /*撤销已运行完成的线程*/
void   Destroy(int i);           /*撤销当前线程*/
int    finish(void);        /*判断所有子线程是否已经完成*/

int    SeekNext(void);                /*当在时间中断调度或其他原因调度时，寻找下一个要执行的TCB,返回TCB内部标识符*/
void   interrupt new_int8(void);    /*时间中断调度*/
void   interrupt my_swtch(void);    /*其他原因调度*/

void   block(struct TCB **qp);            /*线程的阻塞*/
void   wakeup(struct TCB **qp);    /*线程的唤醒*/
void   p(semaphore *sem);        /*信号量的P操作*/
void   v(semaphore *sem);        /*信号量的V操作*/

void   tcbstate(void);

void InitTCB(void)
{
    int i ;
    for(i = 0 ; i < NTCB; i ++)
    {/*初始化各个TCB*/
        tcb[i].priority = 0 ;
        tcb[i].state = START ;   
        tcb[i].name[0] = '\0' ;
        tcb[i].mq = NULL ;
        tcb[i].mutex.value = 1;
        tcb[i].mutex.wq = NULL;
        tcb[i].sm.value = 0 ;
        tcb[i].sm.wq = NULL ;
        tcb[i].next = NULL ;
    }
}/*初始化线程控制块TCB*/

int   create(char *name, codeptr code, int stacklen, int priority )
{
    struct int_regs *stack ;
    int freetcb;
    /*寻找一个空余的TCB*/
    for(freetcb = 1 ; freetcb < NTCB ; freetcb ++)
    {
        if(tcb[freetcb].state == START || tcb[freetcb].state == FINISH)
            break;
    }

    if(freetcb == NTCB)/*没有空余的TCB时*/
    {
        printf("Sorry,create was failed\n");
        return -1;
    }

    /*初始化TCB中各变量的值*/
    tcb[freetcb].priority = priority ;
    tcb[freetcb].state = READY ;
    strcpy(tcb[freetcb].name, name) ;

    tcb[freetcb].stack = (unsigned char *)malloc(stacklen);
    tcb[freetcb].stack = tcb[freetcb].stack + stacklen ;
    stack = (struct int_regs *)tcb[freetcb].stack-1;
    /*TCB私有堆栈的赋值*/
    stack->DS = _DS;
    stack->ES = _ES;
    stack->Flags = 0x200;
    stack->CS = FP_SEG(code);
    stack->IP = FP_OFF(code);
    /*over函数压入堆栈，在线程结束时，作为返回值*/
    stack->Off=FP_OFF(over);
    stack->Seg=FP_SEG(over);
    /*将TCB私有堆栈的段址和栈顶指针记录到TCB中，在TCB切换时要用*/
    tcb[freetcb].ss = FP_SEG(stack);
    tcb[freetcb].sp = FP_OFF(stack);
   
    return freetcb ;
}/*创建线程*/

void interrupt new_int8(void)
{
    disable();
    (*old_int8)();
    TimeCount++;
    if(TimeCount >= TIMEOUT)/*时间片到时*/
    {
        if(!DosBusy())/*当DOS不忙时*/
        {
            /*实现时间中断调度时的CUP现场保护与TCB私有堆栈切换*/
            tcb[currentTcb].ss = _SS ;
            tcb[currentTcb].sp = _SP ;
            if(tcb[currentTcb].state == RUNNING)
                tcb[currentTcb].state = READY ;
            currentTcb = SeekNext();
            _SS = tcb[currentTcb].ss ;
            _SP = tcb[currentTcb].sp ;
            tcb[currentTcb].state = RUNNING ;
            TimeCount = 0 ;
        }
    }
    enable();
    
}/*时间中断调度*/
void  interrupt my_swtch(void)
{
    disable();
    /*实现其他原因的调度，完成CPU现场保护与TCB私有堆栈切换*/
    if(tcb[currentTcb].state == FINISH )/*当调度前的TCB已经结束*/
    {
        currentTcb = SeekNext() ;
        tcb[currentTcb].state = RUNNING ;
        _SS = tcb[currentTcb].ss ;
        _SP = tcb[currentTcb].sp ;
    }
    else
    {
        tcb[currentTcb].ss = _SS ;
        tcb[currentTcb].sp = _SP ;
        if(tcb[currentTcb].state == RUNNING)
            tcb[currentTcb].state = READY ;
        currentTcb = SeekNext() ;
        tcb[currentTcb].state = RUNNING ;
        _SS = tcb[currentTcb].ss ;
        _SP = tcb[currentTcb].sp ;
    }
  
    enable();
}/*其他原因调度*/

void  over(void)
{
    Destroy(currentTcb);
    printf("%s has finished!\n", tcb[currentTcb].name);
    /*printf("I am here! \n");*/
    my_swtch();
   
}/*撤销已运行完成的线程*/
void Destroy(int i)
{
   
    if(tcb[i].state==RUNNING)  
    {   
         disable();   
         tcb[i].state=FINISH; 
         free(tcb[i].stack);    
         enable();
    }
    return;
}/*撤销当前线程*/

int finish(void)
{
    int i ;
    disable();
    for(i = 1 ; i < NTCB ; i ++)
    {
        if(tcb[i].state == RUNNING || tcb[i].state ==READY || tcb[i].state == BLOCK )
        {
            enable();
            return 0 ;
        }
    }
    enable();
    return 1 ;
}/*判断所有子线程是否已经完成*/

int  SeekNext(void)
{
    int i, n_tcb = 0, pri = 0 ;
    disable();
    /*实现寻找一个优先级最高并在READY状态下的TCB*/
    for(i = 1 ; i < NTCB ; i ++)
    {
        if(tcb[i].state == READY && tcb[i].priority > pri)
        {
            n_tcb = i ;
            pri = tcb[n_tcb].priority ;
        }
    }
    /*实现TCB的优先级的变化*/
    for(i = 1 ; i < NTCB && tcb[i].state == READY ; i ++)
    {
        if (i != n_tcb)
        {
            tcb[i].priority ++ ;
        }
    }
       printf("  next %d\n",n_tcb);
    enable();
   return n_tcb;
}/*当在时间中断调度或其他原因调度时，寻找下一个要执行的TCB,返回TCB内部标识符*/

void  tcbstate(void)
{
  int i;
  for(i=0;i<NTCB;i++)
  {
     switch(tcb[i].state)
     {
       case START:
         printf("\n tcb[%d] %s\'s state is starting;",i,tcb[i].name);
         break;
       case FINISH:
         printf("\n tcb[%d] %s\'s state is finished;",i,tcb[i].name);
         break;
       case RUNNING:
         printf("\n tcb[%d] %s\'s state is running;",i,tcb[i].name);
         break;
       case READY:
         printf("\n tcb[%d] %s\'s state is ready;",i,tcb[i].name);
         break;
       case BLOCK:
         printf("\n tcb[%d] %s\'s state is blocked;",i,tcb[i].name);
         break;
      }
  }
  printf("\n");
}

void   tcbstart(void)
{
	old_int8 = getvect(8);
    /*启动多个线程的并发执行*/
    setvect(8, new_int8);
}
void   tcbdestroy(void)
{
    /*所有线程结束的，还原原先的时间中断*/
    setvect(8, old_int8);
}


/*************************   pv  ***************************/

void   block(struct TCB **qp)
{
    struct TCB *temp ;
    disable();
    tcb[currentTcb].state = BLOCK ;
    if((*qp) == NULL)/*当阻塞队列中无其他线程时*/
        (*qp) = &tcb[currentTcb] ;/*将TCB加入阻塞队列中*/
    else
    {
        temp = *qp ;
        while(temp->next != NULL)/*找到阻塞队列中最后的插入位置*/
            temp = temp->next ;
        temp->next = &tcb[currentTcb];/*将TCB加入阻塞队列中*/
    }
    tcb[currentTcb].next = NULL ;
    my_swtch() ;/*因阻塞而调度，调度到下一个要执行的TCB*/
    enable();   
}/*线程的阻塞*/
void   wakeup(struct TCB **qp)
{
    struct TCB *p;
    struct TCB *q;
    q = (*qp);
    if(q!=NULL)
    {
        (*qp) = (*qp)->next ;
        q->state = READY ;    /*将线程状态设置为READY*/
        q->next = NULL;
        p=readyhead->next;
        while(p&&p->next)
        { p=p->next;
        }
        p->next=q;
    }   
}/*线程的唤醒*/
void   p(semaphore *sem)
{
    struct TCB **qp;
    disable();
    sem->value = sem->value-1 ;
    if(sem->value < 0)/*判断信号量中value值，是否应该阻塞当前线程*/
    {
        qp = &(sem->wq);
        block(qp);
    }
    enable() ;
 }/*信号量的P操作*/
void   v(semaphore *sem)
{
    struct TCB **qp;
    disable();
    qp = &(sem->wq);
    sem->value = sem->value+1;
    if(sem->value <= 0)/*判断信号量中value值，是否应该唤醒因这个信号量互斥操作，而阻塞的线程*/
        wakeup(qp);
    enable();
}/*信号量的V操作*/


#endif
