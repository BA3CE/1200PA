/*
*********************************************************************************************************
*                                       GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include <stdio.h>

#include <string.h>
#include "stm32f10x_it.h"
#include "bsp.h"
#include "SPI/25AA02.H"
#include "const.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

/* ----------------- APPLICATION GLOBALS ------------------ */
static  INT16U         REF_POWER=0;
static  INT16U         FWD_POWER=0;
/* -------- uC/OS-II APPLICATION TASK STACKS -------------- */
static  OS_STK         App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  OS_STK         ADC_TaskStk[ADC_TASK_STK_SIZE];
static  OS_STK         BUTTON_TaskStk[BUTTON_TASK_STK_SIZE];
static  OS_STK         Display_TaskStk[Display_TASK_STK_SIZE];
/*
*********************************************************************************************************
*                                      GLOBAL VARIABLES
*********************************************************************************************************
*/
extern vu8 PORT1_Buffer_Tx[UART_MAX_LEN]; //AUX1
extern vu8 PORT1_Buffer_Rx[UART_MAX_LEN];
extern vu8 PORT2_Buffer_Tx[UART_MAX_LEN]; //AUX2
extern vu8 PORT2_Buffer_Rx[UART_MAX_LEN];
extern vu8 PORT3_Buffer_Tx[UART_MAX_LEN]; //AUX3
extern vu8 PORT3_Buffer_Rx[UART_MAX_LEN];
extern vu16 ADC_Buffer[8];

/* ---------- uC/OS-II APPLICATION EVENTS ----------------- */
OS_EVENT *UART2_Sem;

/*
*********************************************************************************************************
*                                      FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void            App_TaskStart               (void *p_arg);
void                    LCD_Write					(const INT8U *cmd,INT8U len); 
static  void            DisplayPower				(INT16U power);
static  void            DisplayTemp					(INT8U temp);
static  void            DisplayRef                  (INT8U ref);
static  void            ADC_TaskCreate              (void);
static  void            ADC_Task           	    	(void *p_arg);
static  void            BUTTON_TaskCreate           (void);
static  void            BUTTON_Task           	    (void *p_arg);
static  void            Display_TaskCreate          (void);
static  void            Display_Task           	    (void *p_arg);
static  INT16U          frequency_measuring         (void);
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This the main standard entry point.
*
* Note(s)     : none.
*********************************************************************************************************
*/

int  main (void)
{
#if (OS_TASK_NAME_SIZE > 5)
    CPU_INT08U  err;
#endif

    BSP_IntDisAll();                                            /* Disable all interrupts until we are ready to accept them */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"              */
	
    OSTaskCreateExt((void (*)(void *)) App_TaskStart,           /* Create the start task                                    */
                    (void           *) 0,
                    (OS_STK         *)&App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_START_PRIO,
                    (INT16U          ) APP_CFG_TASK_START_PRIO,
                    (OS_STK         *)&App_TaskStartStk[0],
                    (INT32U          ) APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_SIZE > 5)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)       */
}

