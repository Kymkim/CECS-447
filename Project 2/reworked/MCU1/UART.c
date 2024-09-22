// UART.c
// Runs on TM4C123
// Simple device driver for the UART. This is an example code for UART board to board communication.
// board to board communitation uses UART2
// By Dr.Min He
#include "UART.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

#define NVIC_EN1_UART2 0x00000002

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 50 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART2_Init(bool RxInt, bool TxInt){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART2; // activate UART2
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; // activate port B
	while((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOD) == 0){}; 
		
  UART2_CTL_R = 0;                     // reset UART2
  UART2_IBRD_R = 81;                    // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.68)
  UART2_FBRD_R = 24;                     // FBRD = int(0.68 * 64 + 0.5) = 44
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART2_LCRH_R = UART_LCRH_WLEN_8;
		
	if (RxInt) {
	  UART2_IM_R |= UART_IM_RXIM;         // Enable RX interrupt
	}
	if (TxInt) {
	  UART2_IM_R |= UART_IM_TXIM;         // Enable TX interrupt
	}
	
  UART2_CTL_R |= UART_CTL_UARTEN|UART_CTL_RXE|UART_CTL_TXE; // enable UART, Rx, Tx
	
	if ( RxInt | TxInt) {
		NVIC_PRI8_R = (NVIC_PRI8_R&~0x0000F000)|0x00008000; // bits 23-21, priority 5
		NVIC_EN1_R = NVIC_EN1_UART2;           // enable interrupt 5 in NVIC
	}
		
	GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
	GPIO_PORTD_CR_R |= 0xFF;
  GPIO_PORTD_AFSEL_R |= 0xC0;           // enable alt funct on PB1-0
  GPIO_PORTD_DEN_R |= 0xC0;             // enable digital I/O on PB1-0
                                        // configure PB1-0 as UART
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0x00FFFFFF)+0x11000000;
  GPIO_PORTD_AMSEL_R &= ~0xC0;          // disable analog functionality on PB1-0
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t UART2_InChar(void){
  while((UART2_FR_R&UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
  return((uint8_t)(UART2_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART2_OutChar(uint8_t data){
  while((UART2_FR_R&UART_FR_TXFF) != 0);
  UART2_DR_R = data;
}
