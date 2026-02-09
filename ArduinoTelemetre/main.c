#include <stdio.h>
#include <stdbool.h>

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "print.h"

void PWM(short int a)
{
    switch (a)
    {
	// case 1: // PWM rotation pour servo moteur
	//     TCCR2A = (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
	//     TCCR2B = (7 << CS20);
	//     TCNT2 = 0x00;
	//     OCR2A = cycle;
	//     break;
	
	case 2: // Starts a timer for the "HC-SR04" echo response
	    TCCR1A = (1 << COM1A1) | (1 << WGM10); 
	    TCCR1B = (1 << WGM12) | (1 << CS10); // Mode 5 Fast PWM 8bit
	    TCNT1 = 0; // Start to 0
	    OCR1A = 128; // 50% duty cycle
	    break;

	case 3: // Stop the timer
	    TCCR1A = 0;
	    TCCR1B = 0;
	    PORTB &= ~(1 << PORTB1);
	default:
	    break;
    }
}

void Port_init()
{
    DDRB &= ~(1 << DDB0); // Echo reponse from HC-SR04
    DDRB |= (1 << DDB5) | (1 << DDB2) | (1 << DDB1); // PB2 Trig for HC-SR04 and PB1 is a timer PB5 is for the L led

    PCICR = (1 << PCIE0); // Set the jump into ISR when interruption occur
    PCIFR = (1 << PCIF0); // Re-initialise all interruption that occur
    PCMSK0 = (1 << PCINT0); // Active the interruption on DDB0
}

void demande_de_mesure()
{
    PORTB |= (1 << PORTB2);
    _delay_us(10);
    PORTB &= ~(1 << PORTB2);
}

#define T_PWM2 0.00001605f
#define v 187.f
#define TIMER_MAX 2000

unsigned int timer;
float distance;

int main(void)
{
    USART_Init(0);
    Port_init();
    sei();

    while (1) {

	demande_de_mesure();
	// ISR Interruption;
	for (int i = 0; i < 60 - (int)(T_PWM2*timer*1000.f); i++)
	    _delay_ms(1);

	PORTB ^= (1 << PORTB5);
    }
}

ISR (PCINT0_vect)
{
    if (!(PINB & (1 << PINB0))) return ;
    PWM(2);
    timer = 0;
    while (PINB & (1 << PINB0)) {
	bool pass = false;
	while (PINB & (1 << PINB1)) {
	    if (pass == false) timer++;
	    pass = true;
	}
    }
    PWM(3);

    distance = (float)timer*T_PWM2*v*1000.f;
    // if (timer < TIMER_MAX) {
	// printf("distance: %.3f\n\r", distance/1000.f);
	printf("%.3f\n\r", distance/1000.f);
	// prog_obstacle();
    // }
    // else
    //     printf("ERROR: Mesure HORS PORTEE\r\n");
}
