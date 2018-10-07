/* Includes ------------------------------------------------------------------*/
#include <intrinsics.h>
#include "bsp.h"
/* Private value -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_DELAY()	__no_operation();	\
					__no_operation();	\
					__no_operation(); 	\
					__no_operation();	\
                    __no_operation();	\
					__no_operation();	\
					__no_operation(); 	\
					__no_operation();	\
					__no_operation();	\
					__no_operation(); 	\
					__no_operation();	\
                    __no_operation();	\
					__no_operation();	\
					__no_operation(); 	\
					__no_operation();	\
					__no_operation()

u8 ack;

/*******************************************************************
                     起动总线函数
函数原型: void  Start_I2c();
功能:       启动I2C总线,即发送I2C起始条件.

********************************************************************/
void Start_I2c(void)
{
  	GPIO_SetBits(GPIOB, SDA);		/*发送起始条件的数据信号*/
 	I2C_DELAY();
  	GPIO_SetBits(GPIOB, SCL);
	I2C_DELAY();    				/*起始条件建立时间大于4.7us,延时*/
	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
	GPIO_ResetBits(GPIOB, SDA); 	/*发送起始信号*/
  	I2C_DELAY();		   			/* 起始条件锁定时间大于4μs*/
  	I2C_DELAY();
  	I2C_DELAY();
  	I2C_DELAY();
  	I2C_DELAY();
  	GPIO_ResetBits(GPIOB, SCL);   	/*钳住I2C总线，准备发送或接收数据 */
  	I2C_DELAY();
  	I2C_DELAY();
  	I2C_DELAY();
	I2C_DELAY();
  	I2C_DELAY();
}
/*******************************************************************
                      结束总线函数
函数原型: void  Stop_I2c();
功能:       结束I2C总线,即发送I2C结束条件.

********************************************************************/
void Stop_I2c(void)
{
	GPIO_ResetBits(GPIOB, SDA);   /*发送结束条件的数据信号*/
  	I2C_DELAY();				  /*发送结束条件的时钟信号*/
  	GPIO_SetBits(GPIOB, SCL);;    /*结束条件建立时间大于4μs*/
  	I2C_DELAY();
  	I2C_DELAY();
  	I2C_DELAY();
	I2C_DELAY();
  	I2C_DELAY();
  	GPIO_SetBits(GPIOB, SDA);	 /*发送I2C总线结束信号*/
  	I2C_DELAY();
  	I2C_DELAY();
  	I2C_DELAY();
	I2C_DELAY();
  	I2C_DELAY();
}
/*******************************************************************
                 字节数据传送函数
函数原型: void  SendByte(uchar c);
功能:  将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,并对
     此状态位进行操作.(不应答或非应答都使ack=0 假)
     发送数据正常，ack=1; ack=0表示被控器无应答或损坏。
********************************************************************/
void SendByte(u8 c)
{
    u8 BitCnt;

    for(BitCnt=0;BitCnt<8;BitCnt++){  /*要传送的数据长度为8位*/
        if((c<<BitCnt)&0x80)	 /*判断发送位*/
	      GPIO_SetBits(GPIOB, SDA);
       	else
              GPIO_ResetBits(GPIOB, SDA);
     	I2C_DELAY();
     	GPIO_SetBits(GPIOB, SCL);               /*置时钟线为高，通知被控器开始接收数据位*/
      	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
     	GPIO_ResetBits(GPIOB, SCL);
    }
    I2C_DELAY();
    I2C_DELAY();
    I2C_DELAY();
    I2C_DELAY();
    I2C_DELAY();
    GPIO_SetBits(GPIOB, SDA);               /*8位发送完后释放数据线，准备接收应答位*/
    I2C_DELAY();
    I2C_DELAY();
    I2C_DELAY();
    I2C_DELAY();
    I2C_DELAY();
    GPIO_SetBits(GPIOB, SCL);
    I2C_DELAY();
	
    if(GPIO_ReadInputDataBit(GPIOB, SDA)==1)
        ack=0;
    else
	ack=1;        /*判断是否接收到应答信号*/
    GPIO_ResetBits(GPIOB, SCL);
    I2C_DELAY();
    I2C_DELAY();
}
/*******************************************************************
                 字节数据传送函数
函数原型: uchar  RcvByte();
功能:  用来接收从器件传来的数据,并判断总线错误(不发应答信号)，
     发完后请用应答函数。
********************************************************************/	
u8 RcvByte(void)
{
	u8 retc=0;
  	u8 BitCnt;

  	GPIO_SetBits(GPIOB, SDA);               /*置数据线为输入方式*/
  	for(BitCnt=0;BitCnt<8;BitCnt++){
		I2C_DELAY();
        GPIO_ResetBits(GPIOB, SCL);        /*置时钟线为低，准备接收数据位*/
        I2C_DELAY();
		I2C_DELAY();
		I2C_DELAY();
		I2C_DELAY();
		I2C_DELAY();
        GPIO_SetBits(GPIOB, SCL);        /*置时钟线为高使数据线上数据有效*/
        I2C_DELAY();
		I2C_DELAY();
        retc=retc<<1;
        if(GPIO_ReadInputDataBit(GPIOB, SDA)==1)
			retc=retc+1; /*读数据位,接收的数据位放入retc中 */
        I2C_DELAY();
		I2C_DELAY();
    }
  	GPIO_ResetBits(GPIOB, SCL);
  	I2C_DELAY();
	I2C_DELAY();
  	return(retc);
}

