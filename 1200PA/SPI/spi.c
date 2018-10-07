#include "bsp.h"

INT8U SPI_Send_Byte(INT8U data)
{
    // ��鲢�ȴ�TX������Ϊ��
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    
    // ������Ϊ�պ��򻺳���д��Ҫ���͵��ֽ�����
    SPI_I2S_SendData(SPI1, data);	

    // ��鲢�ȴ�RX������Ϊ�ǿ�
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    // ���ݷ�����ϣ���RX����������flash���ص����� 	
    return SPI_I2S_ReceiveData(SPI1); 
}