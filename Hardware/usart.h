#ifndef __USART_H
#define __USART_H
#include <stdio.h>

void usart_init(void);
void send(uint8_t byte);
uint16_t GETF(void);
uint16_t dedao(void);
void USART1_IRQHandler(void);
#endif
