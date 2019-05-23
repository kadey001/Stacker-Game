/*
 * StackerGame.c
 *
 * Created: 5/22/2019 3:19:12 PM
 * Author : Kelton Adey
 */ 

#include <avr/io.h>
#include "io.c"
#include "timer.h"
#include "queue.h"
#include "scheduler.h"
#include "pwm.c"
#include "shift_reg.c"

#define uc unsigned char


/*
B0 = SER
B1 = RCLK
B2 = SRCLK
B3 = SRCLR

C0 = Button 1
C1 = Button 2
C2 = Button 3
C3 = Button 4
*/

int main(void)
{
    DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTC = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	
	PWM_on();
	TimerSet(50);
	TimerOn();
	
	uc C0 = 0x00;
	uc C1 = 0x00;
	uc C2 = 0x00;
	uc C3 = 0x00;
	
	transmit_data(0xF0);
	
    while (1) 
    {
		C0 = ~PINC & 0x01;
		C1 = ~PINC & 0x02;
		C2 = ~PINC & 0x04;
		C3 = ~PINC & 0x08;
		
		if(C0) {
			transmit_data(0x01);
		}
		else if(C1) {
			clear_data();
			//transmit_data(0x10);
		}
	}
}