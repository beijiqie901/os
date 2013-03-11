#include "lius/thread.h"
#include<stdlib.h>    /*用于malloc*/
#include<string.h>    /*用于strcmp与strcpy*/

#define NBUF 10        /*空闲缓冲区的数量*/

struct buffer* freebuff;        /*空闲消息缓冲队列的结构体定义*/

semaphore sfb = {NBUF, NULL};    /*空闲缓冲区的同步信号量*/
semaphore mutexbf = {1, NULL};    /*对空闲缓冲区处理的互斥信号量*/
void   InitBuff(void);                /*初始化空闲消息缓冲队列*/
void   insbuf(struct buffer **mq, struct buffer *buff);        /*消息的插入一个TCB的消息队列的操作*/
struct buffer * Getbuf(void);                                /*获取一个空闲的消息块*/
void   send(char *receiver, char *a, int size);                /*实现将消息发给指定的线程*/
void   receive(struct buffer *b);                            /*接收消息后，将消息块从TCB的消息队列中取下*/
struct buffer * sender(struct buffer **mq);                /*发送者线程要完成的功能，将消息块还给空闲消息缓冲队列*/
                                     
void   receiver(void);                                        /*接收者线程要完成的功能，消息接收*/


void InitBuff(void)
{
    int i;
    struct buffer *temp1, *temp2 ;
    /*在堆上分配各个消息块的空间*/
    temp1 = (struct buffer *)malloc(sizeof(struct buffer));
    freebuff = temp1 ;
    for(i = 0 ; i < NBUF ; i ++)  /*链接成一个队列*/
    {
        temp2 = (struct buffer *)malloc(sizeof(struct buffer)) ;
        temp1->next = temp2 ;
        temp1 = temp2 ;
    }
    temp1->next = NULL ;
}/*初始化空闲消息缓冲队列，并链接成一个队列*/

void   insbuf(struct buffer **mq, struct buffer *buff)
{
    struct buffer *temp;
    if(buff == NULL)
        return ;
    buff->next = NULL ;
    if(*mq == NULL)
        *mq = buff ;
    else{
        temp = *mq ;
        while(temp->next != NULL)
            temp = temp->next ;
        temp->next = buff ;
    }
}/*将buf所指的缓冲区插入到mq所指的缓冲队列末尾*/

struct buffer * Getbuf(void)
{
    struct buffer *buflist;
    buflist = freebuff ;
    freebuff = freebuff->next ;
    return(buflist);
}/*从空闲消息缓冲队列队头取下一个缓冲区*/

/*实现将消息发给指定的线程*/
void   send(char *receiver, char *a, int size)
{
    struct buffer *buff;
    int i , id = -1 ;
    disable();
    for(i = 1 ; i < NTCB ; i ++)
    {
        if(strcmp(receiver, tcb[i].name) == 0)/*判断传入的外部标识符与哪个TCB外部标识符相同*/
        {   
            id = i ; /*id用于寻找receiver的内部标识符*/
            break ;
        }
    }
    if(id == -1)/*没有找到与receive相同的TCB外部标识符*/
    {
        printf("ERROR:Receiver not exist!\n");
        enable();
        return ;
    }

    p(&sfb);        /*实现空闲消息块的同步*/
    p(&mutexbf);    /*实现对空闲消息缓冲区的互斥操作*/
    buff = Getbuf();
    v(&mutexbf);
    /*将传入的消息记录下来*/
    buff->sender = currentTcb ;
    buff->size = size ;
    buff->next = NULL ;
    for(i = 0 ; i < buff->size ;  i ++ )
        buff->text[i] = *(a+i) ;

    p(&tcb[id].mutex);    /*实现对TCB中的消息队列互斥操作*/
    insbuf(&(tcb[id].mq), buff);
    v(&tcb[id].mutex);
    v(&tcb[id].sm);        /*实现TCB中发送与接收消息的同步*/
    enable();
}

void receive(struct buffer *b)
{
    struct buffer *temp ;
    temp = freebuff ;
    while(temp->next != NULL)
        temp = temp->next ;
    temp->next = b ;
}/*接收消息后，将消息块从TCB的消息队列中取下*/
struct buffer * sender(struct buffer **qp)
{
    struct buffer *temp;
    temp = (*qp) ;
    (*qp) = (*qp)->next ;
    temp->next = NULL ;
    return(temp);
}/*接收消息后，将消息块还给空闲消息缓冲队列*/

/*实现消息的接收*/
void   receiver(void)
{
    struct buffer *buff = NULL;

    disable();
    if(tcb[currentTcb].mq == NULL)/*当TCB中的消息队列为空时*/
    {
        printf("now there is no massage in function %s\n", tcb[currentTcb].name);
        enable();
        return ;
    }
    enable();
    p(&tcb[currentTcb].sm);        /*实现TCB中发送与接收消息的同步*/
    p(&tcb[currentTcb].mutex);    /*实现对TCB中取消息的互斥操作*/
    buff = sender(&(tcb[currentTcb].mq));
    v(&tcb[currentTcb].mutex);
    /*在终端上显示各线程发来的消息内容*/
    printf("%s receive a message from %s:  ", tcb[currentTcb].name, tcb[buff->sender].name );
    printf("%s\n\n", buff->text);

    p(&mutexbf);    /*实现对空闲消息缓冲区的互斥操作*/
    receive(buff);
    v(&mutexbf);
    v(&sfb);        /*实现空闲消息块的同步*/
}
