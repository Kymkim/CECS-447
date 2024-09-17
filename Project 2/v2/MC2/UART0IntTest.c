// UART0IntTest.c
// Runs on TM4C123
// Used to test the UART Rx interrupt.

// By Dr. Min He on 9/23/2023

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "UART.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  
extern void WaitForInterrupt(void);
uint32_t Str_to_UDec(uint8_t str[]);
void UDec_to_Str(uint8_t str[], uint32_t n);

#define MAX_STR_LEN 50

uint8_t str_idx=0;
uint8_t string[MAX_STR_LEN];  
bool end_of_str=false;

int main(void){
  uint32_t n;
  uint8_t i;
	DisableInterrupts();
  PLL_Init();
  UART_Init(true,false);              // initialize UART
	EnableInterrupts();
	
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
		while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}		
		end_of_str = false;
		str_idx = 0;
		OutCRLF();
		UART_OutString((uint8_t *)" OutString="); 
		UART_OutString(string); 
		OutCRLF();
		
		UART_OutString((uint8_t *)"InUDec(<2^32-1): ");
		while (!end_of_str) { // wait until the whole string is received.
			WaitForInterrupt();
		}		
		end_of_str = false;
		str_idx = 0;
		OutCRLF();
		
		n = Str_to_UDec(string);
		UDec_to_Str(string,n);
    UART_OutString((uint8_t *)" OutUDec="); 
		UART_OutString(string); 
		OutCRLF();
  }
}

uint32_t Str_to_UDec(uint8_t str[]){
	uint32_t number=0;;
	uint8_t character,idx=0;
	
  character = str[idx];
  while(character != NULL){
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 2^32-1
    }
    else { // none decimal digit fond, stop converting and return previous digits
			return number;
    }
    character = str[++idx]; // get the next digit
  }
  return number;
}

void UDec_to_Str(uint8_t str[], uint32_t n){
	static uint8_t idx=0;
	
	if ((n/10)!=0) {
		UDec_to_Str(str,n/10);
		UDec_to_Str(str,n%10);
	}
	else {
  	str[idx++] = n%10+'0';
		return;
	}
	  
	str[idx] = '\0';	
	return;
}

// Take care of Rx interrupt and ignore Tx interrupt
void UART0_Handler(void){
	uint8_t chr;
	
  if(UART0_RIS_R&UART_RIS_RXRIS){       // received one item
		if ((UART0_FR_R&UART_FR_RXFE) == 0) {
			chr = UART0_DR_R&0xFF;
			if (chr==CR){  // reach end of the string
				end_of_str=true;
		    string[str_idx]=NULL;  // add null terminator to end a C string.
			}
			else if (str_idx<(MAX_STR_LEN-1)){  // save one spot for C null terminator '\0'.
				if (chr==BS) {
          UART_OutChar(BS);
					str_idx--;
				}
				else {
			    string[str_idx++]=chr;  // add the latest received symbol to end a C string.
					UART_OutChar(chr);
			  }
			}
	  }
    UART0_ICR_R = UART_ICR_RXIC;        // acknowledge RX FIFO
  }
}
