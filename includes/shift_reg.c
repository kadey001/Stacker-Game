#include <avr/io.h>

#define uc unsigned char

#define SER 0x0001
#define RCLK 0x0002
#define SRCLK 0x0004
#define SRCLR 0x0008

void transmit_data(uc data1, uc data2) {
	unsigned short data = data1;
	data = data << 8;
	data = data + data2;
	
	// for each bit of data
	for(uc i = 0; i < 16; ++i){
		// Set SRCLR to 1 allowing data to be set
		// Also clear SRCLK in preparation of sending data
		PORTB = SRCLR;
		// set SER = next bit of data to be sent.
		unsigned short sendBit = (SRCLR | (SER & data));
		data = data >> 1;
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB = (sendBit | SRCLK);
		// end for each bit of data
	}
	// set RCLK = 1. Rising edge copies data from the "Shift" register to the "Storage" register
	PORTB = (SRCLR | RCLK);
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}
void clear_data() {
	PORTB = SRCLK;
	PORTB = RCLK;
}