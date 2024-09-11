// This is startup program for CECS447 Project 1 part 1.
// Hardware connection: 
// Positive logic Speaker/Headset is connected to PA3.
// onboard two switches are used for music box control.
// Switch 1 toggle between music on/off, switch 2 toggle through the three songs.
// Authors: Min He
// Date: August 28, 2022

#include "tm4c123gh6pm.h"
#include "music.h"
#include "SysTick.h"
#include <stdint.h>

unsigned char playing = 0;

void Delay(void);

// initail values for piano major notes: assume SysTick clock is 16MHz.
const uint32_t Tone_Tab[] =
// initial values for three major notes for 16MHz system clock
// Note name: C, D, E, F, G, A, B
// Offset:0, 1, 2, 3, 4, 5, 6
{30534,27211,24242,22923,20408,18182,16194, // C4 major notes
 15289,13621,12135,11454,10204,9091,8099, // C5 major notes
 7645,6810,6067,5727,5102,4545,4050};// C6 major notes

// Index for notes used in music scores
#define C5 0+7
#define D5 1+7
#define E5 2+7
#define F5 3+7
#define G5 4+7
#define A5 5+7
#define B5 6+7
#define C6 0+2*7
#define D6 1+2*7
#define E6 2+2*7
#define F6 3+2*7
#define G6 4+2*7

#define PAUSE 255				// assume there are less than 255 tones used in any song
//#define MAX_NOTES 50  // assume maximum number of notes in any song is 100. You can change this value if you add a long song.
#define MAX_NOTES 255

uint8_t CURRENT = 0;
 
// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
NTyp Score_Tab[][MAX_NOTES] = {

// score table for Mary Had A Little Lamb
{E5, 4, D5, 4, C5, 4, D5, 4, E5, 4, E5, 4, E5, 8, 
 D5, 4, D5, 4, D5, 8, E5, 4, G5, 4, G5, 8,
 E5, 4, D5, 4, C5, 4, D5, 4, E5, 4, E5, 4, E5, 8, 
 D5, 4, D5, 4, E5, 4, D5, 4, C5, 8, 0, 0 },

 // score table for Twinkle Twinkle Little Stars
{C5,4,C5,4,G5,4,G5,4,A5,4,A5,4,G5,8,F5,4,F5,4,E5,4,E5,4,D5,4,D5,4,C5,8, 
 G5,4,G5,4,F5,4,F5,4,E5,4,E5,4,D5,8,G5,4,G5,4,F5,4,F5,4,E5,4,E5,4,D5,8, 
 C5,4,C5,4,G5,4,G5,4,A5,4,A5,4,G5,8,F5,4,F5,4,E5,4,E5,4,D5,4,D5,4,C5,8,0,0},
	
// score table for Happy Birthday
{//so   so   la   so   doe' ti
   G5,2,G5,2,A5,4,G5,4,C6,4,B5,4,
// pause so   so   la   so   ray' doe'
   PAUSE,4,  G5,2,G5,2,A5,4,G5,4,D6,4,C6,4,
// pause so   so   so'  mi'  doe' ti   la
   PAUSE,4,  G5,2,G5,2,G6,4,E6,4,C6,4,B5,4,A5,8, 
// pause fa'  fa'   mi'  doe' ray' doe' stop
	 PAUSE,4,  F6,2,F6,2, E6,4,C6,4,D6,4,C6,8,0,0}

};

unsigned char note_index=0;

// play the current song once
void play_a_song(void)
{
	
	//For the loops
	unsigned char j=0;
	
	while((note_index < MAX_NOTES) && (is_music_on())){
		NTyp curr_note = Score_Tab[CURRENT][note_index];
		
		unsigned char note = curr_note.tone_index;
		
		//If note is a rest
		if(note == PAUSE){
			SysTick_stop(); //Stop the systick timer...
		}else{
			SysTick_Set_Current_Note(Tone_Tab[note]);
		  SysTick_start();
		}
		
		//Hold note
		for(j=0; j<curr_note.delay; j++){
				Delay();
		}
	
		//Loop Updates
		note_index++;
		
		if(note == 0){
			SysTick_stop();
			note_index = 0;
		}
	}
}

// Move to the next song
void next_song(void)
{
	CURRENT = (CURRENT+1)%3;
	note_index = 0;
}

// check to see if the music is on or not
uint8_t is_music_on(void)
{
	return playing;
}

// turn off the music
void turn_off_music(void)
{
  SysTick_stop();
	playing = 0;
	CURRENT = 0;
	note_index = 0;
}

// turn on the music
void turn_on_music(void)
{
  SysTick_start();
	playing = 1;
	CURRENT = 0;
	note_index = 0;
}

// Initialize music output pin:
// Make PA3 an output to the speaker, 
// enable digital I/O, ensure alt. functions off
void Music_Init(void){
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0; // activate port A
	while ((SYSCTL_RCGCGPIO_R&SYSCTL_RCGCGPIO_R0)!=SYSCTL_RCGCGPIO_R0){}

  GPIO_PORTA_DIR_R |= 0x08;             // make PF2 out (built-in LED)
  GPIO_PORTA_AFSEL_R &= ~0x08;          // disable alt funct on PF2
  GPIO_PORTA_DEN_R |= 0x08;             // enable digital I/O on PF2
  GPIO_PORTA_PCTL_R &= ~0x0000F000;     // configure PA3 to be GPIO
  GPIO_PORTA_AMSEL_R &= ~ 0x08;               // disable analog functionality on PF	
}

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	uint32_t volatile time;
  time = 727240*20/91;  // 0.1sec for 16MHz
//  time = 727240*100/91;  // 0.1sec for 80MHz
  while(time){
		time--;
  }
}


