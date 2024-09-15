// CECS447Project2MCU1.c
// Runs on TM4C123
// Starter file for CEC447 Project 2 UART MCU1
// Min He
// September 26, 2023

#include "PLL.h"
#include "UART0.h"
#include "UART2.h"
#include "LEDSW.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

#define MAX_STR_LEN 20

// TODO: define bit values for all Colors 
#define RED 		()
#define BLUE 		()
#define GREEN 	()
#define PURPLE 	()
#define WHITE 	()
#define DARK 		()
#define CRAN 		()

// TODO: define all colors in the color wheel
const	uint8_t color_wheel[] = {};

// TODO: define bit addresses for the onboard three LEDs and two switches
#define LEDs 		()
#define SW1			()
#define SW2			()
	
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);
extern void DisableInterrupts(void);
void Mode1(void);
void Mode2(void);
void Mode3(void);
void Display_Menu(void);
	
bool end_of_str = false;
uint8_t string[MAX_STR_LEN];
uint8_t str_idx = 0;

int main(void){
	DisableInterrupts();
  PLL_Init();
  UART0_Init();  // for PC<->MCU1
	//UART2_Init();  // for MCU1<->MCU2
	LEDSW_Init();  // Initialize the onboard three LEDs and two push buttons
	EnableInterrupts();

  while(1){
		// displays the main menu 
		Display_Menu(); 
		switch(UART0_InChar()){
			case '1':
				Mode1();
				break;
			case '2':
				Mode2();
				break;
			case '3': 
				Mode3();
				break;
			default:
				break; 
		}
		UART0_OutCRLF();
  }
}

// Take care of Rx interrupt and ignore Tx interrupt
void UART0_Handler(void){
	uint8_t chr;
	
  if(UART0_RIS_R&UART_RIS_RXRIS){       // received one item
		if ((UART0_FR_R&UART_FR_RXFE) == 0) {
			chr = UART0_DR_R&0xFF;
			if (chr==CR){  // reach end of the string
				end_of_str=true;
		    string[str_idx]=NULL;  // add null terminator to end a C string.
			}
			else if (str_idx<(MAX_STR_LEN-1)){  // save one spot for C null terminator '\0'.
				if (chr==BS) {
          UART0_OutChar(BS);
					str_idx--;
				}
				else {
			    string[str_idx++]=chr;  // add the latest received symbol to end a C string.
					UART0_OutChar(chr);
			  }
			}
	  }
    UART0_ICR_R = UART_ICR_RXIC;        // acknowledge RX FIFO
  }
}


void Display_Menu(void){
	UART0_OutString((uint8_t *)"Welcome to CECS 447 Project 2 - UART");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"MCU1");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Main Menu");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"1. PC<->MCU1 LED Control");
	UART0_OutCRLF();
	//Will add other modes later
	UART0_OutString((uint8_t *)"Please choose a communication mode");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"(Only Mode 1 For now...)");
	while(!end_of_str){
		WaitForInterrupt();
	}
	end_of_str = false;
	str_idx = 0;
	UART0_OutCRLF();
}



void Mode1(void){
}

void Mode2(void){
}

void Mode3(void){
}