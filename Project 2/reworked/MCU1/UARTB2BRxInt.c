// main.c: example program for testing MCU to MCU communication
// Runs on TM4C123 
// By Dr.Min He

// board to board communitation use UART1
// Ground connected ground in the USB cable
#include "tm4c123gh6pm.h"
#include "LEDSW.h"
#include "UART.h"
#include "USBUART.h"
#include "PLL.h"

#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

// bit address definition for port data registers
#define LED 		(*((volatile uint32_t *)0x40025038))

////////// Constants //////////  
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// white    RGB    0x0E
// pink     R-B    0x06
// Cran     -GB    0x0C

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

#define SW1       0x10
#define SW2       0x01
#define NVIC_EN0_PORTF 0x40000000

#define MAX_STR_LEN 20

bool end_of_str = false;
uint8_t string[MAX_STR_LEN];
uint8_t str_idx = 0;;
volatile uint8_t COLOR_INDEX = 0;
volatile uint8_t MODE = 0;

extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  
extern void WaitForInterrupt(void);

void Main_Menu();
void Mode2_Menu();
void Mode2_MenuTX();
void Mode2_MenuRX();
void Mode2();

char * LEDtoSTR();
int LEDtoINDEX(void);

int main(void){	
  DisableInterrupts();
	PLL_Init();
	UART0_Init(true, false);	
  UART2_Init(true,false);   // initialize UART with RX interrupt
	//GPIO_PortF_Init();			  // initialize port F
	LEDSW_Init();
	DisableSysTick();
  EnableInterrupts();       // needed for TExaS
	LED = color_wheel[COLOR_INDEX];
  while(1){
		Main_Menu();
		while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}
		end_of_str = false;
		str_idx = 0;
		UART0_OutCRLF();
		switch(string[str_idx]){
			case '1':
				MODE=1;
				EnableSysTick();
				UART0_OutString((uint8_t *)"Will Add Later :)");
				UART0_OutCRLF();
				break;
			case '2':
				MODE=2;
				Mode2();
			default:
				break;
		}
		WaitForInterrupt();
  }
}

//Modes
void Mode2(){
	UART2_OutChar('2');
	while(1){
		Mode2_MenuTX();
		while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}
		end_of_str = false;
		str_idx = 0;
		if(string[0] == '^'){
			UART2_OutChar('^');
			MODE=0;
		}
	}
}

//Menus
void Main_Menu(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Welcome to CECS 447 Project 2 - UART");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"MCU1");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Main Menu");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"1. PC <-> MCU1 LED Control");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"2. MCU1 <-> MCU2 Color Wheel");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"3. PC1 <-> MCU1 <-> MCU2 <-> PC2 Chat Room");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please choose a communication mode (enter 1 or 2 or 3)");
	UART0_OutCRLF();
}	
void Mode2_MenuRX(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "Mode 2 MCU1: press ^ to exit");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "Waiting for color code from MCU1");
	UART0_OutCRLF();
}
void Mode2_MenuTX(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Mode 2 MCU1: press ^ to exit this mode");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"In color wheel state");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please press SW2 to go through the colors in the following color wheel: Dark, Red, Green, Blue, Yellow, Cyan, Purple, White.");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Once a color is selected, press sw1 to send the color to MCU2");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Current color: ");
	UART0_OutString((uint8_t *)LEDtoSTR());
	UART0_OutCRLF();
}


//Helper Functions
char * LEDtoSTR(void){
	switch(LED){
		case(DARK): return "DARK"; break;
		case(RED): return "RED"; break;
		case(GREEN): return "GREEN"; break;
		case(BLUE): return "BLUE"; break;
		case(YELLOW): return "YELLOW"; break;
		case(CYAN): return "CYAN"; break;
		case(PURPLE): return "PURPLE"; break;
		case(WHITE): return "WHITE"; break;
		default: return "NULL"; break;
	}
}
int LEDtoINDEX(void){
	switch(LED){
		case(DARK): return 0; break;
		case(RED): return 1; break;
		case(GREEN): return 2; break;
		case(BLUE): return 3; break;
		case(YELLOW): return 4; break;
		case(CYAN): return 5; break;
		case(PURPLE): return 6; break;
		case(WHITE): return 7; break;
		default: return 0; break;
	}
}

//Interrupt Handlers
void SysTick_Handler(void) {
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
    // Toggle the LED state based on the PWM duty cycle
    if(LED&color_wheel[COLOR_INDEX]){
			NVIC_ST_RELOAD_R = LOW - 1;
			LED &= ~color_wheel[COLOR_INDEX]; // AREA
		}else{
			NVIC_ST_RELOAD_R = HIGH - 1;
			LED |= color_wheel[COLOR_INDEX]; // AREA 
		}

    NVIC_ST_CURRENT_R = 0; // Reset the current value of the SysTick timer
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE; // Restart SysTick timer
}
void GPIOPortF_Handler(void){		
	// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<80000;time++) {}
	
  if(GPIO_PORTF_RIS_R & SW2)
	{
		GPIO_PORTF_ICR_R = SW2;
		if(MODE==2){
			if((COLOR_INDEX+1) > 7){
				COLOR_INDEX = 0;
			}else{
				COLOR_INDEX += 1;
			}
			LED = color_wheel[COLOR_INDEX];
			//Mode2_Menu();
		}
	}
	
	if(GPIO_PORTF_RIS_R & SW1)
	{
		GPIO_PORTF_ICR_R = SW1;
		if(MODE==2){
			UART2_OutChar(color_wheel[COLOR_INDEX]);	
			Mode2_MenuRX();
		}
	}
}
void UART2_Handler(void){
  if(UART2_RIS_R&UART_RIS_RXRIS){       // received one item
		if ((UART2_FR_R&UART_FR_RXFE) == 0){
			if(MODE==2){
				LED = UART2_DR_R&0xFF;
				COLOR_INDEX = LEDtoINDEX();
				Mode2_MenuTX();
			}
		}
    UART2_ICR_R = UART_ICR_RXIC;        // acknowledge RX FIFO
  }
}
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
