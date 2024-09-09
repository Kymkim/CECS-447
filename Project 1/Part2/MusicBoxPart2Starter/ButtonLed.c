// ButtonLed.c: starter file for CECS447 Project 1 Part 1
// Runs on TM4C123, 
// Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "ButtonLed.h"

// Constants
#define SW1 0x10  // bit position for onboard switch 1(left switch)
#define SW2 0x01  // bit position for onboard switch 2(right switch)
#define NVIC_EN0_PORTF 0x40000000  // enable PORTF edge interrupt
#define NVIC_EN0_PORTD 0x00000008  // enable PORTD edge interrupt

// Golbals
volatile uint8_t curr_mode=PIANO;  // 0: piano mode, 1: auto-play mode

//---------------------Switch_Init---------------------
// initialize onboard switch and LED interface
// Input: none
// Output: none 
void ButtonLed_Init(void){ 
	
}

//---------------------PianoKeys_Init---------------------
// initialize onboard Piano keys interface: PORT D 0 - 3 are used to generate 
// tones: CDEF:doe ray mi fa
// No need to unlock. Only PD7 needs to be unlocked.
// Input: none
// Output: none 
void PianoKeys_Init(void){ 
}


uint8_t get_current_mode(void)
{
	return curr_mode;
}
