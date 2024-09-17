// UART1.c
// Runs on TM4C123
// Simple device driver for the UART.
// Daniel Valvano
// September 11, 2013
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu
// Modified by Min He on 3/7/2024

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
   Program 4.12, Section 4.9.4, Figures 4.26 and 4.40

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include "UART1.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART1_Init(void){
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1; // activate UART1
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1; // activate port B
		
  UART1_CTL_R = 0;                     // reset UART1
  UART1_IBRD_R = 8;                    // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.68)
  UART1_FBRD_R = 44;                     // FBRD = int(0.68 * 64 + 0.5) = 44                                        
  UART1_LCRH_R = UART_LCRH_WLEN_8;     // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_CTL_R |= UART_CTL_UARTEN|UART_CTL_RXE|UART_CTL_TXE; // enable UART, Rx, Tx

  GPIO_PORTB_AFSEL_R |= 0x03;           // enable alt funct on PB1-0
  GPIO_PORTB_DEN_R |= 0x03;             // enable digital I/O on PB1-0                                        
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFFFF00)+0x00000011;// configure PB1-0 as UART
  GPIO_PORTB_AMSEL_R &= ~0x03;          // disable analog functionality on PB1-0
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t UART1_InChar(void){
  while((UART1_FR_R&UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
  return((uint8_t)(UART1_DR_R&0xFF));
}

//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(uint8_t data){
  while((UART1_FR_R&UART_FR_TXFF) != 0);
  UART1_DR_R = data;
}

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART1_OutString(uint8_t *pt){
  while(*pt){
    UART1_OutChar(*pt);
    pt++;
  }
	UART1_OutChar(0); // add the null terminator
}

//------------UART1_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until CR is received
//    or until max length of the string is reached.
// terminates the string with a CR.
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
void UART1_InString(uint8_t *bufPt, uint16_t max) {
  uint16_t length=0;
  uint8_t character;
			
	while ((character = UART1_InChar())==0){}  // skip leading zeros
		
  while((character!=CR) && (length < max-1)){
		*bufPt = character;
		bufPt++;
		length++;
    character = UART1_InChar();
	}
  *bufPt = 0; // adding null terminator to the end of the string.
}

