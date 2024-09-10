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
#include "ButtonLed.h"

// define bit addresses for Port B bits 0,1,2,3,4,5 => DAC inputs 
#define DAC (*((volatile unsigned long *)0x400053F0 ))   

unsigned char Index;
	
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
#define B5 6+2*7
#define C6 0+3*7
#define D6 1+3*7
#define E6 2+3*7
#define F6 3+3*7
#define G6 4+3*7
#define A6 5+3*7
#define B6 6+3*7	 

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
{{//so   so   la   so   doe' ti
   G5,2,G5,2,A5,4,G5,4,C6,4,B5,4,
// pause so   so   la   so   ray' doe'
   SILENCE,4,  G5,2,G5,2,A5,4,G5,4,D6,4,C6,4,
// pause so   so   so'  mi'  doe' ti   la
   SILENCE,4,  G5,2,G5,2,G6,4,E6,4,C6,4,B5,4,A5,8, 
// pause fa'  fa'   mi'  doe' ray' doe' stop
	 SILENCE,4,  F6,2,F6,2, E6,4,C6,4,D6,4,C6,8,0,0},

// Mary Had A Little Lamb
{E5, 4, D5, 4, C5, 4, D5, 4, E5, 4, E5, 4, E5, 8, 
 D5, 4, D5, 4, D5, 8, E5, 4, G5, 4, G5, 8,
 E5, 4, D5, 4, C5, 4, D5, 4, E5, 4, E5, 4, E5, 8, 
 D5, 4, D5, 4, E5, 4, D5, 4, C5, 8, 0, 0 },

// Twinkle Twinkle Little Stars
{C5,4,C5,4,G5,4,G5,4,A5,4,A5,4,G5,8,F5,4,F5,4,E5,4,E5,4,D5,4,D5,4,C5,8, 
 G5,4,G5,4,F5,4,F5,4,E5,4,E5,4,D5,8,G5,4,G5,4,F5,4,F5,4,E5,4,E5,4,D5,8, 
 C5,4,C5,4,G5,4,G5,4,A5,4,A5,4,G5,8,F5,4,F5,4,E5,4,E5,4,D5,4,D5,4,C5,8,0,0}
};


// File scope golbal
volatile uint8_t curr_song=0;      // 0: Happy Birthday, 1: Mary Had A Little Lamb. 2: Twinkle Twinkle Little Stars
volatile uint8_t stop_play=1;      // 0: continue playing a song, 1: stop playing a song
volatile uint8_t octave=0;         // 0: lower C, 1: middle C, 2: upper C
volatile uint8_t curr_note=0;

volatile uint8_t curr_mode=PIANO;  // 0: piano mode, 1: auto-play mode
volatile uint8_t music_playing=0;

																		// **************DAC_Init*********************
// Initialize 6-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){ unsigned long volatile delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
  delay = SYSCTL_RCGC2_R;    // allow time to finish activating
  GPIO_PORTB_AMSEL_R &= ~0x3F;      // no analog 
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // regular function
  GPIO_PORTB_DIR_R |= 0x3F;      // make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F;   // disable alt funct on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;      // enable digital I/O on PB5-0
  GPIO_PORTB_DR8R_R |= 0x3F;        // enable 8 mA drive on PB5-0
}

void Sound_Init(){
	Index=0;
	NVIC_ST_CTRL_R = 0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000;
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC|NVIC_ST_CTRL_INTEN;
}

// **************Sound_Start*********************
// Set reload value and enable systick timer
// Input: time duration to be generated in number of machine cycles
// Output: none
void Sound_Start(uint32_t period){
	NVIC_ST_RELOAD_R = period-1;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;
}

void Sound_stop(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
}

// Interrupt service routine
// Executed based on number of sampels in each period
void SysTick_Handler(void){
	GPIO_PORTF_DATA_R ^= 0x08;     // toggle PF3, debugging
  Index = (Index+1)&0x3F;        // 16 samples for each period
	//Index = (Index+1)%16;
	DAC = SineWave[Index]; // output to DAC: 3-bit data
}


void GPIOPortF_Handler(void){
	// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}
	if(GPIO_PORTF_RIS_R&0x01){
			if(get_current_mode()==PIANO){
				octave=(octave+1)%NUM_OCT;
			}else{
				Sound_stop();
				curr_song=(curr_song+1)%NUM_SONGS;
				curr_note = 0;
			}
			GPIO_PORTF_ICR_R = 0x01;
	}
	if(GPIO_PORTF_RIS_R&0x10){
		if(get_current_mode()==PIANO){
			curr_mode = AUTO_PLAY;
			curr_note = 0;
			curr_song = 0;
		}else{
			curr_mode = PIANO;
		}
		GPIO_PORTF_ICR_R = 0x10;
	}
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


// Dependency: Requires PianoKeys_Init to be called first, assume at any time only one key is pressed
// Inputs: None
// Outputs: None
// Description: Rising/Falling edge interrupt on PD6-PD0. Whenever any 
// button is pressed, or released the interrupt will trigger.
void GPIOPortD_Handler(void){  
	uint8_t i = 0;
  // simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<72724;time++) {}
  if(get_current_mode()==PIANO){
		if(GPIO_PORTD_RIS_R&0x01){
			Sound_Start(tonetab[0+(octave*7)]/NUM_SAMPLES);
			for(i=0;i<10;i++){
					Delay();
			}
			Sound_stop();
			GPIO_PORTD_ICR_R = 0x01;
		}
		if(GPIO_PORTD_RIS_R&0x02){
			Sound_Start(tonetab[1+(octave*7)]/NUM_SAMPLES);
			for(i=0;i<10;i++){
					Delay();
			}
			Sound_stop();
			GPIO_PORTD_ICR_R = 0x02;
		}
		if(GPIO_PORTD_RIS_R&0x04){
			Sound_Start(tonetab[2+(octave*7)]/NUM_SAMPLES);
			for(i=0;i<10;i++){
					Delay();
			}
			Sound_stop();
			GPIO_PORTD_ICR_R = 0x04;
		}
		if(GPIO_PORTD_RIS_R&0x08){
			Sound_Start(tonetab[3+(octave*7)]/NUM_SAMPLES);
			for(i=0;i<10;i++){
					Delay();
			}
			Sound_stop();
			GPIO_PORTD_ICR_R = 0x08;
		}
	}else{
			GPIO_PORTD_ICR_R = 0x0F;
	}
}

uint16_t counter = 0;

uint8_t is_music_on(void){
  return music_playing;
}

void play_a_song(){
	curr_note=0;
	
	while(playlist[curr_song][curr_note].delay){
		if(get_current_mode()==AUTO_PLAY){
					if(playlist[curr_song][curr_note].tone_index == SILENCE){
						Sound_stop();
					}
					else{
						Sound_Start(tonetab[playlist[curr_song][curr_note].tone_index]/NUM_SAMPLES);
					}
					
					for(counter=0;counter<playlist[curr_song][curr_note].delay;counter++){
						Delay();
					}
					
					Sound_stop();
					curr_note++;
		}else{
				break;
		}
	}
}


uint8_t get_current_mode(void)
{
	return curr_mode;
}



