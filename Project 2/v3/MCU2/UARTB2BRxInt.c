// CECS 447 UART Project
// Board to Board and Board to PC UART Communication
// Runs on TM4C123 
// By Oliver Cabral and Jason Chan

// board to board communitation use UART1
// Ground connected ground in the USB cable
#include "tm4c123gh6pm.h"
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
volatile uint8_t COLOR_INDEX = 0;
volatile uint8_t MODE = 0;
uint8_t QUIT_CHAT = 0;

bool in_end_of_str = false;
uint8_t in_string[MAX_STR_LEN];
volatile uint8_t in_str_idx = 0;

extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  
extern void WaitForInterrupt(void);

void GPIO_PortF_Init(void);

void Mode2();
void Mode3();
void Main_Menu();
void Mode2_MenuTX();
void Mode2_MenuRX();

char* LEDtoSTR();
int LEDtoINDEX(void);

int main(void){	
  DisableInterrupts();
	PLL_Init();
	UART0_Init(true, false);	
  UART3_Init(true,false);   // initialize UART with RX interrupt
	GPIO_PortF_Init();			  // initialize port F
  EnableInterrupts();       // needed for TExaS
	LED = color_wheel[COLOR_INDEX];
	UART0_OutCRLF();
  while(1){
		//LED = UART3_InChar();
		Main_Menu();
		in_str_idx=0;
		in_end_of_str=false;
		while(!in_end_of_str){
			WaitForInterrupt();
		}
		in_end_of_str=false;
		in_str_idx=0;
		UART0_OutCRLF();
		switch(in_string[0]){
			case '2':
				MODE=2;
				Mode2();
				break;
			case '3':
				MODE=3;
				QUIT_CHAT = 0;
				Mode3();
				break;
			default:
				UART0_OutCRLF();
				break;
		}
		WaitForInterrupt();
  }
}
void Mode2(){
	Mode2_MenuRX();
}
void Mode3(){
	UART0_OutString((uint8_t *)"Mode 3 MCU1: Chat Room ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Press sw1 at any time to exit the chat room. ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Waiting for message from MCU 1");
	UART0_OutCRLF();
	while(!QUIT_CHAT){
		//Waiting For Message
		in_end_of_str=false;
		in_str_idx=0;
		while((!in_end_of_str) & (!QUIT_CHAT)){
			WaitForInterrupt();
		}
		if(!QUIT_CHAT){
			UART0_OutString((uint8_t *) "MCU 1: ");
			UART0_OutString(in_string);
			UART0_OutCRLF();
		}
		if(QUIT_CHAT){
			return;
		}
		//Send Message
		end_of_str = false;
		str_idx = 0;
		UART0_OutString((uint8_t *)"MCU 2: ");
		while((!end_of_str)&(!QUIT_CHAT)){
			WaitForInterrupt();
		}
		if(!QUIT_CHAT){
			UART3_OutString(string);
			UART3_OutChar(CR);
			UART0_OutCRLF();
		}
		if(QUIT_CHAT){
			return;
		}
	}
}

void Main_Menu(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "Welcome to CECS 447 Project 2 - UART");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "MCU2");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "Waiting for command from MCU1...");
	UART0_OutCRLF();
}
void Mode2_MenuRX(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "Mode 2 MCU2");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *) "Waiting for color code from MCU1");
	UART0_OutCRLF();
}
void Mode2_MenuTX(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Mode 2 MCU2");
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

void GPIO_PortF_Init(void)
{
 SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // (a) activate clock for port F
	while ((SYSCTL_RCGC2_R &= SYSCTL_RCGC2_GPIOF)==0){};
		
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x1F;           // allow changes to PF4,1,0       
  GPIO_PORTF_DIR_R |= 0x0E;          // (c) make LEDs outputs
  GPIO_PORTF_DIR_R &= ~0x11;
  GPIO_PORTF_AFSEL_R &= ~0x1F;       //     disable alt funct 
  GPIO_PORTF_DEN_R |= 0x1F;          //     enable digital   
  GPIO_PORTF_PCTL_R &= ~0x000FFFFF; // configure as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x1F;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF0,4
		
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF0,4 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF0,4 is not both edges
  GPIO_PORTF_IEV_R |= 0x11;    //     PF4 rising edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4
  NVIC_PRI7_R = (NVIC_PRI7_R&~0x00E00000)|0x00800000; // (g) priority 4
  NVIC_EN0_R |= 0x40000000;      // (h) enable interrupt 30 in NVIC
}

void GPIOPortF_Handler(void)
{		
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
		}
	}
	
	if(GPIO_PORTF_RIS_R & SW1)
	{
		GPIO_PORTF_ICR_R = SW1;
		if(MODE==2){
			UART3_OutChar(color_wheel[COLOR_INDEX]);	
			//UART3_OutChar(CR);			
			Mode2_MenuRX();
		}
	}
}

void UART3_Handler(void){
  if(UART3_RIS_R&UART_RIS_RXRIS){       // received one item
		if ((UART3_FR_R&UART_FR_RXFE) == 0){
			uint8_t data = UART3_DR_R&0xFF;
			if(MODE==2){
				if(data=='^'){
					LED = DARK;
					COLOR_INDEX = 0;
					MODE=0;
					Main_Menu();
				}else{
					LED = data;
					COLOR_INDEX = LEDtoINDEX();
					Mode2_MenuTX();	
				}
			}
			else if((MODE==3) | (MODE==0)){
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
	}
  UART3_ICR_R = UART_ICR_RXIC;        // acknowledge RX FIFO
}

void UART0_Handler(void){
	uint8_t chr;
  if(UART0_RIS_R&UART_RIS_RXRIS){       // received one item
		if ((UART0_FR_R&UART_FR_RXFE) == 0) {
			chr = UART0_DR_R&0xFF;
			//UART0_OutChar(chr);
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

