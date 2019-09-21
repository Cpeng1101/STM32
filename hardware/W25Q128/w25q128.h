/*
 * @Author: cpeng 
 * @Date: 2019-09-20 17:09:51 
 * @Last Modified by: cpeng
 * @Last Modified time: 2019-09-20 17:54:31
 */
#ifndef __W25Q128_H__
#define __W25Q128_H__
#include "stm32f10x_conf.h"
#include "sys.h"
#include "spi.h"

#define W25Q128_ReadID_Cmd 0x90

#define W25Q128_ReadSR1_Cmd 0x05
#define W25Q128_ReadSR2_Cmd 0x35
#define W25Q128_ReadSR3_Cmd 0x15

#define W25Q128_WriteSR1_Cmd 0x01
#define W25Q128_WriteSR1_Cmd 0x31
#define W25Q128_WriteSR1_Cmd 0x11

#define W25Q128_WriteEnable_Cmd 0x06
#define W25Q128_WriteDisable_Cmd 0x04

#define W25Q128_ReadFlash_Cmd 0x03
#define W25Q128_WritePage_Cmd 0x02

#define W25Q128_EraseSector_Cmd 0x20

#define W25Q128_CS PBout(12)

#define W25Q_TYPE 0xef17

void W25Q128_Init(void);


u16 W25Q128_ReadID(void); // 读设备ID
u8 W25Q128_ReadSR(void);  // 读状态寄存器
void W25Q128_WriteSR(u8 regn, u8 bitn); // 写状态寄存器
void W25Q128_WriteEnable(void); // 写使能
void W25Q128_WriteDisable(void); // 写保护
void W25Q128_WaitBusy(void); // 等待空闲
void W25Q128_ReadFlash(u8 *pBuffer, u32 ReadAddr, u16 ReadLen); // 读FLASH数据
void W25Q128_WritePage(u8 *pBuffer, u32 WriteAddr, u16 WriteLen);
void W25Q128_EraseSector(u32 EraseAddr);
void W25Q128_WriteFlash(u8 *pBuffer, u32 WriteAddr, u16 WriteLen);
void W25Q128_WritePage_SPI(u8 *pBuffer, u32 WriteAddr, u16 WriteLen);
#endif