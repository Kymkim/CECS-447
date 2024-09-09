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
	NVIC_ST_CTRL_R = 0;	//Disable SysTick
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) | 0x40000000; //Priority 2
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC+NVIC_ST_CTRL_INTEN;
}

// Start running systick timer
void SysTick_start(void)
{
	NVIC_ST_CTRL_R	|= NVIC_ST_CTRL_ENABLE;	
}
// Stop running systick timer
void SysTick_stop(void)
{
	NVIC_ST_CTRL_R	&= ~NVIC_ST_CTRL_ENABLE;				
}

// update the reload value for current note with with n_value
void SysTick_Set_Current_Note(uint32_t n_value)
{
	//SysTick_stop();
	NVIC_ST_RELOAD_R = n_value-1;
	NVIC_ST_CURRENT_R =	0x00000000;		
	//SysTick_start();
}

// Interrupt service routine, 
// frequency is determined by current tone being played:
// stop systick timer, toggle speaker output, update reload value with current note reload value, 
// clear the CURRENT register and restart systick timer.
void SysTick_Handler(void){
	//NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; //Disable SYSTICK
	SPEAKER ^= 0x08;	//Toggle bit 
	NVIC_ST_CURRENT_R = 0;			//Any write clears current count
	//NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE; //Activate SYSTICK
}
