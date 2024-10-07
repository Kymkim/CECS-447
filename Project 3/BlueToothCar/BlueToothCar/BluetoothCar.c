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
#include "PWM.h"
#include "GPIO.h"
#include <stdio.h>
#include <math.h>

#define SW1       0x10
#define SW2       0x01

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
void Delay(float seconds);

void Figure8();
void Circle();
void Square();
void Z();

//Flags
volatile uint8_t MODE = 0; //DEMO MODE = 0; FREE DRIVE MODE = 1

int main(void) {
	unsigned char control_symbol; // for bluetooth controlled LEDs
//  unsigned char str[30];      // for testing strings from Bluetooth

	UART_Init(); // Initialize UART1 and UART0
  PortF_Init(); // Initilize for the three onboard LEDs
	UART0_OutString((unsigned char *)">>> Welcome to Bluetooth Controlled LED App <<<\n\r");
  Car_Dir_Init();
	PWM_PB76_Init();
  PWM_PB76_Duty(START_SPEED, START_SPEED);
	
  // Bluetooth Controlled LEDs
	while(1) {
    control_symbol = UART1_InChar();
    UART0_OutChar(control_symbol);
		UART0_OutChar(CR);
    UART0_OutChar(LF);
		
		if(MODE == 0){	//DEMO MODE
			LED = Red;
			switch (control_symbol){
				case '8':	//Figure 8 and stop
					Figure8();
					break;
				case 'C':	//One Circle and stop
				case 'c':
					Circle();
					break; 
				case 'S':	//One Square and stop
				case 's':
					Square();
					break; 
				case 'Z': //Zigzag 4 segments and stop
				case'z': 
					Z();
					break; 
				default:
					break;
			}
		}else if(MODE == 1){  //FREE ROAM
			LED = Blue;
		}
	}
}

//Figure 8 Definitions
#define PI 3.14159265
#define LOOP_DURATION 2
#define HALF_LOOP_DURATION (LOOP_DURATION/2)
#define TIME_STEP 0.01

void Figure8(){
	float t;
	DIRECTION=FORWARD;
	PWM_PB76_Duty(0,0);
	PWM0_ENABLE_R |= 0x03;
	for(t=0; t < HALF_LOOP_DURATION; t+= TIME_STEP){
		float SPEED_LEFT = 0.5 * (1 + sin(2*PI*t/HALF_LOOP_DURATION));
		float SPEED_RIGHT = 1.0;
		PWM_PB76_Duty(16000*SPEED_LEFT, 16000*SPEED_RIGHT);
		Delay(0.01);
	}
	for(t=0; t < HALF_LOOP_DURATION; t+= TIME_STEP){
		float SPEED_LEFT = 1.0;
		float SPEED_RIGHT = 0.5 * (1 + sin(2*PI*t/HALF_LOOP_DURATION));
		PWM_PB76_Duty(16000*SPEED_LEFT, 16000*SPEED_RIGHT);
		Delay(0.01);
	}
	PWM_PB76_Duty(0,0);
	PWM0_ENABLE_R &= ~0x03;
}	

void Circle(){
	PWM_PB76_Duty(0,0);
	PWM0_ENABLE_R |= 0x03;
	DIRECTION=FORWARD;
	PWM_PB76_Duty(PERIOD*0.5,PERIOD*0.2);
	Delay(1);
	PWM_PB76_Duty(0,0);
	PWM0_ENABLE_R &= ~0x03;
}


void Square(){
	PWM0_ENABLE_R |= 0x03;
	PWM_PB76_Duty(PERIOD*0.9,PERIOD*0.9);
	for(int sides = 0; sides < 4; sides++){
		DIRECTION=FORWARD;
		Delay(1);
		DIRECTION=LEFTPIVOT;
		Delay(1);
	}
	PWM_PB76_Duty(0,0);
	PWM0_ENABLE_R &= ~0x03;
}

void Z(){
		PWM0_ENABLE_R |= 0x03;
	PWM_PB76_Duty(PERIOD*0.9,PERIOD*0.9);
	for(int sides = 0; sides < 4; sides++){
		DIRECTION=FORWARD;
		Delay(1);
		if(sides%2){
			DIRECTION=LEFTPIVOT;
		}else{
			DIRECTION=RIGHTPIVOT;
		}
		Delay(1);
	}
	PWM_PB76_Duty(0,0);
	PWM0_ENABLE_R &= ~0x03;
}

void Delay(float seconds){
	unsigned long volatile time;
  time = 727240*500/91;  // 0.25sec
	time *= (unsigned long)(seconds/0.25);	//Scales up to seconds
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
  while((UART1_FR_R&UART_FR_RXFE) != 0);
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
		MODE ^= 0x1;
	}
}