// Sound.c
// This is the starter file for CECS 447 Project 1 Part 2
// By Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include "tm4c123gh6pm.h"
#include "sound.h"
#include <stdint.h>

// define bit addresses for Port B bits 0,1,2,3,4,5 => DAC inputs 
// #define DAC (*((volatile unsigned long *)            ))   
	
// 6-bit: value range 0 to 2^6-1=63, 64 samples
const uint8_t SineWave[64] = {32,35,38,41,44,47,49,52,54,56,58,59,61,62,62,63,63,63,62,62,
															61,59,58,56,54,52,49,47,44,41,38,35,32,29,26,23,20,17,15,12,
															10, 8, 6, 5, 3, 2, 2, 1, 1, 1, 2, 2, 3, 5, 6, 8,10,12,15,17,
															20,23,26,29};

// initial values for piano major tones.
// Assume SysTick clock frequency is 16MHz.
const uint32_t tonetab[] =
// C, D, E, F, G, A, B
// 1, 2, 3, 4, 5, 6, 7
// lower C octave:130.813, 146.832,164.814,174.614,195.998, 220,246.942
// calculate reload value for the whole period:Reload value = Fclk/Ft = 16MHz/Ft
{122137,108844,96970,91429,81633,72727,64777,
 30534*2,27211*2,24242*2,22923*2,20408*2,18182*2,16194*2, // C4 Major notes
 15289*2,13621*2,12135*2,11454*2,10204*2,9091*2,8099*2,   // C5 Major notes
 7645*2,6810*2,6067*2,5727*2,5102*2,4545*2,4050*2};        // C6 Major notes

// Constants
// index definition for tones used in happy birthday.
#define G4 4
#define A4 5
#define B4 6
#define C5 0+7
#define D5 1+7
#define E5 2+7
#define F5 3+7
#define G5 4+7
#define A5 5+7		

#define MAX_NOTES 255 // maximum number of notes for a song to be played in the program
#define NUM_SONGS 3   // number of songs in the play list.
#define SILENCE MAX_NOTES // use the last valid index to indicate a silence note. The song can only have up to 254 notes. 
#define NUM_OCT  3   // number of octave defined in tontab[]
#define NUM_NOTES_PER_OCT 7  // number of notes defined for each octave in tonetab
#define NVIC_EN0_PORTF 0x40000000  // enable PORTF edge interrupt
#define NVIC_EN0_PORTD 0x00000008  // enable PORTD edge interrupt
#define NUM_SAMPLES 64  // number of sample in one sin wave period

// note table for Happy Birthday
// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
NTyp playlist[][MAX_NOTES] = 
{{
  // Happy Birthday
// so   so   la   so   doe' ti
   G4,2,G4,2,A4,4,G4,4,C5,4,B4,4,
// pause so   so   la   so   ray' doe'
   SILENCE,4,  G4,2,G4,2,A4,4,G4,4,D5,4,C5,4,
// pause so   so   so'  mi'  doe' ti   la
   SILENCE, 4, G4,2,G4,2,G5,4,E5,4,C5,4,B4,4,A4,8, 
// pause fa'  fa'   mi'  doe' ray' doe'  stop
	 SILENCE,4,  F5,2,F5,2, E5,4,C5,4,D5,4,C5,8, SILENCE,0
},

// Mary Had A Little Lamb
{SILENCE,0},

// Twinkle Twinkle Little Stars
{SILENCE,0}
};


// File scope golbal
volatile uint8_t curr_song=0;      // 0: Happy Birthday, 1: Mary Had A Little Lamb. 2: Twinkle Twinkle Little Stars
volatile uint8_t stop_play=1;      // 0: continue playing a song, 1: stop playing a song
volatile uint8_t octave=0;         // 0: lower C, 1: middle C, 2: upper C

																		// **************DAC_Init*********************
// Initialize 6-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){
}

// **************Sound_Start*********************
// Set reload value and enable systick timer
// Input: time duration to be generated in number of machine cycles
// Output: none
void Sound_Start(uint32_t period){
}

void Sound_stop(void)
{
}

// Interrupt service routine
// Executed based on number of sampels in each period
void SysTick_Handler(void){
}

void GPIOPortF_Handler(void){
	// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}
}

// Dependency: Requires PianoKeys_Init to be called first, assume at any time only one key is pressed
// Inputs: None
// Outputs: None
// Description: Rising/Falling edge interrupt on PD6-PD0. Whenever any 
// button is pressed, or released the interrupt will trigger.
void GPIOPortD_Handler(void){  
  // simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}
    
}

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	uint32_t volatile time;
  time = 727240*20/91;  // 0.1sec
  while(time){
		time--;
  }
}

void play_a_song()
{
}


