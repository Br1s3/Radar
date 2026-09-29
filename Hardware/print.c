#include <avr/io.h>
#include "print.h"

/* Transmit data in UDR0 register */
int USART_Transmit(char data, FILE *stream)
{
    if (data == '\n') {
	USART_Transmit('\r', NULL);
    }
    
    /* Wait for empty transmit buffer */
    while ( !(UCSR0A & (1<<UDRE0)) ) ;

    /* Start transmission */
    UDR0 = data;
    return 0;
}

void USART_Init(unsigned int baudrate)
{
    /* Apply the baudrate to UBRR0 register */
    UBRR0H = (unsigned char) (baudrate>>8);
    UBRR0L = (unsigned char) baudrate;
    
    /* Reset the A control register */
    UCSR0A = 0x00 ;

    /* Activate UART receiver and transmitter */
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    
    /* Formats the frame: 8 bytes with 2 stop bits */
    UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);

    /* Redirect the flow UDR0 in the STDOUT */
    static FILE mystdout = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
    stdout = &mystdout;
}

