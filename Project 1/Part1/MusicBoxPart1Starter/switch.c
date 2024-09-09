// switch.c
// Runs on TM4C123
// Provide functions that initialize the onboard push buttons
// Author: Min He
// August 26, 2022

#include "tm4c123gh6pm.h"
#include "switch.h"
#include "music.h"

// Subroutine to initialize port F pins for the two onboard switches
// enable PF4 and PF0 for SW1 and SW2 respectively with falling edge interrupt enabled.
// Inputs: None
// Outputs: None
void Switch_Init(void){ 
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // activate port F
	while ((SYSCTL_RCGCGPIO_R&SYSCTL_RCGCGPIO_R5)!=SYSCTL_RCGCGPIO_R5){}

 // GPIO_PORTA_DIR_R &= ~ 0x11;             // make PF0 out (built-in LED)
  //GPIO_PORTA_AFSEL_R &= ~0x11;          // disable alt funct on PF1
  //GPIO_PORTA_DEN_R |= 0x11;             // enable digital I/O on PF1
  //GPIO_PORTA_PCTL_R &= ~0x000F000F;     // configure PF0 to be GPIO
  //GPIO_PORTA_AMSEL_R &= ~ 0x11;               // disable analog functionality on PF	
		
		
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // unlock GPIO Port F
  GPIO_PORTF_CR_R |= 0x1F;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R &= ~0x11;    // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x11;     //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R &= ~0x000F000F; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00400000; // (g) bits:23-21 for PORTF, set priority to 5
  NVIC_EN0_R |= 0x40000000;      // (h) enable interrupt 30 in NVIC
}

// ISR for PORTF
void GPIOPortF_Handler(void){
	if(GPIO_PORTF_RIS_R&0x10){ //Mode toggle SW1
		GPIO_PORTF_ICR_R = 0x10;
		if(is_music_on()){
			turn_off_music();
		}else{
			turn_on_music();
		}
	}else{ //Activation
		GPIO_PORTF_ICR_R = 0x01;
		next_song();
	}
}

