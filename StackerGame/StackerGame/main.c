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

#define uc unsigned char 

#define SER 0x01
#define RCLK 0x02
#define SRCLK 0x04
#define SRCLR 0x08

void transmit_data(uc data) {
	//uc tempData = 0x00;
	// for each bit of data
	for(uc i = 0; i < 8; ++i){
		// Set SRCLR to 1 allowing data to be set
		// Also clear SRCLK in preparation of sending data
		PORTB = SRCLR;
		// set SER = next bit of data to be sent.
		uc sendBit = (SRCLR | (SER & data));
		data = data >> 1;
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB = (sendBit | SRCLK);
		// end for each bit of data
	}
	// set RCLK = 1. Rising edge copies data from the "Shift" register to the "Storage" register
	PORTB = (SRCLR | RCLK);
	// clears all lines in preparation of a new transmission
	//PORTB = 0x00;
}


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
		if(C1) {
			transmit_data(0x10);
		}
	}
}