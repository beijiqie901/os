#ifndef LIUS_THREAD_H
#define LIUS_THREAD_H

#include<stdio.h>
#include<stdlib.h>    /*����malloc*/
#include<string.h>    /*����strcmp��strcpy*/

#define NTCB 10       /*TCB ���õ�����*/
#define NTEXT 50    /*���̼߳䴫��ʱ����Ϣ����*/

/*�̵߳�����״̬������ʼ����ɣ����У�����������*/
#define START -1   
#define FINISH 0
#define RUNNING 1
#define READY 2
#define BLOCK 3

#define TIMEOUT 2    /*�߳�ʱ��Ƭ��ת��ʱ��*/

typedef int (far* codeptr)();   
typedef struct{
    int value ;
    struct TCB *wq ;
}semaphore ;    /*�ź����ṹ��Ķ���*/

struct buffer{
    int sender ;
    int size ;
    char text[NTEXT];
    struct buffer *next ;
};

struct TCB{
    unsigned char *stack;    /*�߳�˽�ж�ջ��ʼַ*/
    unsigned ss ;            /*��ջ�Ķ�ַ*/
    unsigned sp ;            /*��ջ��ջ��ָ��*/
    int priority ;            /*�̵߳����ȼ�*/
    char state ;            /*�̵߳�״̬��־*/
    char name[10] ;            /*�̵߳��ⲿ��ʶ��*/
    struct buffer *mq ;        /*��Ϣ�����Ŷ�ʹ��*/
    semaphore mutex ;        /*�����ź��������ڶ�TCB��Ϣ���д���ʱ����*/
    semaphore sm ;            /*�߳�ͬ���ź���*/
    struct TCB *next ;
}tcb[NTCB];        /*�߳̿��ƿ�TCB�Ľṹ�嶨��*/
typedef struct TCB *READYQUEUE;  /*��������*/
READYQUEUE readyhead;

/*����һ���߳�˽�ж�ջ�ṹ��*/
struct int_regs{
    unsigned BP, DI, SI, DS, ES, DX, CX, BX, AX, I P, CS, Flags, Off, Seg;
};   

int TimeCount = 0 ;                /*ʱ�����Ϊ0*/
int currentTcb = -1 ;            /*��ǰTCB��ʼΪ-1*/

void   interrupt (*old_int8)(void);    /*���ڼ�¼ԭʱ���жϺ��������ָ��*/

void   InitTCB(void);                /*��ʼ���߳̿��ƿ�TCB*/

int    create(char *name, codeptr code, int stacklen, int priority );  /*�����߳�*/
void   over(void);                /*������������ɵ��߳�*/
void   Destroy(int i);           /*������ǰ�߳�*/
int    finish(void);        /*�ж��������߳��Ƿ��Ѿ����*/

int    SeekNext(void);                /*����ʱ���жϵ��Ȼ�����ԭ�����ʱ��Ѱ����һ��Ҫִ�е�TCB,����TCB�ڲ���ʶ��*/
void   interrupt new_int8(void);    /*ʱ���жϵ���*/
void   interrupt my_swtch(void);    /*����ԭ�����*/

void   block(struct TCB **qp);            /*�̵߳�����*/
void   wakeup(struct TCB **qp);    /*�̵߳Ļ���*/
void   p(semaphore *sem);        /*�ź�����P����*/
void   v(semaphore *sem);        /*�ź�����V����*/

void   tcbstate(void);

