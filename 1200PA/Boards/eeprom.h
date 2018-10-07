/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EEPROM_H
#define __STM32_EEPROM_H

unsigned char I2C_EE_BufferRead(const unsigned char slave_addr,unsigned char *buff,unsigned suba,unsigned char len);
unsigned char I2C_EE_BufferWrite(const unsigned char slave_addr,const unsigned char *buff,unsigned suba,unsigned char len);

#endif