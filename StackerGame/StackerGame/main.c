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
#define us unsigned short
#define ul unsigned long

#define NOTEC4 261.63
#define NOTED4 293.66
#define NOTEB4 493.88
#define NOTEC5 523.25

us success[2] = { NOTEB4, NOTEC5 };
us fail[2] = { NOTED4, NOTEC4 };

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

//===User Defined FSMs===
enum soundState { OFF, SUCCESS, FAIL };
void 

int SMTick1(int state) {
	
	uc button1 = ~PINC & 0x01;
	uc button2 = ~PINC & 0x02;
	static uc tick1;
	
	switch(state) {
		case OFF:
			if(button1) {
				state = SUCCESS;
			}
			else if(button2) {
				state = FAIL;
			}
			else {
				state = OFF;
			}
			break;
		case SUCCESS: 
			if(tick1 == 2) {
				state = OFF;
			}
			else {
				state = SUCCESS;
			}
			break;
		case FAIL: 
			if(tick1 == 2) {
				state = OFF;
			}
			else {
				state = FAIL;
			}
			break;
	}
	switch(state) {
		case OFF: 
			set_PWM(0); 
			tick1 = 0;
			break;
		case SUCCESS: 
			if(tick1 == 0) {
				set_PWM(success[0]);
			}
			else if(tick1 == 1) {
				set_PWM(success[1]);
			}
			++tick1;
			break;
		case FAIL:
			if(tick1 == 0) {
				set_PWM(fail[0]);
			}
			else if(tick1 == 1) {
				set_PWM(fail[1]);
			}
			++tick1;
			break;
	}
	return state;
}

int main(void)
{
    DDRA = 0xFF; PORTA = 0x00; //LCD Control Lines
	DDRB = 0xFF; PORTB = 0x00; //Shift Register
	DDRC = 0x00; PORTC = 0xFF; //INPUTS
	DDRD = 0xFF; PORTD = 0x00; //LCD Data Lines
	
	//Tasks Period
	ul int SMTick1_calc = 200;
	
	
	//Calculating GCD
	ul int GCD = findGCD(SMTick1_calc, SMTick1_calc);
	
	
	//Recalculate GCD periods for scheduler
	ul int SMTick1_period = SMTick1_calc/GCD;
	
	
	//Array of Tasks
	static task task1;
	task *tasks[] = { &task1 };
	const us numTasks = sizeof(tasks)/sizeof(task*);
	
	//Task 1
	task1.state = -1; //Init
	task1.period = SMTick1_period;
	task1.elapsedTime = SMTick1_period;
	task1.TickFct = &SMTick1;
	
	
	PWM_on();
	TimerSet(50);
	TimerOn();
	
	LCD_init();
	LCD_DisplayString(1, "PRESS START");
	
	//uc C0 = 0x00;
	//uc C1 = 0x00;
	//uc C2 = 0x00;
	
	transmit_data(0xF0);
	
	us i; //Loop iterator
    while (1) 
    {
		
		//Scheduler code
		for (i = 0; i < numTasks; i++) {
			//Task ready to tick
			if (tasks[i]->elapsedTime >= tasks[i]->period) {
				//Setup next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				//Reset elapsed time for next tick
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime +=1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
}