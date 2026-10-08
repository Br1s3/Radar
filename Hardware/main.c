#include <stdio.h>
#include <stdbool.h>

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "print.h"

#define T_PWM1 0.000016f
#define v 170.f
#define TIMER_MAX 2000

int rotation = 1;
unsigned int AngleImage = 0;
unsigned int timer;
float distance;


void PWM(short int a)
{
    switch (a)
    {
	case 1: // Here is the PWM to rotate the servo moteur
	/* WGM20 = 0, WGM20 = 1, WGM22 = 1 -> Correct Phase PWM mode
	 * COM2B1 = 1 -> Change the TOP with OCR1A value. To get 50Hz it require OCR1A = 0x4E20
	 * OCR1B = 1000 -> Set the servo motor to 0° by default
	 */
	    TCCR1A = (1 << COM1B1) | (1 << WGM10) | (1 << WGM11);
	    TCCR1B = (1 << WGM13) | (1 << CS11);
	    TCNT1 = 0x00;
	    OCR1A = 0x4E20;
	    OCR1B = 1000;
	    break;
	
	case 2: // Starts a timer for the "HC-SR04" echo response
	/* (1 << WGM20) | (1 << WGM21) -> Fast PWM 8bit
	 * (1 << CS20) -> No prescaler PWMfreq: 16MHz/256 = 62.5 kHz
	 * OCR2A = 128 -> 50% duty cycle
	 */
	    TCCR2A = (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
	    TCCR2B = (1 << CS20);
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
    DDRB &= ~(1 << DDB0);
    // Echo reponse from HC-SR04

    DDRB |= (1 << DDB5) | (1 << DDB3) | (1 << DDB2);
    /* DDB5 is for the L led
     * DDB3 is for OC2A to count HC-SR04 response
     * DDB2 is to set the servo motor's position
     */
     
    DDRD |= (1 << DDD7);
    /* DDD3 is for OC2B
     * DDD7 is to trigger the HC-SR04 module
     */

    PCICR = (1 << PCIE0); // Set the jump into ISR when interruption occur
    PCIFR = (1 << PCIF0); // Re-initialise all interruption that occur
    PCMSK0 = (1 << PCINT0); // Activate the interruption on PB0
}

void demande_de_mesure()
{
    PORTD |= (1 << PORTD7);
    _delay_us(10);
    PORTD &= ~(1 << PORTD7);
}


int main(void)
{
    USART_Init(0);
    Port_init();
    sei();
    PWM(1);
    while (1) {

	demande_de_mesure();
	// ISR Interruption
	for (int i = 0; i < 60 - (int)(T_PWM1*(float)timer*1000.f); i++)
	    _delay_ms(1);

	// Minith -> 1000 = 1ms, Mid -> 1500 = 1.5ms, Maxth -> 2000 = 2ms
	// Mini -> 5 = 0.5ms, Mid -> 1500 = 1.5ms, Max -> 2750 = 2.75ms
	
	AngleImage = OCR1B;
	if (rotation == 1 && AngleImage < 2750) {
	    AngleImage+=25;
	}
	else if (AngleImage >= 2750) {
	    rotation = 0;
	    AngleImage-=25;
	}
	else if (rotation == 0 && AngleImage > 500) {
	    AngleImage-=25;
	}
	else if (AngleImage <= 500) {
	    rotation = 1;
	    AngleImage+=25;
	}

	OCR1B = AngleImage;
	printf("%04d,%.3f\n\r", AngleImage-500, distance/1000.f);

	PORTB ^= (1 << PORTB5); // Makes the LED blink
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

    distance = (float)timer*T_PWM1*v*1000.f;
}
