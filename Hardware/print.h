#ifndef PRINT_H_INCLUDED
#define PRINT_H_INCLUDED
#include <stdio.h>


void USART_Init(unsigned int baudrate);
int USART_Transmit(char data, FILE *stream);






#endif //PRINT_H_INCLUDED
