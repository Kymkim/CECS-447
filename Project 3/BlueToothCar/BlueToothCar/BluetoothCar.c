// BLTControlledLEDs.c
// Runs on LM4F120/TM4C123
// This is an example program which shows how to use Bluetooth to received commands from a Bluetooth terminal
// and use the received command to control the onboard LEDs.
// U1Rx (PB0)
// U1Tx (PB1)
// Ground connected ground in the USB cable


// Header files 
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "Motors.h"
#include "PLL.h"
#include <stdio.h>
#include <math.h>

#define SW1       0x10
#define SW2       0x01
#define LED (*((volatile unsigned long *)0x40025038)) 

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// white    RGB    0x0E
// pink     R-B    0x06

#define Dark    	0x00
#define Red     	0x02
#define Blue    	0x04
#define Green   	0x08
#define Yellow  	0x0A
#define White   	0x0E
#define Purple  	0x06

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A

void UART_Init(void);
unsigned char UART1_InChar(void);
void BLT_InString(unsigned char *bufPt);
void UART0_OutChar(unsigned char data);
void UART0_OutString(unsigned char *pt);
void PortF_Init(void);
void Delay();

void Figure8();
void Circle();
void Square();
void Z();

uint16_t speed = 100;

unsigned char control_symbol; 

//Flags
volatile uint8_t MODE = 1; 
volatile uint8_t MODE_SW = 1;

int main(void) {
	// for bluetooth controlled LEDs
//  unsigned char str[30];      // for testing strings from Bluetooth

	UART_Init(); // Initialize UART1 and UART0
  PortF_Init(); // Initilize for the three onboard LEDs
	Wheels_PWM_Init();
	PLL_Init();
	Dir_Init();
	
  // Bluetooth Controlled LEDs
	while(1) {
			LED = Red;
			control_symbol = UART1_InChar();
			switch (control_symbol){
				case '8':	//Figure 8 and stop
					if(MODE==1) Figure8();
					break;
				case 'C':	//One Circle and stop
				case 'c':
					if(MODE==1)Circle();
					break; 
				case 'S':	//One Square and stop
				case 's':
					if(MODE==1){
						Square();
					}else{
						Stop_Both_Wheels();
					}
					break; 
				case 'Z': //Zigzag 4 segments and stop
				case'z': 
					if(MODE==1) Z();
					break; 
				case 'F':
				case 'f'://Figure 8 and stop
					if(MODE==2){
						Start_Both_Wheels();
						SetSpeed(speed,speed);
						DIRECTION=FORWARD;
					}
					break;
				case 'B':
				case 'b'://One Circle and stop
					if(MODE==2){
						Start_Both_Wheels();
						SetSpeed(speed,speed);
						DIRECTION=BACKWARD;
					}
					break;
				case 'L':
				case 'l':
					if(MODE==2){
						Start_Both_Wheels();
						SetSpeed(speed,speed);
						DIRECTION=LEFTPIVOT;
					}
					break; 
				case 'R':
				case 'r':					//One Square and stop
					if(MODE==2){
						Start_Both_Wheels();
						SetSpeed(speed,speed);
						DIRECTION=RIGHTPIVOT;
					}
					break;
				case 'U': //Zigzag 4 segments and stop
				case'u':
					if(MODE==2){
						if(speed < PERIOD){
							speed += 100;
						}
						SetSpeed(speed,speed);
					}
					break; 
				case 'D': //Zigzag 4 segments and stop
				case'd': 
					if(MODE==2){
						if(speed > 0){
							speed -= 100;
						}
						SetSpeed(speed,speed);
					}
					break;
				case 'X': //Zigzag 4 segments and stop
				case'x': 
					if(MODE==1){
						MODE=2;
					}else{
						MODE=1;
					}
					break;
				default:
					break;
			}
	}
}

//Figure 8 Definitions
#define PI 3.14159265
#define LOOP_DURATION 2
#define HALF_LOOP_DURATION (LOOP_DURATION/2)
#define TIME_STEP 0.01

void Figure8(){
	SetSpeed(0,0);
	Start_Both_Wheels();
	DIRECTION=FORWARD;
	SetSpeed(PERIOD*0.5,PERIOD*0.2);
	for (int i = 0; i < 26; i++) {Delay();}
	SetSpeed(PERIOD*0.2,PERIOD*0.5);
	for (int i = 0; i < 26; i++) {Delay();}
	SetSpeed(0,0);
	Stop_Both_Wheels();
}	