/********************************************************************
                     应答子函数
原型:  void Ack_I2c(bit a);

功能:主控器进行应答信号,(可以是应答或非应答信号)
********************************************************************/
void Ack_I2c(u8 a)
{

	if(a==0)
		GPIO_ResetBits(GPIOB, SDA);     /*在此发出应答或非应答信号 */
    else
	  	GPIO_SetBits(GPIOB, SDA);
  	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
  	GPIO_SetBits(GPIOB, SCL);
    I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
	I2C_DELAY();
 	GPIO_ResetBits(GPIOB, SCL);         /*清时钟线，钳住I2C总线以便继续接收*/
    I2C_DELAY();
	I2C_DELAY();
}
u8 I2C_EE_BufferWrite(const u8 slave_addr,u8 *buff,u16 suba,u8 len)
{
    u8 i;
    u8 suba_h;
    u8 suba_l;
    suba_h=suba>>8;
    suba_l=suba;
    Start_I2c();               /*启动总线*/
    SendByte(slave_addr);            /*发送器件地址*/
    if(ack==0)
        return	0;
    SendByte(suba_h);            /*发送器件子地址*/
    if(ack==0)
        return	0;
    SendByte(suba_l);            /*发送器件子地址*/
    if(ack==0)
	return	0;
    for(i=0;i<len;i++){
    	SendByte(*buff);          /*发送数据*/
       	if(ack==0)
			return 0;
     	buff++;
    }
    Stop_I2c();                 /*结束总线*/
    return	1;
}
u8 I2C_EE_BufferRead(const u8 slave_addr,u8 *buff,unsigned suba,u8 len)
{
	u8 i;
   	u8 suba_h;
   	u8 suba_l;
   	suba_h=suba>>8;
   	suba_l=suba;
   	Start_I2c();                 	/*启动总线*/
   	SendByte(slave_addr);	/*发送器件地址*/
    if(ack==0)
		return 0;
   	SendByte(suba_h);            /*发送器件子地址*/
    if(ack==0)
		return 0;
   	SendByte(suba_l);            /*发送器件子地址*/
   	if(ack==0)
		return 0;

   	Start_I2c();
   	SendByte(slave_addr+1);
    if(ack==0)
		return	0;

   	for(i=0;i<len-1;i++){
    	*buff=RcvByte();               /*发送数据*/
      	Ack_I2c(0);                	/*发送就答位*/
     	buff++;
    }
   	*buff=RcvByte();
    Ack_I2c(1);                 	/*发送非应位*/
 	Stop_I2c();                    	/*结束总线*/
  	return 1;
}


