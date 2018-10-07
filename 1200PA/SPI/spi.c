#include "bsp.h"

INT8U SPI_Send_Byte(INT8U data)
{
    // 检查并等待TX缓冲区为空
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    
    // 缓冲区为空后向缓冲区写入要发送的字节数据
    SPI_I2S_SendData(SPI1, data);	

    // 检查并等待RX缓冲区为非空
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    // 数据发送完毕，从RX缓冲区接收flash返回的数据 	
    return SPI_I2S_ReceiveData(SPI1); 
}