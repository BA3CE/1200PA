#include "bsp.h"
#include "spi/spi.h"

#define SPI_WREN        0x06
#define SPI_WRDI        0x04
#define SPI_RDSR        0x05
#define SPI_WRSR        0x01
#define SPI_READ        0x03
#define SPI_WRITE       0x02

#define SR_WIP          0x00
#define SR_WEL          0x02
#define SR_BP0          0x04
#define SR_BP1          0x08

static void delay1mS()
{
  	int i=0,j=0;
	for(i=0;i<32;i++)
		for(j=0;j<16;j++);
}


void write_byte(INT8U adr, INT8U sz, const INT8U *buf)
{
  	unsigned int  i;

  	
	GPIO_ResetBits(GPIOA,EEPROM_nCS);
	delay1mS();
	SPI_Send_Byte(SPI_WREN);
	delay1mS();
	GPIO_SetBits(GPIOA,EEPROM_nCS);
	delay1mS();
	GPIO_ResetBits(GPIOA,EEPROM_nCS);
	delay1mS();
	SPI_Send_Byte(SPI_WRITE);
	SPI_Send_Byte(adr);
	for (i = 0; i < sz; i++) {
		SPI_Send_Byte(buf[i]);  
	}
	delay1mS();
	GPIO_SetBits(GPIOA,EEPROM_nCS);
  	
}
void read_byte(INT8U address,INT8U sz, INT8U *buf)
{
	unsigned int i;

  	GPIO_ResetBits(GPIOA,EEPROM_nCS);
	delay1mS();
	SPI_Send_Byte(SPI_READ);
  	SPI_Send_Byte(address);

  	for (i = 0; i < sz; i++) {
		*buf++ = SPI_Send_Byte(0x00);

  	}
  	delay1mS();
	GPIO_SetBits(GPIOA,EEPROM_nCS);
}

