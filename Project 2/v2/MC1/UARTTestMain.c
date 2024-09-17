// UARTTestMain.c
// Runs on LM4F120/TM4C123
// Used to test the UART.c driver
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

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
// Modified by Dr. Min He on 9/23/2023

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include "PLL.h"
#include "UART.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

#define MAX_STR_LEN 20

int main(void){
  uint8_t i;
  uint8_t string[MAX_STR_LEN];  
  uint32_t n;

  PLL_Init();
  UART_Init(false,false);              // initialize UART
  OutCRLF();
  for(i='A'; i<='Z'; i=i+1){// print the uppercase alphabet
    UART_OutChar(i);
  }
  OutCRLF();
  UART_OutChar(' ');
  for(i='a'; i<='z'; i=i+1){// print the lowercase alphabet
    UART_OutChar(i);
  }
  OutCRLF();
  UART_OutChar('-');
  UART_OutChar('-');
  UART_OutChar('>');
  OutCRLF();
  while(1){
    UART_OutString((uint8_t *)"InString(less than 19 characters): ");
    UART_InString(string,19);
    OutCRLF();
    UART_OutString((uint8_t *)" OutString="); UART_OutString(string); OutCRLF();

    UART_OutString((uint8_t *)"InUDec: ");  n=UART_InUDec(); OutCRLF();
    UART_OutString((uint8_t *)" OutUDec="); UART_OutUDec(n); OutCRLF();

    UART_OutString((uint8_t *)"InUHex: ");  n=UART_InUHex(); OutCRLF();
    UART_OutString((uint8_t *)" OutUHex="); UART_OutUHex(n); OutCRLF();

  }
}
