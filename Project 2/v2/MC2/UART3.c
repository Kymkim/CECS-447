// UART2.c
// Runs on TM4C123

// U2Rx connected to PD6
// U2Tx connected to PD7

#include "UART2.h"
#include "tm4c123gh6pm.h"
														// 59 58 57 56 |55 54 53 52 |51 50 49 48 |47 46 45 44 |43 42 41 40 |39 38 37 36 |35 34 33 32   
#define NVIC_EN1_UART3 0x8000000 // 1  0  0  0   0  0  0  0

//------------UART2_Init------------
// Initialize UART2 for 115,200 baud rate (assuming 50 MHz UART clock),
// 8-bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART2_Init(bool RxInt, bool TxInt) {
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R3;  // activate UART2
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;  // activate port D

  UART3_CTL_R = 0;                      // disable UART
  UART3_IBRD_R = 81;                    // IBRD = int(50,000,000 / (16 * 38400)) = int(81.3802)
  UART3_FBRD_R = 24;                    // FBRD = int(0.3802 * 64 + 0.5) = 8.228
  UART3_LCRH_R = UART_LCRH_WLEN_8;      // 8-bit word length (no parity bits, one stop bit, FIFOs)

  // Take care of interrupt setup
  if (RxInt || TxInt) {
		
		//UART 3 = 59 = PRI14
		//           59  x       58  x       57  x       56  x                                      																								    
		//0xF0000000 |111|1 0000 |000|0 0000 |000|0 0000 |000|0 0000
		//0x80000000 |100|0 0000 |000|0 0000 |000|0 0000 |000|0 0000
		
    NVIC_PRI14_R = (NVIC_PRI14_R & ~0xF0000000) | 0x80000000;  // bits 15-13, priority 4
    NVIC_EN0_R = NVIC_EN1_UART3;           // enable UART2 interrupt in NVIC
    if (RxInt) {
      UART3_IM_R |= UART_IM_RXIM;         // Enable RX interrupt
    }

    if (TxInt) {
      UART3_IM_R |= UART_IM_TXIM;         // Enable TX interrupt
    }
  }
  
  UART3_CTL_R |= UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;  // enable Tx, RX, and UART
	
	//To Do Tonight
	GPIO_PORTC_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTC_AFSEL_R |= 0xC0;           // enable alt funct on PD7-6
  GPIO_PORTC_DEN_R |= 0xC0;             // enable digital I/O on PD7-6
                                       // configure PD7-6 as UART
  //GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R & 0xFFFF00FF) + 0x00001100;
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R & 0x00FFFFFF) + 0x11000000;	
  GPIO_PORTC_AMSEL_R &= ~0xC0;          // disable analog functionality on PD
}

//---------------------UART2_OutCRLF---------------------
// Output a CR,LF to UART2 to go to a new line
// Input: none
// Output: none
void UART3_OutCRLF(void) {
  UART3_OutChar(CR);
  UART3_OutChar(LF);
}

//------------UART2_InChar------------
// Wait for new serial port input on UART2
// Input: none
// Output: ASCII code for key typed
unsigned char UART3_InChar(void) {
  while ((UART3_FR_R & UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
  return ((unsigned char)(UART3_DR_R & 0xFF));
}

//------------UART2_OutChar------------
// Output 8-bit to serial port UART2
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART3_OutChar(unsigned char data) {
  while ((UART3_FR_R & UART_FR_TXFF) != 0);
  UART3_DR_R = data;
}

//------------UART2_OutString------------
// Output String (NULL termination) on UART2
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART3_OutString(unsigned char *pt) {
  while (*pt) {
    UART3_OutChar(*pt);
    pt++;
  }
  UART3_OutChar(0); // add the null terminator
}

//------------UART2_InString------------
// Accepts ASCII characters from the serial port UART2
// and adds them to a string until <enter> is typed
// or until the max length of the string is reached.
// When the max length is reached, no more input will be accepted,
// and the display will wait for the <enter> key to be pressed.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified,
// and the backspace is echoed.
// Terminates the string with a null character.
// Uses busy-waiting synchronization on RDRF.
// Input: pointer to an empty buffer, size of buffer
// Output: Null-terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void UART3_InString(unsigned char *bufPt, unsigned short max) {
  int length = 0;
  char character;
  character = UART3_InChar();
  while (character != CR) {
    if (character == BS) { // backspace
      if (length) {
        bufPt--;
        length--;
        UART3_OutChar(BS);
      }
    } else if (length < max) {
      *bufPt = character;
      bufPt++;
      length++;
      UART3_OutChar(character);
    }
    character = UART3_InChar();
  }
  *bufPt = 0; // add null terminator to the end of the string
}