/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Argument(s) : p_arg   is the argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static  void  App_TaskStart (void *p_arg)
{
    (void)p_arg;                                                /* See Note #1                                              */
	


    BSP_Init();                                                 /* Initialize BSP functions                                 */
    OS_CPU_SysTickInit();                                       /* Initialize the SysTick.                              */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                                   */
#endif

      
	//static INT8U buff=0x01;
	//static INT8U ret;

    UART2_Sem=OSSemCreate(1);
    
	ADC_TaskCreate();
	Display_TaskCreate();
	BUTTON_TaskCreate();
	INT8U buf=0x05;
	write_byte(0x00,0x01,&buf);
	static INT8U buff1;
	read_byte(0x00,0x01,&buff1);
	
	//buff=0x00;
	//ret=I2C_ReadByte(&buff,1,0x0001|0x0080,IIC_ADDR);
    //int i=0x10;
	
    GPIO_ResetBits(GPIOE,ANT2);
    GPIO_ResetBits(GPIOE,ANT3);
    GPIO_ResetBits(GPIOE,ANT4);
    GPIO_ResetBits(GPIOE,ANT1);
    
    GPIO_ResetBits(GPIOD,BPF2);
    GPIO_ResetBits(GPIOD,BPF3);
    GPIO_ResetBits(GPIOD,BPF4);
    GPIO_ResetBits(GPIOD,BPF5);
    GPIO_ResetBits(GPIOD,BPF6);
    GPIO_ResetBits(GPIOD,BPF7);
    GPIO_SetBits(GPIOD,BPF1);    
    GPIO_ResetBits(GPIOB,OFF);
    
    LCD_Write(ANTSlect[1],sizeof(ANTSlect[0]));
    LCD_Write(ANTSlect[2],sizeof(ANTSlect[0]));
    LCD_Write(ANTSlect[4],sizeof(ANTSlect[0]));
    LCD_Write(ANTSlect[6],sizeof(ANTSlect[0]));
    

    
    
    LCD_Write(Band_Select[0],Band_Select_LEN[0]);

    //DisplayRef(250);
	int temp=0;
	//int ant=0;
    while (DEF_TRUE){                                          /* Task body, always written as an infinite loop.            */   
#if 1
	    //DisplayPower(temp);
	  //DisplayTemp(temp);
		if(temp<1200)
		  	temp+=22;
		else
		  	temp=0;
        //REF_POWER=temp;
#endif
		frequency_measuring();
        OSTimeDlyHMSM(0, 0, 0, 100);

    }
}
void LCD_Write(const INT8U *cmd,INT8U len)
{
    INT16U TxCounter=0;
    INT8U err;
    OSSemPend(UART2_Sem,0,&err);
    
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif	 
    OS_ENTER_CRITICAL();
    
  	memcpy((void *)PORT2_Buffer_Tx,cmd,len);

    do{
        USART_SendData(USART2, PORT2_Buffer_Tx[TxCounter++]);
        
        
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }while(TxCounter<len);
    
	OSTimeDlyHMSM(0, 0, 0, 100);
    
    OS_EXIT_CRITICAL();
    OSSemPost(UART2_Sem);
}
void DisplayPower(INT16U power)
{
  	INT8U power_num[]={0xEE,0xB1,0x10,0x00,0x00,0x01,0x2B,0x20,0x20,0x20,0x20,0x57,0xFF,0xFC,0xFF,0xFF};
	power_num[7]=power/1000+0x30;
	power_num[8]=power%1000/100+0x30;
	power_num[9]=power%100/10+0x30;
	power_num[10]=power%10+0x30;
	if(power<10){
	  	power_num[7]=0x20;
		power_num[8]=0x20;
		power_num[9]=0x20;
	}
	else if(power<100){
	  	power_num[7]=0x20;
		power_num[8]=0x20;
	}
	else if(power<1000){
	  	power_num[7]=0x20;
	}

  	LCD_Write(power_num,sizeof(power_num));
	if(power/22<21){
	  	LCD_Write(Power_Bar_GR1[power/22],sizeof(Power_Bar_GR1[0]));
		LCD_Write(Power_Bar_GR2[0],sizeof(Power_Bar_GR2[0]));
		LCD_Write(Power_Bar_GR3[0],sizeof(Power_Bar_GR3[0]));
	}
	else if(power/22<41){
	  	LCD_Write(Power_Bar_GR1[20],sizeof(Power_Bar_GR1[0]));
		LCD_Write(Power_Bar_GR2[power/22-20],sizeof(Power_Bar_GR2[0]));
		LCD_Write(Power_Bar_GR3[0],sizeof(Power_Bar_GR3[0]));
	}
	else{
	  	LCD_Write(Power_Bar_GR1[20],sizeof(Power_Bar_GR1[0]));
		LCD_Write(Power_Bar_GR2[20],sizeof(Power_Bar_GR2[0]));
		LCD_Write(Power_Bar_GR3[power/22-40],sizeof(Power_Bar_GR3[0]));
	}
	
}
void DisplayTemp(INT8U temp)
{
  	INT8U temp_num[]={0xEE,0xB1,0x10,0x00,0x00,0x01,0x5E,0x20,0x20,0x20,0xFF,0xFC,0xFF,0xFF};
	temp_num[7]=temp/100+0x30;
	temp_num[8]=temp%100/10+0x30;
	temp_num[9]=temp%10+0x30;
	if(temp<10){
	  	temp_num[7]=0x20;
		temp_num[8]=0x20;
	}
	else if(temp<100)
	  	temp_num[7]=0x20;

  	LCD_Write(temp_num,sizeof(temp_num));
	
	if(temp/8<=19){
		LCD_Write(TEMP_Bar_GR[temp/4],sizeof(TEMP_Bar_GR[0]));
		LCD_Write(TEMP_Bar_OG[0],sizeof(TEMP_Bar_OG[0]));
	}
	else{
	  	LCD_Write(TEMP_Bar_GR[19],sizeof(TEMP_Bar_GR[0]));
		LCD_Write(TEMP_Bar_OG[temp/4-19],sizeof(TEMP_Bar_OG[0]));
		
	}	
}
void DisplayRef(INT8U ref)
{
  	INT8U ref_num[]={0xEE,0xB1,0x10,0x00,0x00,0x01,0xC2,0x20,0x20,0x20,0x57,0xFF,0xFC,0xFF,0xFF};
	ref_num[7]=ref/100+0x30;
	ref_num[8]=ref%100/10+0x30;
	ref_num[9]=ref%10+0x30;
	if(ref<10){
	  	ref_num[7]=0x20;
		ref_num[8]=0x20;
	}
	else if(ref<100)
	  	ref_num[7]=0x20;

  	LCD_Write(ref_num,sizeof(ref_num));
	
	if(ref/4<=19){
		LCD_Write(REF_Bar_GR[ref/4],sizeof(REF_Bar_GR[0]));
		OSTimeDlyHMSM(0, 0, 0, 150);
		LCD_Write(REF_Bar_OG[0],sizeof(REF_Bar_OG[0]));
	}
	else{
	  	LCD_Write(REF_Bar_GR[19],sizeof(REF_Bar_GR[0]));
        //LCD_Write(REF_Bar_GR[0],sizeof(REF_Bar_GR[0]));
	  	OSTimeDlyHMSM(0, 0, 0, 150);
		LCD_Write(REF_Bar_OG[ref/4-56],sizeof(REF_Bar_OG[0]));
	}	
}
static void ADC_TaskCreate(void)
{
#if (OS_TASK_NAME_SIZE > 8)        
    CPU_INT08U      err;
#endif
    OSTaskCreateExt((void (*)(void *)) ADC_Task,
                    (void           *) 0,
                    (OS_STK         *)&ADC_TaskStk[ADC_TASK_STK_SIZE - 1],
                    (INT8U           ) ADC_TASK_PRIO,
                    (INT16U          ) ADC_TASK_PRIO,
                    (OS_STK         *)&ADC_TaskStk[0],
                    (INT32U          ) ADC_TASK_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_SIZE > 15)
    OSTaskNameSet(ADC_TASK_PRIO, "ADC_Task", &err);
#endif
	
}
static void ADC_Task(void *p_arg)
{
	(void)p_arg;
	INT32U tmp[8];
    INT8U i,j;
    INT16U Voltage[8];
	
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif		
	while(1){
		for(i=0;i<8;i++)
	  		tmp[i]=0;
		OS_ENTER_CRITICAL();
	    for(i=0;i<32;i++){
    	  	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
      		while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);
       		DMA_ClearFlag(DMA1_FLAG_TC1);
			for(j=0;j<8;j++)
				tmp[j]+=ADC_Buffer[j];
	    }
		OS_EXIT_CRITICAL();
        
        for(j=0;j<8;j++)
			Voltage[j]=tmp[j]/32;		

        for(j=0;j<8;j++){
            Voltage[j]=(INT16U)((double)Voltage[j]*(2495.0/4096.0));

        }
        REF_POWER=(double)Voltage[3]/2.4;
        FWD_POWER=(double)Voltage[4]/11.5;
        

	  	OSTimeDlyHMSM(0, 0, 0, 20);
	}
}
static void Display_TaskCreate(void)
{
#if (OS_TASK_NAME_SIZE > 8)        
    CPU_INT08U      err;
#endif
    OSTaskCreateExt((void (*)(void *)) Display_Task,
                    (void           *) 0,
                    (OS_STK         *)&Display_TaskStk[Display_TASK_STK_SIZE - 1],
                    (INT8U           ) Display_TASK_PRIO,
                    (INT16U          ) Display_TASK_PRIO,
                    (OS_STK         *)&Display_TaskStk[0],
                    (INT32U          ) Display_TASK_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_SIZE > 15)
    OSTaskNameSet(Display_TASK_PRIO, "DisplayC_Task", &err);
#endif
	
}
static void Display_Task(void *p_arg)
{
	(void)p_arg;
	//INT32U tmp[8];
    //INT8U i,j;
    //INT16U Voltage[8];
	
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif		
	while(1){
        OS_ENTER_CRITICAL();
	    DisplayPower(REF_POWER);
	  	OSTimeDlyHMSM(0, 0, 0, 20);
        DisplayRef(FWD_POWER);
        OS_EXIT_CRITICAL();
	}
}

