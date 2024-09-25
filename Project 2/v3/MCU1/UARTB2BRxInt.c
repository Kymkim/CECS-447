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
uint8_t str_idx = 0;
bool in_end_of_str = false;
uint8_t in_string[MAX_STR_LEN];
uint8_t in_str_idx = 0;;

volatile uint8_t COLOR_INDEX = 0;
volatile uint8_t MODE = 0;
volatile uint8_t BRIGHTNESS = 0;
uint8_t QUIT_CHAT = 0;
volatile uint8_t RECIEVE = 0;

extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  
extern void WaitForInterrupt(void);

void Main_Menu();
void Mode1_Menu();
void Mode1_ChangeLED();
void Mode1_ChangeBrightness();
void Mode2_Menu();
void Mode2_MenuTX();
void Mode2_MenuRX();
void Mode1(void);
void Mode2(void);
void Mode3(void);
uint32_t StringToDecimal(uint8_t str[]);

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
		if(MODE == 0){
			Main_Menu();
			str_idx=0;
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
					Mode1();
					break;
				case '2':
					MODE=2;
					Mode2();
					break;
				case '3':
					MODE=3;
					QUIT_CHAT=0;
					Mode3();
					break;
				default:
					UART0_OutCRLF();
					UART0_OutString((uint8_t *) "Invalid Input. Please choose from 1-3");
					UART0_OutCRLF();
					break;
			}
		}
  }
}

//Modes
void Mode1(void){
	uint8_t isDone;
	
	Mode1_Menu();
	str_idx=0;
	while(!end_of_str){
		WaitForInterrupt();
	}
	end_of_str = false;
	str_idx = 0;
	UART0_OutCRLF();
	switch(string[0]){
		case '1':
			Mode1_ChangeLED();
			Mode1();
			break;
		case '2':
			Mode1_ChangeBrightness();
			Mode1();
			break;
		case '3':
			UART0_OutCRLF();
			MODE=0;
			break;
		default:
			UART0_OutCRLF();
			UART0_OutString((uint8_t *) "Invalid Input. Please choose from 1-3");
			UART0_OutCRLF();
			Mode1();
			break;
	}
	DisableSysTick();
}
void Mode2(){
	UART2_OutChar('2');
	UART2_OutChar(CR);
	Mode2_MenuTX();
	while (!end_of_str) { // wait until the whole string is received.
		WaitForInterrupt();
	}
	end_of_str = false;
	str_idx = 0;
	LED = DARK;
	COLOR_INDEX = 0;
	UART2_OutChar('^');
	//UART2_OutChar(CR);
	MODE=0;
}
void Mode3(){
	UART2_OutChar('3');
	UART2_OutChar(CR);
	MODE=3;
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Mode 3 MCU 2: Chat Room ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Press sw1 at any time to exit the chat room. ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please type a message end with a return ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"(less than 20 characters): ");
	UART0_OutCRLF();
	while(!QUIT_CHAT){
		//Send Message
		UART0_OutString((uint8_t *)"MCU 1: ");
		end_of_str = false;
		str_idx = 0;
		while((!end_of_str)&(!QUIT_CHAT)){
			WaitForInterrupt();
		}
		if(!QUIT_CHAT){
			UART2_OutString(string);
			UART2_OutChar(CR);
			UART0_OutCRLF();
		}
		if(QUIT_CHAT){
			return;
		}
		//Waiting For Message
		in_end_of_str=false;
		in_str_idx=0;
		while((!in_end_of_str) & (!QUIT_CHAT)){
			WaitForInterrupt();
		}
		if(!QUIT_CHAT){
			UART0_OutString((uint8_t *) "MCU 2: ");
			UART0_OutString(in_string);
			UART0_OutCRLF();
		}
		if(QUIT_CHAT){
			return;
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
void Mode1_Menu(){
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
void Mode1_ChangeLED(){	
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
        LED = RED;
				COLOR_INDEX = 1;
				UART0_OutCRLF();
				break;
    case 'g':
        LED = GREEN;
				COLOR_INDEX = 2;
				UART0_OutCRLF();
				break;
    case 'b':
        LED = BLUE;
				COLOR_INDEX = 3;
				UART0_OutCRLF();
				break;
		case 'y':
        LED = YELLOW;
				COLOR_INDEX = 4;
				UART0_OutCRLF();
				break;
    case 'c':
        LED = CYAN;
				COLOR_INDEX = 5;
				UART0_OutCRLF();
				break;
    case 'p':
        LED = PURPLE;
				COLOR_INDEX = 6;
				UART0_OutCRLF();
				break;
    case 'w':
        LED = WHITE;
				COLOR_INDEX = 7;
				UART0_OutCRLF();
				break;
    case 'd':
        LED = DARK;
				COLOR_INDEX = 0;
				UART0_OutCRLF();
				break;
    default:
        UART0_OutString((uint8_t *)"Invalid Input. Please try again ");
				UART0_OutCRLF();
				Mode1_ChangeLED();
				break;
	}
}
void Mode1_ChangeBrightness(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please enter a decimal number from 0 to 100 followed by a return: ");
	while(!end_of_str){
		WaitForInterrupt();
	}
	end_of_str=false;
	str_idx=0;
	BRIGHTNESS = StringToDecimal(string);
	
	if(BRIGHTNESS == 100){
		LED = color_wheel[COLOR_INDEX];
		LOW = PERIOD - 1;
		HIGH = 0;
	}else if(BRIGHTNESS == 0){
		LED = DARK; //Turn off LED Brightness is 0
	}else{
		LED = color_wheel[COLOR_INDEX];
    HIGH = (BRIGHTNESS * PERIOD) / 100;
		LOW = PERIOD - HIGH;
	}
  UART0_OutCRLF();
}
uint32_t StringToDecimal(uint8_t str[]){
	uint32_t number=0;;
  uint8_t character,idx=0;

  character = str[idx];
  while(character != NULL){
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 2^32-1
    }
    else { // if not a decimal number, return the result
            return number;
    }
    character = str[++idx]; // get the next digit
  }
  return number;
}

//Interrupt Handlers
void SysTick_Handler(void) {
		NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
    // Toggle the LED state based on the PWM duty cycle
    if(LED&color_wheel[COLOR_INDEX]){
			NVIC_ST_RELOAD_R = LOW - 1;
			LED &= ~color_wheel[COLOR_INDEX];
		}else{
			NVIC_ST_RELOAD_R = HIGH - 1;
			LED |= color_wheel[COLOR_INDEX]; 
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
			//UART2_OutChar(CR);
			Mode2_MenuRX();
		}
		if(MODE==3){
			QUIT_CHAT=1;
			MODE=0;
			UART2_OutChar(0xFF);
			UART2_OutChar(CR); //Send Command
		}
	}
}

void UART2_Handler(void){
  if(UART2_RIS_R&UART_RIS_RXRIS){       // received one item
		if ((UART2_FR_R&UART_FR_RXFE) == 0){
			uint8_t data = UART2_DR_R&0xFF;
			if(MODE==2){
				LED = data;
				COLOR_INDEX = LEDtoINDEX();
				Mode2_MenuTX();
			}
			if((MODE==3) | (MODE==0)){
				if(data==0xFF){
					MODE = 0;
					QUIT_CHAT=1;
				}
				if(data==CR){
					in_end_of_str = true;
					in_string[in_str_idx] = NULL;
					//in_str_idx = 0;
				}else if (in_str_idx < (MAX_STR_LEN-1)){
					if(data==BS){
						//UART3_OutChar(BS);
						in_str_idx--;
					}else{
						in_string[in_str_idx++] = data;
					}
				}
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
