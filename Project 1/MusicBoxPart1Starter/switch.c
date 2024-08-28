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
}

// ISR for PORTF
void GPIOPortF_Handler(void){
}