void InitTCB(void)
{
    int i ;
    for(i = 0 ; i < NTCB; i ++)
    {/*��ʼ������TCB*/
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
}/*��ʼ���߳̿��ƿ�TCB*/

int   create(char *name, codeptr code, int stacklen, int priority )
{
    struct int_regs *stack ;
    int freetcb;
    /*Ѱ��һ�������TCB*/
    for(freetcb = 1 ; freetcb < NTCB ; freetcb ++)
    {
        if(tcb[freetcb].state == START || tcb[freetcb].state == FINISH)
            break;
    }

    if(freetcb == NTCB)/*û�п����TCBʱ*/
    {
        printf("Sorry,create was failed\n");
        return -1;
    }

    /*��ʼ��TCB�и�������ֵ*/
    tcb[freetcb].priority = priority ;
    tcb[freetcb].state = READY ;
    strcpy(tcb[freetcb].name, name) ;

    tcb[freetcb].stack = (unsigned char *)malloc(stacklen);
    tcb[freetcb].stack = tcb[freetcb].stack + stacklen ;
    stack = (struct int_regs *)tcb[freetcb].stack-1;
    /*TCB˽�ж�ջ�ĸ�ֵ*/
    stack->DS = _DS;
    stack->ES = _ES;
    stack->Flags = 0x200;
    stack->CS = FP_SEG(code);
    stack->IP = FP_OFF(code);
    /*over����ѹ���ջ�����߳̽���ʱ����Ϊ����ֵ*/
    stack->Off=FP_OFF(over);
    stack->Seg=FP_SEG(over);
    /*��TCB˽�ж�ջ�Ķ�ַ��ջ��ָ���¼��TCB�У���TCB�л�ʱҪ��*/
    tcb[freetcb].ss = FP_SEG(stack);
    tcb[freetcb].sp = FP_OFF(stack);
   
    return freetcb ;
}/*�����߳�*/

void interrupt new_int8(void)
{
    disable();
    (*old_int8)();
    TimeCount++;
    if(TimeCount >= TIMEOUT)/*ʱ��Ƭ��ʱ*/
    {
        if(!DosBusy())/*��DOS��æʱ*/
        {
            /*ʵ��ʱ���жϵ���ʱ��CUP�ֳ�������TCB˽�ж�ջ�л�*/
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
    
}/*ʱ���жϵ���*/
void  interrupt my_swtch(void)
{
    disable();
    /*ʵ������ԭ��ĵ��ȣ����CPU�ֳ�������TCB˽�ж�ջ�л�*/
    if(tcb[currentTcb].state == FINISH )/*������ǰ��TCB�Ѿ�����*/
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
}/*����ԭ�����*/

void  over(void)
{
    Destroy(currentTcb);
    printf("%s has finished!\n", tcb[currentTcb].name);
    /*printf("I am here! \n");*/
    my_swtch();
   
}/*������������ɵ��߳�*/
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
}/*������ǰ�߳�*/

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
}/*�ж��������߳��Ƿ��Ѿ����*/

int  SeekNext(void)
{
    int i, n_tcb = 0, pri = 0 ;
    disable();
    /*ʵ��Ѱ��һ�����ȼ���߲���READY״̬�µ�TCB*/
    for(i = 1 ; i < NTCB ; i ++)
    {
        if(tcb[i].state == READY && tcb[i].priority > pri)
        {
            n_tcb = i ;
            pri = tcb[n_tcb].priority ;
        }
    }
    /*ʵ��TCB�����ȼ��ı仯*/
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
}/*����ʱ���жϵ��Ȼ�����ԭ�����ʱ��Ѱ����һ��Ҫִ�е�TCB,����TCB�ڲ���ʶ��*/

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
    /*��������̵߳Ĳ���ִ��*/
    setvect(8, new_int8);
}
void   tcbdestroy(void)
{
    /*�����߳̽����ģ���ԭԭ�ȵ�ʱ���ж�*/
    setvect(8, old_int8);
}


/*************************   pv  ***************************/

void   block(struct TCB **qp)
{
    struct TCB *temp ;
    disable();
    tcb[currentTcb].state = BLOCK ;
    if((*qp) == NULL)/*�������������������߳�ʱ*/
        (*qp) = &tcb[currentTcb] ;/*��TCB��������������*/
    else
    {
        temp = *qp ;
        while(temp->next != NULL)/*�ҵ��������������Ĳ���λ��*/
            temp = temp->next ;
        temp->next = &tcb[currentTcb];/*��TCB��������������*/
    }
    tcb[currentTcb].next = NULL ;
    my_swtch() ;/*�����������ȣ����ȵ���һ��Ҫִ�е�TCB*/
    enable();   
}/*�̵߳�����*/
void   wakeup(struct TCB **qp)
{
    struct TCB *p;
    struct TCB *q;
    q = (*qp);
    if(q!=NULL)
    {
        (*qp) = (*qp)->next ;
        q->state = READY ;    /*���߳�״̬����ΪREADY*/
        q->next = NULL;
        p=readyhead->next;
        while(p&&p->next)
        { p=p->next;
        }
        p->next=q;
    }   
}/*�̵߳Ļ���*/
void   p(semaphore *sem)
{
    struct TCB **qp;
    disable();
    sem->value = sem->value-1 ;
    if(sem->value < 0)/*�ж��ź�����valueֵ���Ƿ�Ӧ��������ǰ�߳�*/
    {
        qp = &(sem->wq);
        block(qp);
    }
    enable() ;
 }/*�ź�����P����*/
void   v(semaphore *sem)
{
    struct TCB **qp;
    disable();
    qp = &(sem->wq);
    sem->value = sem->value+1;
    if(sem->value <= 0)/*�ж��ź�����valueֵ���Ƿ�Ӧ�û���������ź���������������������߳�*/
        wakeup(qp);
    enable();
}/*�ź�����V����*/


#endif
