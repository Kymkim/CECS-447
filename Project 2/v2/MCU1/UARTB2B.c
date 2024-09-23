// UARTB2B.c: example program for testing MCU to MCU UART communication
// Runs on TM4C123 
// By Dr.Min He

// board to board communitation use UART1
// Ground connected ground in the USB cable
#include "tm4c123gh6pm.h"
#include "UART.h"
#include <stdint.h>

// bit address definition for port data registers
#define LED (*((volatile uint32_t *)0x40025008))  // use onboard RED LED: PF1

////////// Constants //////////  
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// white    RGB    0x0E
// pink     R-B    0x06
// Cran     -GB    0x0C

#define DARK    	0x00
#define RED     	0x02
#define SW1       0x10
#define SW2       0x01
#define NVIC_EN0_PORTF 0x40000000

extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  
extern void WaitForInterrupt(void);

void GPIO_PortF_Init(void);

int main(void){
  DisableInterrupts();	
  UART1_Init(false,false);  // initialize UART with no interrupt
	GPIO_PortF_Init();			  // initialize port F
  EnableInterrupts();       // needed for TExaS
	LED = DARK;
  while(1){
		LED = UART1_InChar(); // busy waiting approach for UART Rx
  }
}

void GPIO_PortF_Init(void)
{
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // (a) activate clock for port F
	while ((SYSCTL_RCGC2_R &= SYSCTL_RCGC2_GPIOF)==0){};
		
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x13;           // allow changes to PF4,1,0       
  GPIO_PORTF_DIR_R |= 0x02;          // (c) make RED LED output
  GPIO_PORTF_DIR_R &= ~0x11;
  GPIO_PORTF_AFSEL_R &= ~0x13;       //     disable alt funct 
  GPIO_PORTF_DEN_R |= 0x13;          //     enable digital   
  GPIO_PORTF_PCTL_R &= ~0x000F00FF; // configure as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x13;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF0,4
		
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF0,4 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF0,4 is not both edges
  GPIO_PORTF_IEV_R |= 0x11;    //     PF4 rising edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4
  NVIC_PRI7_R = (NVIC_PRI7_R&~0x00E00000)|0x00C00000; // (g) priority 6
  NVIC_EN0_R |= NVIC_EN0_PORTF;      // (h) enable interrupt 30 in NVIC
}

void GPIOPortF_Handler(void)
{		
	// simple debouncing code: generate 20ms to 30ms delay
	for (uint32_t time=0;time<80000;time++) {}
	
  if(GPIO_PORTF_RIS_R & SW2)
	{
		GPIO_PORTF_ICR_R = SW2;
		LED ^= RED;		
	}
	
	if(GPIO_PORTF_RIS_R & SW1)
	{
		GPIO_PORTF_ICR_R = SW1;
		UART1_OutChar(LED);		
	}
}
