#include<dos.h>

#define GET_INDOS 0X34
#define GET_GRIT_ERR 0X5D06

char far *indos_ptr = 0 ;       
char far *crit_err_ptr = 0 ;

void   InitDos(void);                /*初始化DOS,获得InDos标志和严重错误标志criterr的地址*/
int    DosBusy(void);                /*用于判断当前DOS是否处于繁忙状态*/

void   InitDos()         
{
    union   REGS   regs;
    struct   SREGS   segregs;
    regs.h.ah=GET_INDOS;         /*使用34H号系统功能调用*/
    intdosx(&regs,&regs,&segregs);
    indos_ptr=MK_FP(segregs.es,regs.x.bx);
    if(_osmajor<3)
        crit_err_ptr=indos_ptr+1;       /*严重错误在INDOS后一字节处*/
    else   if(_osmajor==3&&_osmajor==0)
        crit_err_ptr=indos_ptr-1;       /*严重错误在INDOS前一字节处*/
    else
    {
        regs.x.ax=GET_GRIT_ERR;
        intdosx(&regs,&regs,&segregs);
        crit_err_ptr=MK_FP(segregs.ds,regs.x.si);
    }
}/*初始化DOS，取得INDOS标志和严重错误标志地址*/
int  DosBusy(void)
{
    if(indos_ptr && crit_err_ptr)
        return   (*indos_ptr||*crit_err_ptr);
    else
        return(-1);
}/*用于判断当前DOS是否处于繁忙状态*/