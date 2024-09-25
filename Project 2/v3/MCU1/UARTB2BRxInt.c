// PC <-> MCU <-> MCU <-> PC UART Communication
// UART Communication with 3 different modes.
// Runs on TM4C123 
// By Oliver Cabral and Jason Chan

// Dependencies and includes
#include "tm4c123gh6pm.h"
#include "LEDSW.h"
#include "UART.h"
#include "USBUART.h"
#include "PLL.h"
#include <stdint.h>
#include <stdbool.h>  

// LED Definitions for controlling different LED colors
#define LED 		(*((volatile uint32_t *)0x40025038))
#define RED 		(0x02)	// Binary for red LED
#define BLUE 		(0x04)	// Binary for blue LED
#define GREEN 	(0x08)	// Binary for green LED
#define PURPLE 	(0x06)	// Combination of red + blue
#define CYAN 		(0x0C)	// Combination of blue + green
#define YELLOW  (0x0A)	// Combination of red + green
#define WHITE 	(0x0E)	// Combination of red + green + blue
#define DARK 		(0X00)	// All off

// Color wheel for cycling through different colors
const	uint8_t color_wheel[] = {DARK, RED, GREEN, BLUE, YELLOW, CYAN, PURPLE, WHITE};
volatile uint8_t COLOR_INDEX = 0;  // Keeps track of current color
volatile uint8_t BRIGHTNESS = 0;   // LED brightness control

// Switch Definitions for user input (SW1, SW2)
#define SW1       0x10
#define SW2       0x01
#define NVIC_EN0_PORTF 0x40000000

// UART 0 string variables to hold output strings
#define MAX_STR_LEN 20
bool end_of_str = false;           // Flag for end of string input
uint8_t string[MAX_STR_LEN];       // Stores the input string
uint8_t str_idx = 0;               // Index to track input string position

// UART 2 input string variables (MCU-to-MCU communication)
bool in_end_of_str = false;
uint8_t in_string[MAX_STR_LEN];    // Stores incoming string from MCU 2
uint8_t in_str_idx = 0;            // Index to track the incoming string

// Mode and control flags
volatile uint8_t MODE = 0;         // Mode indicator (0 = menu, 1 = LED control, 2 = color wheel, 3 = chat)
uint8_t QUIT_CHAT = 0;             // Flag to quit chat mode

// Interrupt handling functions (defined elsewhere)
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  
extern void WaitForInterrupt(void);

// Function declarations
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
  // Initialize system
  DisableInterrupts();         // Disable interrupts during initialization
	PLL_Init();                  // Set system clock
	UART0_Init(true, false);	    // Initialize UART0 for PC communication
  UART2_Init(true, false);     // Initialize UART2 for MCU-MCU communication (with RX interrupt)
	LEDSW_Init();							     // Initialize LED and switches
	DisableSysTick();            // Disable SysTick timer initially
  EnableInterrupts();           // Re-enable interrupts

	LED = color_wheel[COLOR_INDEX];  // Set the initial LED color
	
  while(1){
		// Check if MODE is 0 (main menu)
		if(MODE == 0){
			Main_Menu();                    // Display main menu
			str_idx=0;                      // Reset input string index

			// Wait until a complete input string is received
			while (!end_of_str) {
				WaitForInterrupt();         // Go into low power until interrupted
			}

			// Reset input flags and process the input
			end_of_str = false;
			str_idx = 0;
			UART0_OutCRLF();                // Send newline to the UART terminal
			
			// Check which mode is selected based on the input string
			switch(string[str_idx]){
				case '1':
					MODE=1;                   // Set Mode to 1 (LED control)
					EnableSysTick();           // Enable SysTick for brightness control
					Mode1();                   // Enter Mode 1
					break;
				case '2':
					MODE=2;                   // Set Mode to 2 (MCU-MCU color control)
					Mode2();                   // Enter Mode 2
					break;
				case '3':
					MODE=3;                   // Set Mode to 3 (MCU-PC chat mode)
					QUIT_CHAT=0;               // Reset chat quit flag
					Mode3();                   // Enter Mode 3
					break;
				default:
					// Handle invalid input and prompt for valid choices
					UART0_OutCRLF();
					UART0_OutString((uint8_t *) "Invalid Input. Please choose from 1-3");
					UART0_OutCRLF();
					break;
			}
		}
  }
}

// Mode 1: Control LED (color and brightness) via UART from the PC
void Mode1(void){
	Mode1_Menu();                // Display Mode 1 menu (LED control)
	str_idx = 0;

	// Wait until a valid input is received
	while(!end_of_str){
		WaitForInterrupt();
	}

	// Process input to change LED settings
	end_of_str = false;
	str_idx = 0;
	UART0_OutCRLF();
	switch(string[0]){
		case '1':
			Mode1_ChangeLED();     // Change the LED color
			Mode1();               // Re-enter Mode 1
			break;
		case '2':
			Mode1_ChangeBrightness(); // Change the LED brightness
			Mode1();                  // Re-enter Mode 1
			break;
		case '3':
			// Exit Mode 1 and return to main menu
			UART0_OutCRLF();
			MODE=0;
			break;
		default:
			// Handle invalid input
			UART0_OutCRLF();
			UART0_OutString((uint8_t *) "Invalid Input. Please choose from 1-3");
			UART0_OutCRLF();
			Mode1();                 // Re-enter Mode 1 after invalid input
			break;
	}
	DisableSysTick();              // Disable SysTick when exiting Mode 1
}

// Mode 2: MCU to MCU color wheel mode
void Mode2(){
	UART2_OutChar('2');           // Send mode ID to the second MCU
	UART2_OutChar(CR);            // Send carriage return

	Mode2_MenuTX();               // Display Mode 2 TX menu

	// Wait for user input to change color
	while (!end_of_str) {
		WaitForInterrupt();
	}

	end_of_str = false;
	str_idx = 0;

	// Reset LED state and color index after exiting Mode 2
	LED = DARK;
	COLOR_INDEX = 0;
	UART2_OutChar('^');           // Send exit signal to the second MCU

	MODE=0;                       // Return to main menu
}

// Mode 3: Chat room between PC and MCU2
void Mode3(){
	UART2_OutChar('3');           // Send mode ID to the second MCU
	UART2_OutChar(CR);            // Send carriage return

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

	// Chat loop: send and receive messages between PC and MCU2
	while(!QUIT_CHAT){
		// Send message to MCU2
		UART0_OutString((uint8_t *)"MCU 1: ");
		end_of_str = false;
		str_idx = 0;
		while((!end_of_str)&(!QUIT_CHAT)){
			WaitForInterrupt();
		}
		if(!QUIT_CHAT){
			UART2_OutString(string);    // Send the message to MCU2
			UART2_OutChar(CR);          // Send carriage return
			UART0_OutCRLF();
		}

		// Wait for message from MCU2
		if(!QUIT_CHAT){
			in_end_of_str=false;
			in_str_idx=0;
			while((!in_end_of_str) & (!QUIT_CHAT)){
				WaitForInterrupt();
			}
			if(!QUIT_CHAT){
				UART0_OutString((uint8_t *) "MCU 2: "); // Display received message
				UART0_OutString(in_string);
				UART0_OutCRLF();
			}
		}
	}
}
