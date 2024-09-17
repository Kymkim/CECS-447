// CECS447Project2MCU1.c
// Runs on TM4C123
// Starter file for CEC447 Project 2 UART MCU1
// Min He
// September 26, 2023

#include "PLL.h"
#include "UART0.h"
#include "UART3.h"
#include "LEDSW.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

#define MAX_STR_LEN 20

												//GBRx
#define RED 		(0x02)	//0010
#define BLUE 		(0x04)	//0100
#define GREEN 	(0x08)	//1000
#define PURPLE 	(0x06)	//0110
#define CYAN 		(0x0C)	//1100
#define YELLOW  (0x0A)	//1010
#define WHITE 	(0x0E)	//1110
#define DARK 		(0X00)	//0000


// TODO: define all colors in the color wheel
const	uint8_t color_wheel[] = {DARK, RED, GREEN, BLUE, YELLOW, CYAN, PURPLE, WHITE};
	
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);
extern void DisableInterrupts(void);
void Beginning_Prompt(void);

void Mode1(void);

void Mode2(void);

void Mode3(void);

	
#define LEDs 		(*((volatile uint32_t *)0x40025038))
		
	
bool end_of_str = false;
uint8_t string[MAX_STR_LEN];
uint8_t str_idx = 0;
uint8_t brightness = 0;
uint8_t curr_col_index = 0;

int main(void){
	DisableInterrupts();
  PLL_Init();
  UART0_Init(true,false);  // for PC<->MCU1
	UART3_Init(true,false);  // for MCU1<->MCU2
	LEDSW_Init();  // Initialize the onboard three LEDs and two push buttons
	EnableInterrupts();
	
	Beginning_Prompt();
  while(1){
		switch(UART3_InChar()){
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

void Beginning_Prompt(void){
	UART0_OutString((uint8_t *)"Welcome to CECS 447 Project 2 - UART");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"MCU1");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Main Menu");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"1. PC<->MCU1 LED Control ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"2. MCU1<->MCU2 Color Wheel. ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"3. PC<->MCU1<->MCU2<->PC Chat Room ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please choose a communication mode ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"(Please choose from 1, 2 ,3)");
}
