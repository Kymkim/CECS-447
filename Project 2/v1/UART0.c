// UART0.c
// Runs on TM4C123
// By Dr. Min He

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include "UART0.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART0_Init(void){
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0; // activate UART0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0; // activate port A
	
  UART0_CTL_R = 0;                      // disable UART
  UART0_IBRD_R = 8;                    // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.68)
  UART0_FBRD_R = 44;                     // FBRD = int(0.68 * 64 + 0.5) = 44                                        
  UART0_LCRH_R = UART_LCRH_WLEN_8;    // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_CTL_R |= UART_CTL_RXE|UART_CTL_TXE|UART_CTL_UARTEN;// enable Tx, RX and UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0                                        
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)|0x00000011;// configure PA1-0 as UART
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}

//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none

void UART0_OutCRLF(void){
  UART0_OutChar(CR);
  UART0_OutChar(LF);
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t UART0_InChar(void){
  while((UART0_FR_R&UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
  return((uint8_t)(UART0_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART0_OutChar(uint8_t data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}


//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART0_OutString(uint8_t *pt){
  while(*pt){
    UART0_OutChar(*pt);
    pt++;
  }
	UART0_OutChar(0); // add the null terminator
}

//------------UART0_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
//    when max length is reach, no more input will be accepted
//    the display will wait for the <enter> key to be pressed.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void UART0_InString(uint8_t *bufPt, uint16_t max) {
  uint16_t length=0;
  uint8_t character;
	
  character = UART0_InChar();
  while(character != CR){
    if(character == BS){ // back space
      if(length){
        bufPt--;
        length--;
        UART0_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      UART0_OutChar(character);
    }
    character = UART0_InChar();
  }
  *bufPt = 0; // adding null terminator to the end of the string.
}
