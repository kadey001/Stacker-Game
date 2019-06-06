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
#include "queue.h"
#include "scheduler.h"
#include "pwm.c"
#include "shift_reg.c"

#define uc unsigned char
#define us unsigned short
#define ul unsigned long

//Music Notes
#define NOTEC4 261.63
#define NOTED4 293.66
#define NOTEG4 392.00
#define NOTEF4 349.23
#define NOTEA4 440.00
#define NOTEB4 493.88
#define NOTEC5 523.25

//===Shared variables===
//[0] = row zero. EX: 0x01 would place block row 1 column 8 (from top to bottom)
uc placedBlocks[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uc currentBlocks[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uc placedBlock;
uc result = 0x00;
task task3;
ul int GCD;

//===Functions===
us handlePlacement(uc placed, uc current, uc size, uc transition) {
	uc i = 0;
	uc returnSize;
	us returnValue;
	
	if(size == 1) {
		transition = 3;
	}
	else if((size == 2 && transition == 1)) {
		transition = 2;
	}
	if(transition == 0) {//Not a transition, still 3 blocks
		if(placed == current) {
			returnValue = current;
			returnValue = returnValue << 8;
			returnValue = returnValue | 0x0002;
		}
		else {
			//Check if either is on right side
			if(placed & 0x01) {//placed = 0000 0111
				if(current & 0x02) {//current = 0000 1110
					return 0x0602;
				}
				else {
					return 0x0401;
				}
			}
			else if(current & 0x01) {//current = 0000 0111
				if(placed & 0x02) {//placed = 0000 1110
					return 0x0602;
				}
				else {
					return 0x0401;
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
						returnSize = 0x02;
					}
					else {
						returnSize = 0x01;
					}
					returnValue = placed & current;
					returnValue = returnValue << (i + 8);
					returnValue = returnValue | returnSize;
				}
				else {
					if(placed & 0x02) {
						returnSize = 2;
					}
					else {
						returnSize = 0x01;
					}
					returnValue = placed & current;
					returnValue = returnValue << (i + 8);
					returnValue = returnValue | returnSize;
				}
			}
		}
	}
	else if(transition == 1) {//1st Transition -> 2 blocks
		if(placed == current) {
			returnValue = current;
			returnValue = returnValue << 8;
			returnValue = returnValue | 0x0002;
			return returnValue;
		}
		else {
			if(placed & 0x01) {
				if(current & 0x02) {
					returnSize = 2;
				}
				else {
					returnSize = 1;
				}
				returnValue = placed & current;
				returnValue = returnValue << 8;
				returnValue = returnValue | returnSize;
			}
			else if(current & 0x01) {
				if(placed & 0x02) {
					returnSize = 2;
				}
				else {
					returnSize = 1;
				}
				returnValue = placed & current;
				returnValue = returnValue << 8;
				returnValue = returnValue | returnSize;
			}
			else {
				while(!((placed & 0x01) || (current & 0x01))) {
					current = current >> 1;
					placed = placed >> 1;
					i++;
				}
				if(current & 0x01) {
					if(placed & 0x02) {
						returnSize = 2;
					}
					else {
						returnSize = 1;
					}
				}
				else {
					if(current & 0x02) {
						returnSize = 2;
					}
					else {
						returnSize = 1;
					}
				}
				returnValue = placed & current;
				returnValue = returnValue << (i + 8);
				returnValue = returnValue | returnSize;
			}
		}
		
	}
	else if(transition == 2) {//Currently 2
		if(placed == current) {
			returnValue = current;
			returnValue = returnValue << 8;
			returnValue = returnValue | 0x0002;
		}
		else {
			while(!((placed & 0x01) || (current & 0x01))) {
				current = current >> 1;
				placed = placed >> 1;
				i++;
			}
			if(placed & 0x01) {
				if((placed == 0x07) && (current & 0x02)) {
					returnSize = 2;
				}
				else {
					returnSize = 1;
				}
			}
			else {
				if((placed == 0x0E) && (current & 0x04)) {
					returnSize = 2;
				}
				else {
					returnSize = 1;
				}
			}
			returnValue = placed & current;
			returnValue = returnValue << (i + 8);
			returnValue = returnValue | returnSize;
		}
	}
	else {//2nd transition -> 1 block
		returnValue = placed & current;
		returnValue = returnValue << 8;
		returnValue = returnValue | 0x0001;
	}
	return returnValue;
}
uc checkPlacement(uc placed, uc current, uc size) { //Checks if placement was valid, returns # of blocks placed correctly
	if(!(placed & current)) {
		return 0;
	}
	else {
		return 1;
	}
}

//===User Defined FSMs===
enum soundState { SOUND_OFF, SOUND_FAIL, SOUND_WIN, SOUND_PLACE };

int SMTick1(int state) {
	static us success[2] = { NOTEB4, NOTEC5 }; //Sound when block is successfully placed.
	static us win[4] = { NOTEF4, NOTEA4, NOTEB4, NOTEC5 }; //Plays when win
	static us fail[2] = { NOTED4, NOTEC4 }; //Sound when block is misplaced, ending game.
	static uc tick1;
	
	switch(state) {
		case SOUND_OFF:
			if(result) {
				if(result == 0x01) {
					state = SOUND_FAIL;
				}
				else if(result == 0x02) {
					state = SOUND_WIN;
				}
				else if(result == 0x03) {
					state = SOUND_PLACE;
				}
				else {
					state = SOUND_OFF;
				}
				result = 0x00;
			}
			else {
				state = SOUND_OFF;
			}
			break;
		case SOUND_FAIL: 
			if(tick1 == 2) {
				state = SOUND_OFF;
			}
			else {
				state = SOUND_FAIL;
			}
			break;
		case SOUND_WIN:
			if(tick1 == 4) {
				state = SOUND_OFF;
			}
			else {
				state = SOUND_WIN;
			}
			break;
		case SOUND_PLACE:
			state = SOUND_OFF;
			break;
		default: state = SOUND_OFF; break;
	}
	switch(state) {
		case SOUND_OFF: 
			set_PWM(0); 
			tick1 = 0;
			break;
		case SOUND_FAIL:
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
			else if(tick1 == 2) {
				set_PWM(win[2]);
			}
			else {
				set_PWM(win[3]);
			}
			++tick1;
			break;
		case SOUND_PLACE:
			set_PWM(NOTEF4);
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
	static uc difficultySelect;
	static uc tick;
	static uc validPlaced;
	static us handleBlockPlaced;
	static uc blockPlaced;
	
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
					currentSize = 3;
					direction = 0;
					tick = 1;
					currentBlocks[7] = 0x0E;
					LCD_ClearScreen();
					LCD_DisplayString(1, "    PLAYING                ");
					result = 0x03;//Plays tone when block is placed
					if (difficulty == 3) {
						task3.period = 40/GCD;
					}
					else if (difficulty == 2) {
						task3.period = 60/GCD;
					}
					else {
						task3.period = 100/GCD;
					}
				}
			}
			else if(difficultyButton) {
				state = DIFFICULTY_SELECT;
				tick = 1;
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
			if(playButton) {
				play = 1;
			}
			else if(difficultyButton) {
				difficultySelect = 1;
			}
			if(play) {
				if(!playButton) {
					play = 0;
					tick = 1;
					state = MENU;
				}
			}
			if(difficultySelect) {
				if(!difficultyButton) {
					difficultySelect = 0;
					tick = 1;
					if(difficulty == 3) {
						difficulty = 1;
					}
					else {
						difficulty++;
					}
				}
			}
			break;
		default:
			difficulty = 1;
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
				LCD_DisplayString(1, "   PRESS START                     ");
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
				result = 0x03;
				currentRow = 6;
				placedBlocks[7] = currentBlocks[7];
				currentBlocks[6] = 0x07;
			}
			else if(currentRow == 0) {
				if(checkPlacement(placedBlocks[currentRow + 1], currentBlocks[currentRow], currentSize)) {
					state = WIN;
				}
				else {
					state = LOSE;
				}
			}
			else {
				//If placement is valid, continue. ValidPlaced stores 
				validPlaced = checkPlacement(placedBlocks[currentRow + 1], currentBlocks[currentRow], currentSize);
				if(validPlaced) {
					result = 0x03;//Plays tone when block is placed
					//placedBlocks[currentRow] = currentBlocks[currentRow];
					if(currentRow == 6) {
						handleBlockPlaced = handlePlacement(placedBlocks[currentRow + 1], currentBlocks[currentRow], currentSize, 0);
						currentSize = handleBlockPlaced;
						placedBlock = handleBlockPlaced >> 8;
					}
					else if(currentRow >= 4) {
						handleBlockPlaced = handlePlacement(placedBlocks[currentRow + 1], currentBlocks[currentRow], currentSize, 1);
						currentSize = handleBlockPlaced;
						placedBlock = handleBlockPlaced >> 8;
					}
					else {
						handleBlockPlaced = handlePlacement(placedBlocks[currentRow + 1], currentBlocks[currentRow], currentSize, 3);
						currentSize = handleBlockPlaced;
						placedBlock = handleBlockPlaced >> 8;
					}
					placedBlocks[currentRow] = placedBlock;
					currentRow--;
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
					state = LOSE;
				}
			}
			break;
		case WIN:
			//Write to LCD YOU WIN!
			if(tick == 1) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "    YOU WIN!                     ");
				tick = 0;
				result = 0x02;
			}
			break;
		case LOSE:
			//Write to LCD YOU LOSE
			if(tick == 1) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "    YOU LOSE                     ");
				tick = 0;
				result = 0x01;
			}
			break;
		case GAME_RESET:
			//Nothing
			break;
		case DIFFICULTY_SELECT:
			if(tick == 1) {
				LCD_ClearScreen();
				if(difficulty == 1) {
					LCD_DisplayString(1, "  Difficulty:        EASY       ");
				}
				else if(difficulty == 2) {
					LCD_DisplayString(1,"  Difficulty:       MEDIUM       ");
				}
				else {
					LCD_DisplayString(1,"  Difficulty:        HARD        ");
				}
				tick = 0;
			}
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
	ul int SMTick1_calc = 150;
	ul int SMTick2_calc = 2;
	ul int SMTick3_calc = 100;
	
	//Calculating GCD
	GCD = findGCD(SMTick1_calc, SMTick2_calc);
	GCD = findGCD(GCD, SMTick3_calc);
	
	//Recalculate GCD periods for scheduler
	ul int SMTick1_period = SMTick1_calc/GCD;
	ul int SMTick2_period = SMTick2_calc/GCD;
	ul int SMTick3_period = SMTick3_calc/GCD;
	
	//Array of Tasks
	static task task1;
	static task task2;
	//static task task3;
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