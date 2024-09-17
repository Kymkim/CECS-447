#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

//U2Rx = PD6
//U2Tx = PD7

#define CR 		0x0D
#define LF 		0x0A
#define BS 		0x08
#define ESC 	0x1B
#define SP		0x20
#define DEL		0x7F
#define NULL	0

void UART2_Init(bool RxInt, bool TxInt);

void UART2_OutCRLF(void);

unsigned char UART2_InChar(void);

void UART2_OutChar(unsigned char data);

void UART2_InString(unsigned char *bufPt, unsigned short max);