void Circle(){
	SetSpeed(0,0);
	Start_Both_Wheels();
	DIRECTION=FORWARD;
	SetSpeed(PERIOD*0.5,PERIOD*0.2);
	for (int i = 0; i < 26; i++) {Delay();}
	SetSpeed(0,0);
	Stop_Both_Wheels();
}


void Square(){
	Start_Both_Wheels();
	for(int sides = 0; sides < 4; sides++){
		SetSpeed(PERIOD*0.5,PERIOD*0.5);
		DIRECTION=FORWARD;
		for (int i = 0; i < 10; i++) {Delay();}
		SetSpeed(PERIOD*0.125,PERIOD*0.125);
		DIRECTION=LEFTPIVOT;
		for (int i = 0; i < 10; i++) {Delay();}
	}
	SetSpeed(0,0);
	Stop_Both_Wheels();
}

void Z(){
	Start_Both_Wheels();
	SetSpeed(PERIOD*0.5,PERIOD*0.5);
	for(int sides = 0; sides < 4; sides++){
		SetSpeed(PERIOD*0.5,PERIOD*0.5);
		DIRECTION=FORWARD;
		for (int i = 0; i < 10; i++) {Delay();}
		if(sides%2){
			SetSpeed(PERIOD*0.125,PERIOD*0.125);
			DIRECTION=LEFTPIVOT;
			for (int i = 0; i < 12; i++) {Delay();}
		}else{
			SetSpeed(PERIOD*0.125,PERIOD*0.125);
			DIRECTION=RIGHTPIVOT;
			for (int i = 0; i < 12; i++) {Delay();}
		}
	}
	SetSpeed(0,0);
	Stop_Both_Wheels();
}



void Delay(void){
	uint32_t volatile time;
  time = 727240*20/91;  // 0.1sec for 16MHz
//  time = 727240*100/91;  // 0.1sec for 80MHz
  while(time){
		time--;
  }
}

//------------UART_Init------------
// Initialize the UART for 19200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
	// Activate Clocks
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // activate UART0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A
	
	
	UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 17;                    // IBRD = int(16,000,000 / (16 * 57600)) = int(17.3611111)
  UART0_FBRD_R = 23;                     // FBRD = round(3611111 * 64) = 27
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= 0x301;                 // enable UART for both Rx and Tx

  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1,PA0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1,PA0
                                        // configure PA1,PA0 as UART0
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA1,PA0
	
  UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
	
	// Data Communication Mode, Buad Rate = 57600
  UART1_IBRD_R = 17;                    // IBRD = int(16,000,000 / (16 * 57600)) = int(17.3611111)
  UART1_FBRD_R = 23;                     // FBRD = round(3611111 * 64) = 27
	
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART1_CTL_R |= 0x301;                 // enable UART for both Rx and Tx
  
  GPIO_PORTB_AFSEL_R |= 0x03;           // enable alt funct on PB1,PB0
  GPIO_PORTB_DEN_R |= 0x03;             // enable digital I/O on PB1,PB0
                                        // configure PB1,PB0 as UART1
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTB_AMSEL_R &= ~0x03;          // disable analog functionality on PB1,PB0

}

//------------UART0_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART0_OutChar(unsigned char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}

//------------UART0_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART0_OutString(unsigned char *pt){
  while(*pt){
    UART0_OutChar(*pt);
    pt++;
  }
}

//------------UART1_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART1_InChar(void){
  while(((UART1_FR_R&UART_FR_RXFE) != 0) & MODE_SW){}
	MODE_SW = 1;
  return((unsigned char)(UART1_DR_R&0xFF));	
}

// This function reads response from HC-05 Bluetooth module.
void BLT_InString(unsigned char *bufPt) {
  unsigned char length=0;
  bufPt[length] = UART1_InChar();
  
  // Two possible endings for a reply from HC-05: OK\r\n, ERROR:(0)\r\n
  while (bufPt[length]!=LF) {
    length++;
    bufPt[length] = UART1_InChar();
  };
    
  // add null terminator
  length++;
  bufPt[length] = 0;
}

// Port F Initialization
void PortF_Init(void){
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

void GPIOPortF_Handler(void){
		// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<80000;time++) {}

	if(GPIO_PORTF_RIS_R & SW1)
	{
		GPIO_PORTF_ICR_R = SW1;
		if(MODE==1){
			MODE=2;
			MODE_SW = 0;
			LED = Red;
		}else{
			MODE=1;
			MODE_SW = 0;
			LED = Dark;
			speed=100;
		}
	}
}