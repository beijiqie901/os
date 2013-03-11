#ifndef LIUS_PRODUCT_H
#define LIUS_PRODUCT_H

#include "lius/thread.h"
#define COUNT 5

int buffer_pc[COUNT];
int point_in=0;
int point_out=0;
semaphore sync_empty = {COUNT, NULL};    /* ��д��buffer��Ŀ */
semaphore sync_full = {0, NULL};   /*  д��buffer����Ŀ */
semaphore mutex_in = {1, NULL};    /*д����������Ļ����ź���*/
semaphore mutex_out = {1, NULL};    /*������������Ļ����ź���*/

void produce(int value);
int consume();

void produce(int value)
{
	/*printf("\nin produce %s----",tcb[currentTcb].name);*/
	p(&sync_empty);
	p(&mutex_in);

	buffer_pc[point_in]=value;
	printf(":[%d]-%d ",point_in,value);
	point_in=(point_in+1)%COUNT;

	v(&mutex_in);
	printf("--pin-full");
	v(&sync_full);
	printf("--pover--%d",sync_full.value);
}

int consume()
{
	int value;
	/*printf("\n  in consume  %s--%d-",tcb[currentTcb].name,sync_full.value);*/
	p(&sync_full);
	p(&mutex_in);

	value=buffer_pc[point_out];
	printf("  %s:[%d]-%d ",tcb[currentTcb].name,point_out,value);
	
	point_out=(point_out+1)%COUNT;

	v(&mutex_in);
	v(&sync_empty);
	printf("cover");
	return value;
}



#endif