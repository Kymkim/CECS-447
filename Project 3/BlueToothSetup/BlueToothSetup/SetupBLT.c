// SetupBLT.c
// Runs on TM4C123
// This is an example program to setup HC-05 Bluetooth module with no user interface.
// UART0 is used for the TM4C123 to communicate with PC serial terminal, 
// UART1 is used for the TM4C123 to cummunicate with HC-05 Bluetooth module
// U1Rx (PB0) connects to HC-05 TXD pin
// U1Tx (PB1) connects to HC-05 RXD pin
// HC-05 VCC connects to vbus pin on TM4C123
// HC-05 EN connects to +3.3v
// By Min He,10/11/2022

#include "tm4c123gh6pm.h"
#include "UART0.h"
#include "BLT.h"
#include <stdint.h>  // for data type alias

// main function for programming BT device with no UI
int main(void) {
	
	char bt_cmd[32];
	char bt_reply[32];
	
	UART0_Init();
	BLT_Init();
	
	//Fancy Menu stuff cuz why not
	UART0_OutString((uint8_t *) "+----------------------------------------------------------+" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "| WELCOME TO BLUETOOTH SERIAL TERMINAL                 ^-^ |" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "+----------------------------------------------------------+" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "|This is the startup program for the HC-05 Bluetooth module|" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "|You are at 'AT' Command Mode                              |" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "|Type 'AT' and followed with a command                     |" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "|Example: AT+NAME=DH                                       |" ); UART0_OutCRLF();
	UART0_OutString((uint8_t *) "+----------------------------------------------------------+" ); UART0_OutCRLF();

	while(1){
		UART0_InString((uint8_t *) bt_cmd, 29); //Take in command
		BLT_OutString ((uint8_t *)  bt_cmd); 		 //Send Command
		BLT_OutString ((uint8_t *)"\r\n");			 //CR LF to indicate send
		while((UART1_FR_R&UART_FR_BUSY)!=0){};   //Wait until UART1 FIFO ready
		UART0_OutCRLF();
		BLT_InString((uint8_t *) bt_reply);			 //Take in HC-05 Reply
		UART0_OutString((uint8_t *) bt_reply);		 //Print reply from HC-05
			
		//Query Command Have Two Message
		if(bt_cmd[7] == '?'){
			UART0_OutCRLF();
			BLT_InString((uint8_t *) bt_reply);			 //Take in HC-05 Reply
			UART0_OutString((uint8_t *) bt_reply);		 //Print reply from HC-05
		}
		
		UART0_OutCRLF();
		
	}
}