static void BUTTON_TaskCreate(void)
{
#if (OS_TASK_NAME_SIZE > 8)        
    CPU_INT08U      err;
#endif
    OSTaskCreateExt((void (*)(void *)) BUTTON_Task,
                    (void           *) 0,
                    (OS_STK         *)&BUTTON_TaskStk[BUTTON_TASK_STK_SIZE - 1],
                    (INT8U           ) BUTTON_TASK_PRIO,
                    (INT16U          ) BUTTON_TASK_PRIO,
                    (OS_STK         *)&BUTTON_TaskStk[0],
                    (INT32U          ) BUTTON_TASK_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_SIZE > 15)
    OSTaskNameSet(BUTTON_TASK_PRIO, "BUTTON_Task", &err);
#endif
	
}
static void BUTTON_Task(void *p_arg)
{
	(void)p_arg;
	INT8U ANT_Select=1;
    INT8U BAND_Select=1;
    INT8U SYBY_OPR=1;
    INT8U POWER=0;
    INT8U FAN=0;
	while(1){
        if(GPIO_ReadInputDataBit(GPIOD,KEY1)==0){
            OSTimeDlyHMSM(0, 0, 0, 10);
            if(GPIO_ReadInputDataBit(GPIOD,KEY1)==0){
                if(SYBY_OPR==1){
                    GPIO_SetBits(GPIOB,OFF);
                    LCD_Write(OPR,sizeof(OPR));
                    LCD_Write(STB_ICON[1],sizeof(STB_ICON[0]));
                    OSTimeDlyHMSM(0, 0, 2, 0);
                    GPIO_SetBits(GPIOC,Power_ON);
                    OSTimeDlyHMSM(0, 0, 1, 0);
                    GPIO_ResetBits(GPIOC,Power_ON);
                    LCD_Write(STB_ICON[0],sizeof(STB_ICON[0]));
                    SYBY_OPR=0;
                }
                else{
                    LCD_Write(STBY,sizeof(STBY));
                    LCD_Write(STB_ICON[2],sizeof(STB_ICON[1]));
                    GPIO_ResetBits(GPIOB,OFF);
                    SYBY_OPR=1;
                }
                while(GPIO_ReadInputDataBit(GPIOD,KEY1)==0);
            }  	
        }
        else if(GPIO_ReadInputDataBit(GPIOD,KEY2)==0){
            OSTimeDlyHMSM(0, 0, 0, 10);
            if(GPIO_ReadInputDataBit(GPIOD,KEY2)==0){
                BAND_Select++;
                if(BAND_Select>10)
			  	    BAND_Select=1;
                switch(BAND_Select){
                    case 1:
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF1);
                        GPIO_SetBits(GPIOB,_2dB);
                        GPIO_ResetBits(GPIOB,_10dB);
                        LCD_Write(Band_Select[0],Band_Select_LEN[0]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 2:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOB,_2dB);
                        GPIO_SetBits(GPIOB,_10dB);
                        LCD_Write(Band_Select[1],Band_Select_LEN[1]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 3:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF3);
                        
                        LCD_Write(Band_Select[2],Band_Select_LEN[2]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 4:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF4);
                        
                        LCD_Write(Band_Select[3],Band_Select_LEN[4]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 5:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF4);
                        
                        LCD_Write(Band_Select[4],Band_Select_LEN[5]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 6:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF5);
                        
                        LCD_Write(Band_Select[5],Band_Select_LEN[6]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 7:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF5);
                        
                        LCD_Write(Band_Select[6],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;   
                    case 8:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF6);
                        
                        LCD_Write(Band_Select[7],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;    
                    case 9:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF6);
                        
                        LCD_Write(Band_Select[8],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;  
                    case 10:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_SetBits(GPIOD,BPF7);
                        
                        LCD_Write(Band_Select[9],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;                           
                }                
                while(GPIO_ReadInputDataBit(GPIOD,KEY2)==0);
            }  	
        }      
        else if(GPIO_ReadInputDataBit(GPIOD,KEY3)==0){
            OSTimeDlyHMSM(0, 0, 0, 10);
            if(GPIO_ReadInputDataBit(GPIOD,KEY3)==0){
                if(BAND_Select<=1)
			  	    BAND_Select=11;
                BAND_Select--;
                switch(BAND_Select){
                    case 1:
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOB,_2dB);
                        GPIO_SetBits(GPIOB,_10dB);                        
                        LCD_Write(Band_Select[0],Band_Select_LEN[0]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 2:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF2);
                        GPIO_SetBits(GPIOB,_2dB);
                        GPIO_ResetBits(GPIOB,_10dB);
                        LCD_Write(Band_Select[1],Band_Select_LEN[1]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 3:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF3);
                        
                        LCD_Write(Band_Select[2],Band_Select_LEN[2]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 4:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF4);
                        
                        LCD_Write(Band_Select[3],Band_Select_LEN[4]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 5:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF4);
                        
                        LCD_Write(Band_Select[4],Band_Select_LEN[5]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 6:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF5);
                        
                        LCD_Write(Band_Select[5],Band_Select_LEN[6]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 7:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF5);
                        
                        LCD_Write(Band_Select[6],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;   
                    case 8:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF6);
                        
                        LCD_Write(Band_Select[7],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;    
                    case 9:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF7);
                        GPIO_SetBits(GPIOD,BPF6);
                        
                        LCD_Write(Band_Select[8],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;  
                    case 10:
                        GPIO_ResetBits(GPIOD,BPF1);
                        GPIO_ResetBits(GPIOD,BPF2);
                        GPIO_ResetBits(GPIOD,BPF3);
                        GPIO_ResetBits(GPIOD,BPF4);
                        GPIO_ResetBits(GPIOD,BPF5);
                        GPIO_ResetBits(GPIOD,BPF6);
                        GPIO_SetBits(GPIOD,BPF7);
                        
                        LCD_Write(Band_Select[9],Band_Select_LEN[7]);
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;                           
                }                
                while(GPIO_ReadInputDataBit(GPIOD,KEY3)==0);
            }  	
        }            
        else if(GPIO_ReadInputDataBit(GPIOD,KEY4)==0){
            OSTimeDlyHMSM(0, 0, 0, 10);
            if(GPIO_ReadInputDataBit(GPIOD,KEY4)==0){
                ANT_Select++;
			    if(ANT_Select>4)
			  	    ANT_Select=1;
                switch(ANT_Select){
                    case 1:
                        LCD_Write(ANTSlect[2],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[4],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[6],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[1],sizeof(ANTSlect[0]));
                        GPIO_ResetBits(GPIOE,ANT2);
                        GPIO_ResetBits(GPIOE,ANT3);
                        GPIO_ResetBits(GPIOE,ANT4);
                        GPIO_ResetBits(GPIOE,ANT1);
                        
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;
                    case 2:
                        LCD_Write(ANTSlect[0],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[4],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[6],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[3],sizeof(ANTSlect[0]));
                        GPIO_SetBits(GPIOE,ANT1);
                        GPIO_ResetBits(GPIOE,ANT3);
                        GPIO_ResetBits(GPIOE,ANT4);
                        GPIO_SetBits(GPIOE,ANT2);
                         
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;   
                    case 3:
                        LCD_Write(ANTSlect[0],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[2],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[6],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[5],sizeof(ANTSlect[0]));
                        GPIO_SetBits(GPIOE,ANT1);
                        GPIO_ResetBits(GPIOE,ANT2);
                        GPIO_ResetBits(GPIOE,ANT4);
                        GPIO_SetBits(GPIOE,ANT3);  
                        
                        LCD_Write(BUZZ,sizeof(BUZZ));
                                            
                        break; 
                    case 4:
                        LCD_Write(ANTSlect[0],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[2],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[4],sizeof(ANTSlect[0]));
                        LCD_Write(ANTSlect[7],sizeof(ANTSlect[0]));
                        GPIO_SetBits(GPIOE,ANT1);
                        GPIO_ResetBits(GPIOE,ANT2);
                        GPIO_ResetBits(GPIOE,ANT3);
                        GPIO_SetBits(GPIOE,ANT4);
                        
                        LCD_Write(BUZZ,sizeof(BUZZ));
                        break;                        
                }
                while(GPIO_ReadInputDataBit(GPIOD,KEY4)==0);
            }  	
        }        
        else if(GPIO_ReadInputDataBit(GPIOD,KEY5)==0){
            OSTimeDlyHMSM(0, 0, 0, 10);
            if(GPIO_ReadInputDataBit(GPIOD,KEY5)==0){
                if(FAN==0){
                    GPIO_SetBits(GPIOC,FAN2);
                    GPIO_ResetBits(GPIOC,FAN1);
                    FAN=1;
                }
                else{
                    GPIO_SetBits(GPIOC,FAN1);
                    GPIO_ResetBits(GPIOC,FAN2);
                    FAN=0;
                }
                while(GPIO_ReadInputDataBit(GPIOD,KEY5)==0);
            }  	
        }          
        else if(GPIO_ReadInputDataBit(GPIOD,KEY6)==0){
            OSTimeDlyHMSM(0, 0, 0, 10);
            if(GPIO_ReadInputDataBit(GPIOD,KEY6)==0){

                while(GPIO_ReadInputDataBit(GPIOD,KEY6)==0);
            }  	
        }  
        else
            OSTimeDlyHMSM(0, 0, 0, 10);
        
        
        if(GPIO_ReadInputDataBit(GPIOB,Power)==0){
            OSTimeDlyHMSM(0, 0, 2, 0);
            if(GPIO_ReadInputDataBit(GPIOB,Power)==0){
                
                if(POWER==0){
                    GPIO_SetBits(GPIOA,LED);
                    GPIO_SetBits(GPIOB,OFF);
                    POWER=1;
                }
                else if(POWER==1){
                    GPIO_ResetBits(GPIOA,LED);
                    GPIO_ResetBits(GPIOB,OFF);
                    POWER=0;
                }
            }
        }
        
	}

}
#define FREQ_ADJ    0
static INT16U frequency_measuring(void)
{
    INT16U counter;

    if(TIM_GetITStatus(TIM3,TIM_IT_CC1)){
        counter = 72000000 / TIM_GetCapture1(TIM3) / 10 + FREQ_ADJ;  //KHz
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    }
    else
        counter=0;
#if 0
    if(counter>9999){
        Display[0][0]=counter/10000%10 +0x30;
        Display[0][1]=counter/1000%10 +0x30;
        Display[0][2]='.';
        Display[0][3]=counter/100%10 +0x30;
        Display[0][4]=counter/10%10 +0x30;
    }
    else{
        Display[0][0]=' ';
        Display[0][1]=counter/1000%10 +0x30;
        Display[0][2]='.';
        Display[0][3]=counter/100%10 +0x30;
        Display[0][4]=counter/10%10 +0x30;
    }
    Display[0][5]='M';
    Display[0][6]='H';
    Display[0][7]='z';
#endif
    return counter;
}