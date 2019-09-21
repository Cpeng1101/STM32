/*
 * @Author: cpeng 
 * @Date: 2019-09-20 17:09:58 
 * @Last Modified by: cpeng
 * @Last Modified time: 2019-09-20 18:06:44
 */
#include "w25q128.h"

void W25Q128_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    W25Q128_CS = 1; // 拉高片选

    SPI2_Init();

    SPI_SetBaudRate(SPI_BaudRatePrescaler_2); // 18MHz,高速模式
    W25Q128_ReadID();
}

// 0x90000000
u16 W25Q128_ReadID(void)
{
    u16 W25QXX = 0;
    W25Q128_CS = 0; // 拉低片选，选中芯片

    SPI_ReadWriteByte(0x90); // 发送读ID命令
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    W25QXX |= SPI_ReadWriteByte(0xff) << 8;
    W25QXX |= SPI_ReadWriteByte(0xff);

    W25Q128_CS = 1; // 拉高片选

    return W25QXX;
}

// 0x05 0x35 0x15
u8 W25Q128_ReadSR()
{
    u8 temp = 0;

    W25Q128_CS = 0;
    SPI_ReadWriteByte(W25Q128_ReadSR1_Cmd);
    temp = SPI_ReadWriteByte(0xff);
    W25Q128_CS = 1;

    return temp;
}

// 0x01 0x31 0x11
void W25Q128_WriteSR(u8 regn, u8 bitn) // 写状态寄存器
{
    W25Q128_CS = 0;
    SPI_ReadWriteByte(regn);
    SPI_ReadWriteByte(bitn);
    W25Q128_CS = 1;
}

void W25Q128_WriteEnable(void)
{
    W25Q128_CS = 0;
    SPI_ReadWriteByte(W25Q128_WriteEnable_Cmd);
    W25Q128_CS = 1;
}

void W25Q128_WriteDisable(void)
{
    W25Q128_CS = 0;
    SPI_ReadWriteByte(W25Q128_WriteDisable_Cmd);
    W25Q128_CS = 1;
}

void W25Q128_WaitBusy()
{
    while ((W25Q128_ReadSR() & 0x01) == 0x01)
    {
        /* code */
    }
}

/*
 * pBuffer： 数据存储区
 * ReadAddr：待读取数据的地址
 * ReadLen： 待读取数据的长度
*/
void W25Q128_ReadFlash(u8 *pBuffer, u32 ReadAddr, u16 ReadLen)
{
    W25Q128_CS = 0;
    SPI_ReadWriteByte(W25Q128_ReadFlash_Cmd);
    SPI_ReadWriteByte((u8)((ReadAddr) >> 16));
    SPI_ReadWriteByte((u8)((ReadAddr) >> 8));
    SPI_ReadWriteByte((u8)ReadAddr);
    for (u16 i = 0; i < ReadLen; i++)
    {
        pBuffer[i] = SPI_ReadWriteByte(0xff);
        //printf("pbuffer: %#x\r\n", pBuffer[i]);
    }
    W25Q128_CS = 1;
}

void W25Q128_WritePage_SPI(u8 *pBuffer, u32 WriteAddr, u16 WriteLen)
{
    W25Q128_WriteEnable();
    W25Q128_CS = 0;
    SPI_ReadWriteByte(W25Q128_WritePage_Cmd);
    SPI_ReadWriteByte((u8)((WriteAddr) >> 16));
    SPI_ReadWriteByte((u8)((WriteAddr) >> 8));
    SPI_ReadWriteByte((u8)WriteAddr);
    for (u16 i = 0; i < WriteLen; i++)
    {
        SPI_ReadWriteByte(pBuffer[i]);
    }
    W25Q128_CS = 1;
    W25Q128_WaitBusy();
}

/* function： 按页写FLASH
 * pBuffer：  数据存储区
 * WriteAddr：待写入数据的地址
 * WriteLen： 待写入数据的长度
 * Description：具有自动换页的功能
*/
void W25Q128_WritePage(u8 *pBuffer, u32 WriteAddr, u16 WriteLen)
{
    u16 pagereman = 256 - WriteAddr % 256;   // 页剩余字节

    if (WriteLen <= pagereman)   // 不需要换页
    {
        pagereman = WriteLen;
    }

    while (1)
    {
        W25Q128_WritePage_SPI(pBuffer, WriteAddr, pagereman);

        if (pagereman == WriteLen)
        {
            break;
        }
        else
        {
            pBuffer += pagereman;
            WriteAddr += pagereman;

            WriteLen -= pagereman;

            if (WriteLen > 256)
            {
                pagereman = 256;
            }
            else
            {
                pagereman = WriteLen;
            }
        }
    }
}



/*
 * function：擦拭一个扇区
 * SectorAddr：扇区地址
*/
void W25Q128_EraseSector(u32 SectorAddr)
{
    W25Q128_WriteEnable();
    W25Q128_WaitBusy();
    W25Q128_CS = 0;
    SPI_ReadWriteByte(W25Q128_EraseSector_Cmd);
    SPI_ReadWriteByte((u8)((SectorAddr) >> 16));
    SPI_ReadWriteByte((u8)((SectorAddr) >> 8));
    SPI_ReadWriteByte((u8)SectorAddr);
    W25Q128_CS = 1;
    W25Q128_WaitBusy();
}

/*
 * function：写Flash
 * pBuffer： 待写入数据
 * WriteAddr：待写入的地址
 * WriteLen： 数据长度
 * description：如果扇区剩余空间(secreman)小于要写入数据的长度，就会自动换扇区
*/
u8 W25Q128_BUFFER[4096];
void W25Q128_WriteFlash(u8 *pBuffer, u32 WriteAddr, u16 WriteLen)
{
    u16 i;
    u8 flags_erase = 0;
    u16 secops = WriteAddr / 4096; // 扇区地址
    u16 secoff = WriteAddr % 4096; // 扇区内偏移
    u16 secreman = 4096 - secoff;  // 扇区剩余空间

    if (WriteLen <= secreman)
    {
        secreman = WriteLen;   // 一个扇区就可以写入完成，不需要换扇区
    }
    

    while (1)
    {
        W25Q128_ReadFlash(W25Q128_BUFFER, secops*4096, 4096);  // 读整个扇区数据

        for (i = 0; i < WriteLen; i++)
        {
            if (W25Q128_BUFFER[secoff + i] != 0xff)  // 需要擦除
            {
                break;
            }
        }

        if (i < WriteLen)  // 需要擦除
        {
            W25Q128_EraseSector(secops*4096);  // 擦除整个扇区

            for(i = 0; i < WriteLen; i++)
            {
                W25Q128_BUFFER[secoff + i] = pBuffer[i]; // 准备数据
            }

            W25Q128_WritePage(W25Q128_BUFFER, secops*4096, 4096); // 以页的方式写入整个扇区
        }
        else   // 不需要擦除，直接向所在页写入数据
        {
            W25Q128_WriteFlash(pBuffer, WriteAddr, secreman);  // 写入数据
        }

        if (WriteLen == secreman)
        {
            break;   // 写入完成，不需要换扇区
        }
        else
        {
            secops++;    // 扇区地址加1
            secoff = 0;  // 指向下个扇区的首地址

            pBuffer += secreman;    // 指针偏移
            WriteAddr += secreman;  // 写地址偏移
            WriteLen -= secreman;   // 换扇区后写入字节
            if (WriteLen > 4096)
            {
                secreman = 4096;  // 还需要换扇区
            }
            else
            {
                secreman = WriteLen;  // 下个扇区足够
            }
        }
    }
}