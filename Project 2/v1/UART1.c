// UART.c
// Runs on TM4C123
// Simple device driver for the UART. This is an example code for UART board to board communication.
// board to board communitation uses UART1
// By Dr.Min He
#include "UART1.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

#define NVIC_EN0_UART1 0x40

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 50 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART1_Init(bool RxInt, bool TxInt){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
	while((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOB) == 0){}; 
		
  UART1_CTL_R = 0;                     // reset UART1
  UART1_IBRD_R = 81;                    // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.68)
  UART1_FBRD_R = 24;                     // FBRD = int(0.68 * 64 + 0.5) = 44
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_LCRH_R = UART_LCRH_WLEN_8;
	if (RxInt) {
	  UART1_IM_R |= UART_IM_RXIM;         // Enable RX interrupt
	}
	
	if (TxInt) {
	  UART1_IM_R |= UART_IM_TXIM;         // Enable TX interrupt
	}
  UART1_CTL_R |= UART_CTL_UARTEN|UART_CTL_RXE|UART_CTL_TXE; // enable UART, Rx, Tx
	
	if ( RxInt | TxInt) {
		NVIC_PRI1_R = (NVIC_PRI1_R&0xFF1FFFFF)|0x00A00000; // bits 23-21, priority 5
		NVIC_EN0_R = NVIC_EN0_UART1;           // enable interrupt 5 in NVIC
	}
		
  GPIO_PORTB_AFSEL_R |= 0x03;           // enable alt funct on PB1-0
  GPIO_PORTB_DEN_R |= 0x03;             // enable digital I/O on PB1-0
                                        // configure PB1-0 as UART
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFFFF00)+0x00000011;
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
