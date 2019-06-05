/*
 * StackerGame.c
 *
 * Created: 5/22/2019 3:19:12 PM
 * Author : Kelton Adey
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>
#include "io.c"
#include "timer.h"
#include "stack.h"
#include "scheduler.h"
#include "pwm.c"
#include "shift_reg.c"

#define uc unsigned char
#define us unsigned short
#define ul unsigned long

#define NOTEC4 261.63
#define NOTED4 293.66
#define NOTEG4 392.00
#define NOTEA4 440.00
#define NOTEB4 493.88
#define NOTEC5 523.25



//===Shared variables===
//[0] = row zero. EX: 0x01 would place block row 1 column 8
uc placedBlocks[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uc currentBlocks[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uc placedBlock;
Stack result;

//===Functions===
uc checkPlacement(uc placed, uc current, uc size) { //Checks if placement was valid, returns # of blocks placed correctly
	uc i = 0;
	if(!(placed & current)) {
		return 0;
	}
	else if(placed == current) {
		return size;
	}
	else {
		switch(size) {
			case 3:
				//Check if either is on right side
				if(placed & 0x01) {
					if(current & 0x02) {
						return 2;
					}
					else {
						return 1;
					}
				}
				else if(current & 0x01) {
					if(placed & 0x02) {
						return 2;
					}
					else {
						return 1;
					}
				}
				else {
					//Neither on right side
					while(!((placed & 0x01) || (current & 0x01))) {
						current = current >> 1;
						placed = placed >> 1;
						i++;
					}
					if(placed & 0x01) {
						if(current & 0x02) {
							return 2;
						}
						else {
							return 1;
						}
					}
					else if(current & 0x01) {
						if(placed & 0x02) {
							return 2;
						}
						else {
							return 1;
						}
					}
				}
				break;
			case 2:
				return 1;
				break;
			case 1:
				return 0;
				break;
		}
	}
	return 0;
}

//===User Defined FSMs===
enum soundState { OFF, SUCCESS, FAIL, SOUND_WIN };

int SMTick1(int state) {
	static us success[2] = { NOTEB4, NOTEC5 }; //Sound when block is successfully placed.
	static us win[4] = { NOTEG4, NOTEA4, NOTEB4, NOTEC5 }; //Plays when win
	static us fail[2] = { NOTED4, NOTEC4 }; //Sound when block is misplaced, ending game.
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
		case SOUND_WIN:
			if(tick1 == 4) {
				state = OFF;
			}
			else {
				state = SOUND_WIN;
			}
			break;
		default: state = OFF; break;
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
		case SOUND_WIN:
			if(tick1 == 0) {
				set_PWM(win[0]);
			}
			else if(tick1 == 1) {
				set_PWM(win[1]);
			}
			else if(tick1 == 1) {
				set_PWM(win[2]);
			}
			else {
				set_PWM(win[3]);
			}
			break;
	}
	return state;
}

enum displayState { ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8 };

int SMTick2(int state) {
	clear_data();
	
	switch(state) {
		case ROW1:
			if(placedBlocks[0] != 0) {
				transmit_data(placedBlocks[0], 0x7F);
			}
			else {
				transmit_data(currentBlocks[0], 0x7F);
			}
			state = ROW2;
			break;
		case ROW2:
			if(placedBlocks[1] != 0) {
				transmit_data(placedBlocks[1], 0xBF);
			}
			else {
				transmit_data(currentBlocks[1], 0xBF);
			}
			state = ROW3;
			break;
		case ROW3:
			if(placedBlocks[2] != 0) {
				transmit_data(placedBlocks[2], 0xDF);
			}
			else {
				transmit_data(currentBlocks[2], 0xDF);
			}
			state = ROW4;
			break;
		case ROW4:
			if(placedBlocks[3] != 0) {
				transmit_data(placedBlocks[3], 0xEF);
			}
			else {
				transmit_data(currentBlocks[3], 0xEF);
			}
			state = ROW5;
			break;
		case ROW5:
			if(placedBlocks[4] != 0) {
				transmit_data(placedBlocks[4], 0xF7);
			}
			else {
				transmit_data(currentBlocks[4], 0xF7);
			}
			state = ROW6;
			break;
		case ROW6:
			if(placedBlocks[5] != 0) {
				transmit_data(placedBlocks[5], 0xFB);
			}
			else {
				transmit_data(currentBlocks[5], 0xFB);
			}
			state = ROW7;
			break;
		case ROW7:
			if(placedBlocks[6] != 0) {
				transmit_data(placedBlocks[6], 0xFD);
			}
			else {
				transmit_data(currentBlocks[6], 0xFD);
			}
			state = ROW8;
			break;
		case ROW8:
			if(placedBlocks[7] != 0) {
				transmit_data(placedBlocks[7], 0xFE);
			}
			else {
				transmit_data(currentBlocks[7], 0xFE);
			}
			state = ROW1;
			break;
		default:
			state = ROW1;
			break;
	}
	
	return state;
}

enum gameState { MENU, PLAY, CHECK, WIN, LOSE, GAME_RESET, DIFFICULTY_SELECT };

int SMTick3(int state) {
	static uc play;
	static uc currentRow;
	static uc currentSize; //Size of current blocks 1-3
	static uc direction; //0 = Left | 1 = Right
	static uc difficulty; //number of starting blocks (lower is harder)
	static uc tick;
	
	uc playButton = ~PINC & 0x01;
	uc gameResetButton = ~PINC & 0x02;
	uc difficultyButton = ~PINC & 0x04;
	
	switch(state) {
		case MENU:
			if(playButton) {
				play = 1;
			}
			if(play) {
				if(!playButton) {
					play = 0;
					state = PLAY;
					currentRow = 7;
					currentSize = difficulty;
					direction = 0;
					tick = 1;
					if (difficulty == 3) {
						currentBlocks[7] = 0x07;
					}
					else if (difficulty == 2) {
						currentBlocks[7] = 0x18;
					}
					else {
						currentBlocks[7] = 0x10;
					}
				}
			}
			else if(difficultyButton) {
				state = DIFFICULTY_SELECT;
			}
			else {
				state = MENU;
			}
			break;
		case PLAY:
			if(gameResetButton) {
				state = MENU;
			}
			else if(playButton) {
				state = CHECK;
			}
			else {
				state = PLAY;
			}
			break;
		case CHECK:
			state = PLAY;
			break;
		case WIN:
			//Nothing
			if(playButton) {
				play = 1;
			}
			if(play) {
				if(!playButton) {
					play = 0;
					state = MENU;
					tick = 1;
				}
			}
			else {
				state = WIN;
			}
			break;
		case LOSE:
			//Nothing
			if(playButton) {
				play = 1;
			}
			if(play) {
				if(!playButton) {
					play = 0;
					state = MENU;
					tick = 1;
				}
			}
			else {
				state = LOSE;
			}
			break;
		case GAME_RESET:
			//Nothing
			break;
		case DIFFICULTY_SELECT:
			//Nothing
			break;
		default:
			difficulty = 3;
			tick = 1;
			state = MENU;
			break;
	}
	uc i;
	switch(state) {
		case MENU:
			if(tick == 1) {
				for(i = 0; i < 8; i++) {
					placedBlocks[i] = 0;
					currentBlocks[i] = 0;
				}
				LCD_ClearScreen();
				LCD_DisplayString(1, "Play/Difficulty");
				tick = 0;
			}
			break;
		case PLAY:
			if(currentBlocks[currentRow] & 0x80) {
				direction = 0;
			}
			else if(currentBlocks[currentRow] & 0x01) {
				direction = 1;
			}
			if(direction) {//Right
				currentBlocks[currentRow] = currentBlocks[currentRow] << 1;
			}
			else {//Left
				currentBlocks[currentRow] = currentBlocks[currentRow] >> 1;
			}
			break;
		case CHECK:
			if(currentRow == 7) {
				currentRow = 6;
				placedBlocks[7] = currentBlocks[7];
				if (difficulty == 3) {
					currentBlocks[6] = 0x07;
				}
				else if (difficulty == 2) {
					currentBlocks[6] = 0x18;
				}
				else {
					currentBlocks[6] = 0x10;
				}
			}
			else if(currentRow == 0) {
				if(checkPlacement(placedBlocks[currentRow - 1], currentBlocks[currentRow], currentSize)) {
					state = WIN;
				}
				else {
					state = LOSE;
				}
			}
			else {
				//If placement is valid, continue. ValidPlaced stores 
				uc validPlaced = checkPlacement(placedBlocks[currentRow + 1], currentBlocks[currentRow], currentSize);
				if(validPlaced) {
					placedBlocks[currentRow] = currentBlocks[currentRow];
					currentRow--;
					if(currentRow == 6) {
						currentSize = validPlaced;
					}
					else if(currentRow >= 3) {
						if(validPlaced > 2) {
							currentSize = 2;
						}
						else {
							currentSize = validPlaced;
						}
					}
					else {
						currentSize = 1;
					}
					
					if(currentSize == 3) {
						currentBlocks[currentRow] = 0x38;
					}
					else if(currentSize == 2) {
						currentBlocks[currentRow] = 0x18;
					}
					else {
						currentBlocks[currentRow] = 0x08;
					}
				}
				else {
					PORTA = PORTA | 0x80;
					state = LOSE;
				}
			}
			break;
		case WIN:
			//Write to LCD YOU WIN!
			if(tick == 1) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "YOU WIN!");
				tick = 0;
			}
			break;
		case LOSE:
			//Write to LCD YOU LOSE
			if(tick == 1) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "YOU LOSE");
				tick = 0;
			}
			break;
		case GAME_RESET:
			//Nothing
			break;
		case DIFFICULTY_SELECT:
			//TODO
			break;
		default:
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
	ul int SMTick2_calc = 2;
	ul int SMTick3_calc = 100;
	
	//Calculating GCD
	ul int GCD = findGCD(SMTick1_calc, SMTick2_calc);
	GCD = findGCD(GCD, SMTick3_calc);
	
	//Recalculate GCD periods for scheduler
	ul int SMTick1_period = SMTick1_calc/GCD;
	ul int SMTick2_period = SMTick2_calc/GCD;
	ul int SMTick3_period = SMTick3_calc/GCD;
	
	//Array of Tasks
	static task task1;
	static task task2;
	static task task3;
	task *tasks[] = { &task1, &task2, &task3 };
	const us numTasks = sizeof(tasks)/sizeof(task*);
	
	//Task 1
	task1.state = -1; //Init
	task1.period = SMTick1_period;
	task1.elapsedTime = SMTick1_period;
	task1.TickFct = &SMTick1;
	
	//Task 2
	task2.state = -1; //Init
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &SMTick2;
	
	//Task 3
	task3.state = -1; //Init
	task3.period = SMTick3_period;
	task3.elapsedTime = SMTick3_period;
	task3.TickFct = &SMTick3;
	
	PWM_on();
	TimerSet(GCD);
	TimerOn();
	
	LCD_init();
	LCD_DisplayString(1, "PRESS START");
	
	clear_data();
	clear_data();
	
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