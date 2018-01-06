/**************************************************************************************
* File Name          : main.c
* Date First Issued  : 01/10/2010
* Description        : Main program body
***************************************************************************************
***************************************************************************************
* History:
* 01/10/2010:          V1.0
**************************************************************************************/

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <board.h>
#include <board_memories.h>
#include <utility/trace.h>
#include <utility/assert.h>
#include "at91sam9g45/AT91SAM9G45.h"
#include <pio/pio.h>
#include <pmc/pmc.h>

#define PINS_BUZZER 	{1 << 31, AT91C_BASE_PIOE, AT91C_ID_PIOD_E, PIO_OUTPUT_0, PIO_DEFAULT}

static const Pin pinsBuzzer[] = {PINS_BUZZER};

//------------------------------------------------------------------------------
/// main
//------------------------------------------------------------------------------
int main( void )
{
	unsigned int i;

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);

    printf("-- Basic Buzzer Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

	printf("Listen to the Buzzer...\r\n");

	PMC_EnablePeripheral(AT91C_ID_PIOD_E);
	PIO_Configure(pinsBuzzer, PIO_LISTSIZE(pinsBuzzer));

	while (1)
	{
		PIO_Set(pinsBuzzer);
	 
		for (i = 0; i < 0x8000000; i++);
	
		PIO_Clear(pinsBuzzer);	

		for (i = 0; i < 0x8000000; i++);
	}

    return 0;
}
