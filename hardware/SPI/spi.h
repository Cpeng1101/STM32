#ifndef __SPI_H__
#define __SPI_H__
#include "stm32f10x_conf.h"

void SPI2_Init(void);
u8 SPI_ReadWriteByte(u8 data);
// 设置SPI波特率
void SPI_SetBaudRate(SPI_BaudRatePrescaler);
#endif