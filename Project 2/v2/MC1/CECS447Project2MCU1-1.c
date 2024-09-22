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
void Mode1(void);
void Mode2(void);
void Mode3(void);
void Display_Menu(void);
void Mode1_Menu(void);
void ChangeLEDColor();
void ChangeBrightness();
uint32_t Str_to_UDec(uint8_t str[]);
	
#define LEDs 		(*((volatile uint32_t *)0x40025038))
#define SW1       0x10
#define SW2       0x01
		
	
bool end_of_str = false;
uint8_t string[MAX_STR_LEN];
uint8_t str_idx = 0;
uint8_t brightness = 0;
uint8_t curr_col_index = 0;
uint8_t curr_mode = 0;

int main(void){
	DisableInterrupts();
  PLL_Init();
  UART0_Init(true,false);  // for PC<->MCU1
	UART2_Init(true,false);  // for MCU1<->MCU22
	LEDSW_Init();  // Initialize the onboard three LEDs and two push buttons
	EnableInterrupts();
	
  while(1){
		// displays the main menu 
		//UART0_OutChar(UART2_InChar());
		Display_Menu(); 
		while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}
		end_of_str = false;
		str_idx = 0;
		UART0_OutCRLF();
		switch(string[0]){
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

void UART2_Handler(void){
	if(UART2_RIS_R & UART_RIS_RXRIS){
		if ((UART2_FR_R & UART_FR_RXFE) == 0){
			LEDs = UART2_DR_R;
		}
		UART2_ICR_R = UART_ICR_RXIC;
	}
}


void Display_Menu(void){
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

void Mode1_Menu(void){
	UART0_OutString((uint8_t *)"Mode 1 Menu");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please select an option from the following list (enter 1 or 2 or 3)");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"1. Choose an LED color");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"2. Change the brightness of current LED(s)");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"3. Exit");
	UART0_OutCRLF();
}


void Mode1(void){
	UART0_OutCRLF();
	Mode1_Menu();
	while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}
	end_of_str = false;
  str_idx = 0;
	UART0_OutCRLF();
	switch(string[0]){
		//Option 1
		case '1':
			ChangeLEDColor();
			Mode1(); //Go Back to Menu
			break;
		case '2':
			ChangeBrightness();
			Mode1();
			break;
		case '3':
			UART0_OutCRLF();
			break;
		default:
			UART0_OutCRLF();
			UART0_OutString((uint8_t *)"Invalid Input. Please try again!");
			UART0_OutCRLF();
			Mode1();
			break;
	}
}

void ChangeLEDColor(){
	UART0_OutCRLF();
	end_of_str = false;
  str_idx = 0;
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please select a color from the following list: ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"d(dark),r(red),g(green),b(blue),y(yellow),c(cran),p(purple),w(white): ");
	while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}
	end_of_str = false;
  str_idx = 0;
  UART0_OutCRLF();
  switch (string[0]) {
    case 'r':
        LEDs = RED;
				curr_col_index = 1;
        break;

    case 'g':
        LEDs = GREEN;
				curr_col_index = 2;
        break;

    case 'b':
        LEDs = BLUE;
				curr_col_index = 3;
        break;
		
		case 'y':
        LEDs = YELLOW;
				curr_col_index = 4;
        break;
		
    case 'c':
        LEDs = CYAN;
				curr_col_index = 5;
        break;

    case 'p':
        LEDs = PURPLE;
				curr_col_index = 6;
        break;

    case 'w':
        LEDs = WHITE;
				curr_col_index = 7;
        break;

    case 'd':
        LEDs = DARK;
				curr_col_index = 0;
        break;

    default:
        UART0_OutString((uint8_t *)"Invalid Input. Please try again ");
        return ChangeLEDColor();
	}
}

void ChangeBrightness(){
	UART0_OutCRLF();
  UART0_OutString((uint8_t *)"Please enter a decimal number from 0 to 100 followed by a return: ");

  while (!end_of_str) {
		WaitForInterrupt();
  }
  end_of_str = false;
  str_idx = 0;

  brightness = Str_to_UDec(string);
		
	if(brightness == 100){
		LEDs = color_wheel[curr_col_index];
		LOW = PERIOD - 1;
		HIGH = 0;
	}else if(brightness == 0){
		LEDs = DARK; //Turn off LED Brightness is 0
	}else{
		LEDs = color_wheel[curr_col_index];
    HIGH = (brightness * PERIOD) / 100;
		LOW = PERIOD - HIGH;
	}
		
  UART0_OutCRLF();
}

void Mode2_Menu(void){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Mode 2 MCU1: press ^ to exit this mode ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"In color Wheel State.  ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please press sw2 to go through the colors in  ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"the following color wheel: Dark, Red, Green, ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Blue, Yellow, Cran, Purple, White. ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Once a color is selected, press sw1 to send  ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"the color to MCU2.");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Current color:  ");
}

void Mode2(void){
	curr_mode = 2;
	UART2_OutChar('2');
	Mode2_Menu();
	UART0_OutString((uint8_t *)"Current color - not yet implemented");
	UART0_OutCRLF();
	while(!end_of_str){
		WaitForInterrupt();
	}
	end_of_str=false;
	curr_col_index=0;
	UART0_OutCRLF();
}

void GPIOPortF_Handler(void)
{		
	// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<80000;time++) {}
	
  if(GPIO_PORTF_RIS_R & SW2)
	{
		GPIO_PORTF_ICR_R = SW2;
		curr_col_index = (curr_col_index+1)%8;
		LEDs = color_wheel[curr_col_index];
	}
	
	if(GPIO_PORTF_RIS_R & SW1)
	{
		GPIO_PORTF_ICR_R = SW1;
		if(curr_mode == 2){
			UART2_OutChar(LEDs);
			UART0_OutCRLF();
			UART0_OutString((uint8_t *)"Mode 2 MCU1: press ^ to exit this mode ");
			UART0_OutCRLF();
			UART0_OutString((uint8_t *)"Current color: Not Yet Implemented ");
			
			//Show Current Color ToDo Later
			
			UART0_OutCRLF();
			UART0_OutString((uint8_t *)"Waiting for color code from MCU2  ");
			UART0_OutCRLF();		
		}
	}
}

void Mode3(void){
}

uint32_t Str_to_UDec(uint8_t str[]){
    uint32_t number=0;;
    uint8_t character,idx=0;

  character = str[idx];
  while(character != NULL){
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 2^32-1
    }
    else { // none decimal digit fond, stop converting and return previous digits
            return number;
    }
    character = str[++idx]; // get the next digit
  }
  return number;
}

void SysTick_Handler(void) {
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
    // Toggle the LED state based on the PWM duty cycle
    if(LEDs&color_wheel[curr_col_index]){
			NVIC_ST_RELOAD_R = LOW - 1;
			LEDs &= ~color_wheel[curr_col_index]; // AREA
		}else{
			NVIC_ST_RELOAD_R = HIGH - 1;
			LEDs |= color_wheel[curr_col_index]; // AREA 
		}

    NVIC_ST_CURRENT_R = 0; // Reset the current value of the SysTick timer
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE; // Restart SysTick timer
}