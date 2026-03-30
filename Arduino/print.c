#include <avr/io.h>
#include "print.h"

/* transmetre les donn? dans udr0 */
int USART_Transmit(char data, FILE *stream)
{
    if (data == '\n') {
	USART_Transmit('\r', NULL);
    }
	/* Wait for empty transmit buffer */
	while ( !(UCSR0A & (1<<UDRE0)) ) ;

	/* Start transmittion */
	UDR0 = data;
	return 0;
}

void USART_Init(unsigned int baudrate)
{
    /* Appliquer le baudrate a UBRR0 */
    UBRR0H = (unsigned char) (baudrate>>8);
    UBRR0L = (unsigned char) baudrate;
    
    /* RAZ du registre de controle A */
    UCSR0A = 0x00 ;

    /* Active USART receiver and transmitter */
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    
    /* Met la trame au format: 8 data 2stop */
    UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);

    /* redirige le flux UDR0 dans stdout */
    static FILE mystdout = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
    stdout = &mystdout;
}

