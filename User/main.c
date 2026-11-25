#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "usart.h"

uint16_t RX;

int main(void)
{
	/*模块初始化*/
	OLED_Init();
	usart_init();

	/*上电发一次：A + 666*/
	send(0x41);
	printf("666");

	while (1)
	{
		if (GETF() == 1)
		{
			RX = dedao();
			OLED_ShowHexNum(3, 2, RX, 2);
		}
	}
}
