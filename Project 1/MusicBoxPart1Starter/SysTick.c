// SysTick.c
// Runs on TM4C123. 
// This module provides timing control for CECS447 Project 1 Part 1.
// Authors: Min He
// Date: August 28, 2022

#include "tm4c123gh6pm.h"
#include <stdint.h>

// TODO: Define the bit address for PA3 which connects to the speaker.
#define SPEAKER (*((volatile unsigned long *)0x400043FC))

// Initialize SysTick with interrupt priority 2. Do not start it.
void SysTick_Init(void){
	
	NVIC_ST_CTRL_R		=	0x00000000; 				//Disable SysTick Timer
	NVIC_ST_RELOAD_R 	= NVIC_ST_RELOAD_M; 	//Temporary Reload valie
	NVIC_ST_CURRENT_R =	0x00000000;					//Reset current value
	NVIC_SYS_PRI3_R 	=	(NVIC_SYS_PRI3_R&0x1FFFFFFF) | 0x4000000;
	
}

// Start running systick timer
void SysTick_start(void)
{
	NVIC_ST_CTRL_R	|= 0x07;				
}
// Stop running systick timer
void SysTick_stop(void)
{
	NVIC_ST_CTRL_R		&= ~(0x07);				
}

// update the reload value for current note with with n_value
void SysTick_Set_Current_Note(uint32_t n_value)
{
	NVIC_ST_RELOAD_R = n_value;
}

// Interrupt service routine, 
// frequency is determined by current tone being played:
// stop systick timer, toggle speaker output, update reload value with current note reload value, 
// clear the CURRENT register and restart systick timer.
void SysTick_Handler(void){
	SPEAKER ^= 0x08;
}
