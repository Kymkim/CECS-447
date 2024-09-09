// This is an example program for music programming.
// You will run this program without modification. 

// The program will continuously play "Happy Birthday"
// Authors: Min He
// Date: August 26, 2018

// Header files 
#include "tm4c123gh6pm.h"
#include "Sound.h"
#include "SwitchLed.h"

// 2. Declarations Section

// define music note data structure 
struct Note {
  unsigned long tone_index;
  unsigned char delay;
};
typedef const struct Note NTyp;

// Constants
#define TONE_DURATION 2 // each tone uses the same duration 
#define NUM_VALs  16  // Assume 3-bit DAC is used, that will give 16 values for one period.

// initial values for piano major tones.
// Assume SysTick clock frequency is 16MHz.
const unsigned long tonetab[] =
// C, D, E, F, G, A, B
// 1, 2, 3, 4, 5, 6, 7
// C3: 122137,108844,96970,91429,81633,72727,64777   // C3 Major notes
{30534*2,27211*2,24242*2,22923*2,20408*2,18182*2,16194*2, // C4 Major notes
 15289*2,13621*2,12135*2,11454*2,10204*2,9091*2,8099*2, // C5 Major notes
 7645*2,6810*2,6067*2,5727*2,5102*2,4545,4050*2}; // C6 Major notes

// index definition for tones used in happy birthday.
#define G6 4
#define A6 5
#define B6 6
#define C7 0+7
#define D7 1+7
#define E7 2+7
#define F7 3+7
#define G7 4+7
#define PAUSE 255
 
// note table for Happy Birthday
// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
NTyp happybirthday[] = 
{
// so   so   la   so   doe' ti
   G6,2,G6,2,A6,4,G6,4,C7,4,B6,4,
// pause so   so   la   so   ray' doe'
   PAUSE,4,  G6,2,G6,2,A6,4,G6,4,D7,4,C7,4,
// pause so   so   so'  mi'  doe' ti   la
   PAUSE, 4, G6,2,G6,2,G7,4,E7,4,C7,4,B6,4,A6,8, 
// pause fa'  fa'   mi'  doe' ray' doe'  stop
	 PAUSE,4,  F7,2,F7,2, E7,4,C7,4,D7,4,C7,8, 0,0
};

//   Function Prototypes
void Delay(void);
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);
void play_a_song(NTyp notetab[]);

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
  DisableInterrupts();    
  DAC_Init();       // Port B is DAC
	Sound_Init();
	SwitchLed_Init();
	EnableInterrupts();
	
  while(1){
		play_a_song(happybirthday);
  }
}

void play_a_song(NTyp notetab[])
{
	unsigned char i=0, j;

	while (notetab[i].delay) {
		if (notetab[i].tone_index==PAUSE) 
			Sound_Stop(); // silence tone, turn off SysTick timer
		else {
			Sound_Start(tonetab[notetab[i].tone_index]/NUM_VALs);
		}
		
		// tempo control: play current note for specified duration
		for (j=0;j<notetab[i].delay;j++) 
			Delay();
		
		Sound_Stop();
		i++;  // move to the next note
	}
	
	// pause after each play
	for (j=0;j<15;j++) 
		Delay();
}

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	unsigned long volatile time;
  time = 727240*20/91;  // 0.1sec
  while(time){
		time--;
  }
}
