#include <stdio.h>
#include <stdbool.h>

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "print.h"

int count = 0;
int inc = 1;

void PWM(short int a)
{
    switch (a)
    {
	case 1: // PWM rotation pour servo moteur
	/* WGM20 = 0, WGM20 = 1, WGM22 = 1 -> Correct Phase PWM mode
	 * COM2B1 = 1 -> Change the TOP value of the PWM to get 50Hz require for the Servo motor
	 */
	    TCCR1A = (1 << COM1B1) | (1 << WGM10) | (1 << WGM11);
	    TCCR1B = (1 << WGM13) | (1 << CS11);
	    TCNT1 = 0x00;
	    OCR1A = 0x4E20;
	    OCR1B = 1000;
	    break;
	
	case 2: // Starts a timer for the "HC-SR04" echo response
	    TCCR2A = (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
	    TCCR2B = (1 << CS20); // Mode 5 Fast PWM 8bit
	    TCNT2 = 0; // Start to 0
	    OCR2A = 128; // 50% duty cycle
	    break;

	case 3: // Stop the timer
	    TCCR2A = 0;
	    TCCR2B = 0;
	    PORTB &= ~(1 << PORTB3);
	default:
	    break;
    }
}

void Port_init()
{
    DDRB &= ~(1 << DDB0); // Echo reponse from HC-SR04
    DDRB |= (1 << DDB5) | (1 << DDB3) | (1 << DDB2) | (1 << DDB1); // and PB1 is a timer PB5 is for the L led
    DDRD |= (1 << DDD3) | (1 << DDD7); // PD7 Trig for HC-SR04

    PCICR = (1 << PCIE0); // Set the jump into ISR when interruption occur
    PCIFR = (1 << PCIF0); // Re-initialise all interruption that occur
    PCMSK0 = (1 << PCINT0); // Active the interruption on DDB2
}

void demande_de_mesure()
{
    PORTD |= (1 << PORTD7);
    _delay_us(10);
    PORTD &= ~(1 << PORTD7);
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
    PWM(1);
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
	while (PINB & (1 << PINB3)) {
	    if (pass == false) timer++;
	    pass = true;
	}
    }
    PWM(3);

    distance = (float)timer*T_PWM2*v*1000.f;

    // Minith -> 1000 = 1ms, Mid -> 1500 = 1.5ms, Maxth -> 2000 = 2ms
    // Mini -> 5 = 0.5ms, Mid -> 1500 = 1.5ms, Max -> 2750 = 2.75ms
    
    int OCR1B_buf = OCR1B;
    // if (count++ < 5) ;
    // if (count++ < 1) ;
    // else {
    // 	count = 0;
	if (inc == 1 && OCR1B_buf < 2750) {
	    OCR1B_buf+=25;
	}
	else if (OCR1B_buf >= 2750) {
	    inc = 0;
	    OCR1B_buf-=25;
	}
	else if (inc == 0 && OCR1B_buf > 500) {
	    OCR1B_buf-=25;
	}
	else if (OCR1B_buf <= 500) {
	    inc = 1;
	    OCR1B_buf+=25;
	}
    // }
    // OCR1B_buf = 5;
    OCR1B = OCR1B_buf;
    printf("%04d,%.3f\n\r", OCR1B_buf-500, distance/1000.f);
}
