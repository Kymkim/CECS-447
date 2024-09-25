// board to board communication using UART 3
// Ground is connected to the ground in the USB cable
// By: Oliver Cabral and Jason Chan

#include "tm4c123gh6pm.h"
#include "UART.h"
#include "USBUART.h"
#include "PLL.h"

#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

// bit address definition for port data registers
#define LED (*((volatile uint32_t *)0x40025038))  // Control the LED at PortF

#define RED    (0x02)  // Binary 0010
#define BLUE   (0x04)  // Binary 0100
#define GREEN  (0x08)  // Binary 1000
#define PURPLE (0x06)  // Binary 0110
#define CYAN   (0x0C)  // Binary 1100
#define YELLOW (0x0A)  // Binary 1010
#define WHITE  (0x0E)  // Binary 1110
#define DARK   (0x00)  // Binary 0000

// Define color wheel for cycling through colors
const uint8_t color_wheel[] = {DARK, RED, GREEN, BLUE, YELLOW, CYAN, PURPLE, WHITE};

#define SW1 0x10    // Switch 1 bit mask
#define SW2 0x01    // Switch 2 bit mask
#define NVIC_EN0_PORTF 0x40000000  // Enable PortF interrupt in NVIC

#define MAX_STR_LEN 20  // Maximum length for input/output strings

bool end_of_str = false;  // To track the end of the string
uint8_t string[MAX_STR_LEN];  // Buffer to hold the output string
uint8_t str_idx = 0;  // Index for string buffer
volatile uint8_t COLOR_INDEX = 0;  // Current index in color wheel
volatile uint8_t MODE = 0;  // Current mode (2 or 3)
uint8_t QUIT_CHAT = 0;  // Flag to exit chat in mode 3

bool in_end_of_str = false;  // Track input string termination
uint8_t in_string[MAX_STR_LEN];  // Buffer for input string
volatile uint8_t in_str_idx = 0;  // Index for input string

extern void DisableInterrupts(void);  // Disable all interrupts
extern void EnableInterrupts(void);   // Enable all interrupts
extern void WaitForInterrupt(void);   // Wait for an interrupt

// Function Prototypes
void GPIO_PortF_Init(void);
void Mode2();
void Mode3();
void Main_Menu();
void Mode2_MenuTX();
void Mode2_MenuRX();

char* LEDtoSTR();  // Convert LED state to string
int LEDtoINDEX();  // Get index of current LED color

int main(void){	
  DisableInterrupts();  // Disable interrupts before initialization
	PLL_Init();  // Initialize Phase Locked Loop
	UART0_Init(true, false);  // Initialize UART0 (TX/RX not interrupt-driven)
  UART3_Init(true, false);  // Initialize UART3 with RX interrupt
	GPIO_PortF_Init();  // Initialize GPIO PortF for LED control
  EnableInterrupts();  // Enable interrupts
	LED = color_wheel[COLOR_INDEX];  // Set the initial LED color
	UART0_OutCRLF();  // Output a new line on UART0

  while(1){	
		Main_Menu();  // Display the main menu
		in_str_idx = 0;  // Reset input string index
		in_end_of_str = false;  // Reset input end flag
		
    // Wait for the input to complete
		while(!in_end_of_str){
			WaitForInterrupt();
		}

		in_end_of_str = false;
		in_str_idx = 0;
		UART0_OutCRLF();  // Output a new line

		// Handle the selected mode based on input
		switch(in_string[0]){
			case '2':
				MODE = 2;
				Mode2();  // Enter Mode 2 (color mode)
				break;
			case '3':
				MODE = 3;
				QUIT_CHAT = 0;
				Mode3();  // Enter Mode 3 (chat mode)
				break;
			default:
				UART0_OutCRLF();  // Invalid command, just print a new line
				break;
		}
		WaitForInterrupt();
  }
}

// Mode 2: Color control via UART
void Mode2(){
	Mode2_MenuRX();  // Display the Mode 2 RX menu
}

