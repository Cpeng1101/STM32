#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "spi.h"
#include "w25q128.h"
#include "stm32f10x_conf.h"

int main(void)
{
	delay_init();
	uart_init(115200);
	LED_Init();
 	W25Q128_Init();
	u8 string[] = {"hello"};
	u8 len = sizeof(string);
	u8 buffer[256];

	u16 FlashID = W25Q128_ReadID();
	printf("Flash ID: %#x\r\n", FlashID);

	W25Q128_WriteFlash(string, 0, len);
	W25Q128_ReadFlash(buffer, 0, len);
	printf("Read: %s\r\n", buffer);


	while (1)
	{
		LED0 = !LED0;
		delay_ms(300);
	}
}
