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

#include <board.h>
#include <pio/pio.h>
#include <dbgu/dbgu.h>
#include <irq/irq.h>
#include <rtc/rtc.h>
#include <utility/trace.h>
#include <usart/usart.h>

#include <stdio.h>
#include <stdarg.h>

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------
// test which usart, USART_0 or USART_3
#define USART_0
//#define USART_3

/// USART0 TXD pin definition.
#define PIN_USART0_TXD  {1 << 19, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// USART0 RXD pin definition.
#define PIN_USART0_RXD  {1 << 18, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

/// USART3 TXD pin definition.
#define PIN_USART3_TXD  {1 << 8, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// USART3 RXD pin definition.
#define PIN_USART3_RXD  {1 << 9, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
#if	defined(USART_0)
	#define TEST_USART_BASE	AT91C_BASE_US0	
	#define TEST_USART_ID	AT91C_ID_US0
	const Pin pinsUsart[] = {PIN_USART0_TXD, PIN_USART0_RXD};
#elif defined(USART_3)
	#define TEST_USART_BASE	AT91C_BASE_US3
	#define TEST_USART_ID	AT91C_ID_US3
	const Pin pinsUsart[] = {PIN_USART3_TXD, PIN_USART3_RXD};
#endif
        
//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Default main() function. Initializes the DBGU and writes a string on the
/// DBGU.
//------------------------------------------------------------------------------
int main(void)
{
    unsigned char c;
	unsigned int mode;

	// Configure USART pins
	PIO_Configure(pinsUsart, PIO_LISTSIZE(pinsUsart));
	// Set USART's mode
	mode =  AT91C_US_USMODE_NORMAL
            | AT91C_US_CLKS_CLOCK
            | AT91C_US_CHRL_8_BITS
            | AT91C_US_PAR_NONE
            | AT91C_US_NBSTOP_1_BIT
            | AT91C_US_CHMODE_NORMAL;
	PMC_EnablePeripheral(TEST_USART_ID);
	USART_Configure(TEST_USART_BASE, mode, 115200, BOARD_MCK);
	// Enable USART transmite and receive
	USART_SetTransmitterEnabled(TEST_USART_BASE, 1);
	USART_SetReceiverEnabled(TEST_USART_BASE, 1);
 
#if	defined(USART_0)
	printf("TEST USART0...\r\n");
#elif defined(USART_3)
	printf("TEST USART3...\r\n");
#endif
   
	printf("Please input:\r\n");
	while (1)
	{
		c = USART_GetChar(TEST_USART_BASE);
		USART_PutChar(TEST_USART_BASE, c);
	}

    return 0;
}

struct __FILE { int handle;} ;
FILE __stdout;
FILE __stderr;

//------------------------------------------------------------------------------
///  Outputs a character to a file.
//------------------------------------------------------------------------------
int fputc(int ch, FILE *f) {
    if ((f == stdout) || (f == stderr)) {
        USART_PutChar(TEST_USART_BASE, ch);
        return ch;
    }
    else {
        return EOF;
    }
}