// Mode 3: Chat room mode via UART
void Mode3(){
	UART0_OutString((uint8_t *)"Mode 3 MCU 2: Chat Room ");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Press SW1 at any time to exit the chat room.");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Waiting for a message from MCU 1...");
	UART0_OutCRLF();

	while(!QUIT_CHAT){
		in_end_of_str = false;
		in_str_idx = 0;

		// Wait for a message or exit chat
		while((!in_end_of_str) && (!QUIT_CHAT)){
			WaitForInterrupt();
		}

		if(!QUIT_CHAT){
			UART0_OutString((uint8_t *)"MCU 1: ");
			UART0_OutString(in_string);  // Display the received message
			UART0_OutCRLF();
		}

		if(QUIT_CHAT){
			return;
		}

		// Send a message to MCU 1
		end_of_str = false;
		str_idx = 0;
		UART0_OutString((uint8_t *)"MCU 2: ");
		while((!end_of_str) && (!QUIT_CHAT)){
			WaitForInterrupt();
		}
		if(!QUIT_CHAT){
			UART3_OutString(string);  // Transmit the message via UART3
			UART3_OutChar(CR);  //Send indicator
			UART0_OutCRLF();
		}
		if(QUIT_CHAT){
			return;
		}
	}
}

// Main Menu: Display the welcome message
void Main_Menu(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Welcome to CECS 447 Project 2 - UART");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"MCU2");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Waiting for command from MCU1...");
	UART0_OutCRLF();
}

// Mode 2 RX Menu
void Mode2_MenuRX(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Mode 2 MCU2");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Waiting for color code from MCU1");
	UART0_OutCRLF();
}

// Mode 2 TX Menu: Prompt user to select a color
void Mode2_MenuTX(){
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Mode 2 MCU2");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"In color wheel state");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Please press SW2 to go through the colors...");
	UART0_OutCRLF();
	UART0_OutString((uint8_t *)"Current color: ");
	UART0_OutString((uint8_t *)LEDtoSTR());  // Display the current color
	UART0_OutCRLF();
}

// GPIO Port F Initialization: Set up Port F for LED and switch control
void GPIO_PortF_Init(void){
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;  // Activate clock for Port F
	while((SYSCTL_RCGC2_R &= SYSCTL_RCGC2_GPIOF) == 0){};  // Wait for clock to stabilize
		
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;  // Unlock PortF PF0
  GPIO_PORTF_CR_R |= 0x1F;  // Allow changes to PF4, PF0
  GPIO_PORTF_DIR_R |= 0x0E;  // Set PF1-PF3 as outputs (LEDs)
  GPIO_PORTF_DIR_R &= ~0x11;  // Set PF0, PF4 as inputs (switches)
  GPIO_PORTF_AFSEL_R &= ~0x1F;  // Disable alternate function
  GPIO_PORTF_DEN_R |= 0x1F;  // Enable digital functionality
  GPIO_PORTF_PCTL_R &= ~0x000FFFFF;  // Configure as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x1F;  // Disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x11;  // Enable weak pull-up on PF0, PF4
	GPIO_PORTF_IS_R &= ~0x11;  // PF0, PF4 are edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;  // PF0, PF4 are not both edges
  GPIO_PORTF_IEV_R &= ~0x11;  // PF0, PF4 falling edge event
  GPIO_PORTF_ICR_R |= 0x11;  // Clear flag for PF0, PF4
  GPIO_PORTF_IM_R |= 0x11;  // Enable interrupt on PF0, PF4
	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // Priority 5
  NVIC_EN0_R |= NVIC_EN0_PORTF;  // Enable interrupt in NVIC
}

// Convert the current LED state to a string
char* LEDtoSTR(){
	switch(LED){
		case DARK:
			return (char *)"Dark";
		case RED:
			return (char *)"Red";
		case BLUE:
			return (char *)"Blue";
		case GREEN:
			return (char *)"Green";
		case YELLOW:
			return (char *)"Yellow";
		case CYAN:
			return (char *)"Cyan";
		case PURPLE:
			return (char *)"Purple";
		case WHITE:
			return (char *)"White";
		default:
			return (char *)"Unknown";
	}
}

// Get the current color index from the LED state
int LEDtoINDEX(){
	for(int i = 0; i < sizeof(color_wheel); i++){
		if(LED == color_wheel[i]){
			return i;
		}
	}
	return -1;
}
