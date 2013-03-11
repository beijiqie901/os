#include<dos.h>

#define GET_INDOS 0X34
#define GET_GRIT_ERR 0X5D06

char far *indos_ptr = 0 ;       
char far *crit_err_ptr = 0 ;

void   InitDos(void);                /*��ʼ��DOS,���InDos��־�����ش����־criterr�ĵ�ַ*/
int    DosBusy(void);                /*�����жϵ�ǰDOS�Ƿ��ڷ�æ״̬*/

void   InitDos()         
{
    union   REGS   regs;
    struct   SREGS   segregs;
    regs.h.ah=GET_INDOS;         /*ʹ��34H��ϵͳ���ܵ���*/
    intdosx(&regs,&regs,&segregs);
    indos_ptr=MK_FP(segregs.es,regs.x.bx);
    if(_osmajor<3)
        crit_err_ptr=indos_ptr+1;       /*���ش�����INDOS��һ�ֽڴ�*/
    else   if(_osmajor==3&&_osmajor==0)
        crit_err_ptr=indos_ptr-1;       /*���ش�����INDOSǰһ�ֽڴ�*/
    else
    {
        regs.x.ax=GET_GRIT_ERR;
        intdosx(&regs,&regs,&segregs);
        crit_err_ptr=MK_FP(segregs.ds,regs.x.si);
    }
}/*��ʼ��DOS��ȡ��INDOS��־�����ش����־��ַ*/
int  DosBusy(void)
{
    if(indos_ptr && crit_err_ptr)
        return   (*indos_ptr||*crit_err_ptr);
    else
        return(-1);
}/*�����жϵ�ǰDOS�Ƿ��ڷ�æ״̬*/