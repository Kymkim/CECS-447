// Sound.c, derived from SysTickInts.c

// Runs on LM4F120 or TM4C123, 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// September 15, 2013
// Port B bits 2-0 have the 3-bit DAC
// Port F is onboard LaunchPad switches and LED
// Port F bit 4 is negative logic switch to play sound, SW1
// SysTick ISR: PF3 ISR heartbeat
#include "tm4c123gh6pm.h"

#define DAC (*((volatile unsigned long *)0x4000501C))
	
unsigned char Index;  

// 3-bit 16-element sine wave
const unsigned char SineWave[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3};

// **************DAC_Init*********************
// Initialize 3-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){unsigned long volatile delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
  delay = SYSCTL_RCGC2_R;    // allow time to finish activating
  GPIO_PORTB_AMSEL_R &= ~0x07;      // no analog 
  GPIO_PORTB_PCTL_R &= ~0x00000FFF; // regular function
  GPIO_PORTB_DIR_R |= 0x07;      // make PB2-0 out
  GPIO_PORTB_AFSEL_R &= ~0x07;   // disable alt funct on PB2-0
  GPIO_PORTB_DEN_R |= 0x07;      // enable digital I/O on PB2-0
  GPIO_PORTB_DR8R_R |= 0x07;        // enable 8 mA drive on PB2-0
}

// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by lenght of ISR
// Output: none
void Sound_Init(unsigned long period){
  Index = 0;
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1      
  NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC|NVIC_ST_CTRL_INTEN;  // enable SysTick with core clock and interrupts
}

void Sound_Stop(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
}

void Sound_Start(unsigned long period)
{
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;
}

// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
  GPIO_PORTF_DATA_R ^= 0x08;     // toggle PF3, debugging
  Index = (Index+1)&0x0F;        // 16 samples for each period
	// Index = (Index+1)%16;
	DAC = SineWave[Index]; // output to DAC: 3-bit data
}
