#include "lius/thread.h"
#include<stdlib.h>    /*����malloc*/
#include<string.h>    /*����strcmp��strcpy*/

#define NBUF 10        /*���л�����������*/

struct buffer* freebuff;        /*������Ϣ������еĽṹ�嶨��*/

semaphore sfb = {NBUF, NULL};    /*���л�������ͬ���ź���*/
semaphore mutexbf = {1, NULL};    /*�Կ��л���������Ļ����ź���*/
void   InitBuff(void);                /*��ʼ��������Ϣ�������*/
void   insbuf(struct buffer **mq, struct buffer *buff);        /*��Ϣ�Ĳ���һ��TCB����Ϣ���еĲ���*/
struct buffer * Getbuf(void);                                /*��ȡһ�����е���Ϣ��*/
void   send(char *receiver, char *a, int size);                /*ʵ�ֽ���Ϣ����ָ�����߳�*/
void   receive(struct buffer *b);                            /*������Ϣ�󣬽���Ϣ���TCB����Ϣ������ȡ��*/
struct buffer * sender(struct buffer **mq);                /*�������߳�Ҫ��ɵĹ��ܣ�����Ϣ�黹��������Ϣ�������*/
                                     
void   receiver(void);                                        /*�������߳�Ҫ��ɵĹ��ܣ���Ϣ����*/


void InitBuff(void)
{
    int i;
    struct buffer *temp1, *temp2 ;
    /*�ڶ��Ϸ��������Ϣ��Ŀռ�*/
    temp1 = (struct buffer *)malloc(sizeof(struct buffer));
    freebuff = temp1 ;
    for(i = 0 ; i < NBUF ; i ++)  /*���ӳ�һ������*/
    {
        temp2 = (struct buffer *)malloc(sizeof(struct buffer)) ;
        temp1->next = temp2 ;
        temp1 = temp2 ;
    }
    temp1->next = NULL ;
}/*��ʼ��������Ϣ������У������ӳ�һ������*/

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
}/*��buf��ָ�Ļ��������뵽mq��ָ�Ļ������ĩβ*/

struct buffer * Getbuf(void)
{
    struct buffer *buflist;
    buflist = freebuff ;
    freebuff = freebuff->next ;
    return(buflist);
}/*�ӿ�����Ϣ������ж�ͷȡ��һ��������*/

/*ʵ�ֽ���Ϣ����ָ�����߳�*/
void   send(char *receiver, char *a, int size)
{
    struct buffer *buff;
    int i , id = -1 ;
    disable();
    for(i = 1 ; i < NTCB ; i ++)
    {
        if(strcmp(receiver, tcb[i].name) == 0)/*�жϴ�����ⲿ��ʶ�����ĸ�TCB�ⲿ��ʶ����ͬ*/
        {   
            id = i ; /*id����Ѱ��receiver���ڲ���ʶ��*/
            break ;
        }
    }
    if(id == -1)/*û���ҵ���receive��ͬ��TCB�ⲿ��ʶ��*/
    {
        printf("ERROR:Receiver not exist!\n");
        enable();
        return ;
    }

    p(&sfb);        /*ʵ�ֿ�����Ϣ���ͬ��*/
    p(&mutexbf);    /*ʵ�ֶԿ�����Ϣ�������Ļ������*/
    buff = Getbuf();
    v(&mutexbf);
    /*���������Ϣ��¼����*/
    buff->sender = currentTcb ;
    buff->size = size ;
    buff->next = NULL ;
    for(i = 0 ; i < buff->size ;  i ++ )
        buff->text[i] = *(a+i) ;

    p(&tcb[id].mutex);    /*ʵ�ֶ�TCB�е���Ϣ���л������*/
    insbuf(&(tcb[id].mq), buff);
    v(&tcb[id].mutex);
    v(&tcb[id].sm);        /*ʵ��TCB�з����������Ϣ��ͬ��*/
    enable();
}

void receive(struct buffer *b)
{
    struct buffer *temp ;
    temp = freebuff ;
    while(temp->next != NULL)
        temp = temp->next ;
    temp->next = b ;
}/*������Ϣ�󣬽���Ϣ���TCB����Ϣ������ȡ��*/
struct buffer * sender(struct buffer **qp)
{
    struct buffer *temp;
    temp = (*qp) ;
    (*qp) = (*qp)->next ;
    temp->next = NULL ;
    return(temp);
}/*������Ϣ�󣬽���Ϣ�黹��������Ϣ�������*/

/*ʵ����Ϣ�Ľ���*/
void   receiver(void)
{
    struct buffer *buff = NULL;

    disable();
    if(tcb[currentTcb].mq == NULL)/*��TCB�е���Ϣ����Ϊ��ʱ*/
    {
        printf("now there is no massage in function %s\n", tcb[currentTcb].name);
        enable();
        return ;
    }
    enable();
    p(&tcb[currentTcb].sm);        /*ʵ��TCB�з����������Ϣ��ͬ��*/
    p(&tcb[currentTcb].mutex);    /*ʵ�ֶ�TCB��ȡ��Ϣ�Ļ������*/
    buff = sender(&(tcb[currentTcb].mq));
    v(&tcb[currentTcb].mutex);
    /*���ն�����ʾ���̷߳�������Ϣ����*/
    printf("%s receive a message from %s:  ", tcb[currentTcb].name, tcb[buff->sender].name );
    printf("%s\n\n", buff->text);

    p(&mutexbf);    /*ʵ�ֶԿ�����Ϣ�������Ļ������*/
    receive(buff);
    v(&mutexbf);
    v(&sfb);        /*ʵ�ֿ�����Ϣ���ͬ��*/
}
