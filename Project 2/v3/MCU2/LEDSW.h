#include "tm4c123gh6pm.h"
#include <stdint.h>

// TODO: define bit values for all Colors 
#define PERIOD 20000 

extern volatile unsigned long HIGH,LOW;

void LEDSW_Init(void);
char* LEDGetColorString();