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

void transmit_data(uc data) {
	
}

int main(void)
{
    DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTA = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	
    while (1) 
    {
    }
}

