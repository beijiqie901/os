/*
    search keyword: my_swtch new_int8(baidu.com)
    url: http://fayaa.com/code/view/8589/full/
*/

#include<stdio.h>
#include<stdlib.h>
#include "lius/initdos.h"
#include "lius/thread.h"
#include "lius/message.h"
#include "lius/product.h"

void   f1(void);    /*线程1*/
void   f2(void);    /*线程2*/

void pc(void);
void thread1(void);
void thread2(void);

void  main()
{
    disable();        /*屏蔽中断*/

    InitDos();
    InitTCB();
    InitBuff();

    strcpy(tcb[0].name, "main");   /*创建0#线程*/
    tcb[0].state = RUNNING ;
    currentTcb = 0 ;
    /*创建各线程*/
    pc();
    tcbstate();
	tcbstart();
    /*调用my_swtch，实现基于优先权的时间片轮转调度*/
    my_swtch();

    enable();

    while(!finish());
    tcb[0].name[0] = '\0' ;
    tcb[0].state = FINISH ;

	tcbdestroy();
    printf("\nMulti_Task   system   terminated.\n");
    getch();
}

void pc(void)
{
	/*
	create("produce1", (codeptr) thread1, 1024, 3);
	create("consume1", (codeptr) thread2, 1024, 1);
	create("produce2", (codeptr) thread1, 1024, 3);
	create("consume2", (codeptr) thread2, 1024, 5);
	*/
     create("p3", (codeptr) thread1, 1024, 7);
	create("c3", (codeptr) thread2, 1024, 3);
	create("c2", (codeptr) thread2, 1024, 5);

	/*
    create("f1", (codeptr) f1, 1024, 3);
    create("f2", (codeptr) f2, 1024, 1);
	*/
 }

int max=10;
void thread1(void)
{
    int i,j,k;
	for(i=0;i<10;)
	{
		produce(i);
		/*printf("p:%d|  ",i);		*/
          i++;
         printf("pa%d-",currentTcb);
         /**/
		for(j=0;j<10000;j++)
			for(k=0;k<10000;k++);	
          printf("pb");
	}
}
void thread2(void)
{
    int i,j,k,value;
	for(i=0;i<10;)
	{
		value=consume();
		/*printf("c:%d\n  ",value);*/
          i++;
          printf("ca%d-",currentTcb);
         /* */
		for(j= 0;j<10000;j++)
			for(k=0;k<10000;k++);	
       
          printf("cb");
	}
}

/*
int i1=0;
void thread1(void)
{
    int j,k;
	for(;i1<max;)
	{
		produce(i1);
		printf("produce:%d|  ",i1);		
          i1++;
		for(j=0;j<10000;j++)
			for(k=0;k<5000;k++);	
	}
}
int i2=0;
void thread2(void)
{
    int j,k,value;
	for(;i2<max;)
	{
		value=consume();
		printf("consume:%d|  ",value);
          i2++;
		for(j=0;j<10000;j++)
			for(k=0;k<30000;k++);	
	}
}
*/
/**/
void f1(void)
{
    int  i,j,k;
    char a[NTEXT]="HDU Message!";
    for(i=0;i<10 ; i ++)
    {
        putchar('a');
        for(j=0;j<10000;j++)
            for(k=0;k<10000;k++);
    }
    printf("\n");
    for(i=0;i<=3;i++)
    {
        printf("f1 is sending massages to f2!\n");
        send("f2",a,NTEXT);
        for(j=0;j<10000;j++)
            for(k=0;k<10000;k++);
        }
}

void f2(void)
{
    int i,j,k;
    for(i=0;i<10;i++)
    {
        putchar('b');
        for(j=0;j<10000;j++)
            for(k=0;k<10000;k++);
    }
    printf("\n");
    for(i=0;i<=3;i++)
    {
        receiver();
        for(j=0;j<10000;j++)
            for(k=0;k<10000;k++);
    }
}
