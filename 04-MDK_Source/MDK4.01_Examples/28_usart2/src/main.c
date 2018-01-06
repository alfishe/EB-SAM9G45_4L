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
// use handshaking or not
//#define HANDSHAKE

/// USART2 TXD pin definition.
#define PIN_USART2_TXD  {1 << 6, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// USART2 RXD pin definition.
#define PIN_USART2_RXD  {1 << 7, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART2_RTS2  {1 << 9, AT91C_BASE_PIOC, AT91C_ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_USART2_CTS2  {1 << 11, AT91C_BASE_PIOC, AT91C_ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
#define TEST_USART_BASE	AT91C_BASE_US2	
#define TEST_USART_ID	AT91C_ID_US2
const Pin pinsUsart[] = {PIN_USART2_TXD, PIN_USART2_RXD, PIN_USART2_RTS2, PIN_USART2_CTS2};
        
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
#if defined(HANDSHAKE)
	mode =  AT91C_US_USMODE_MODEM
            | AT91C_US_CLKS_CLOCK
            | AT91C_US_CHRL_8_BITS
            | AT91C_US_PAR_NONE
            | AT91C_US_NBSTOP_1_BIT
            | AT91C_US_CHMODE_NORMAL;
#else
	mode =  AT91C_US_USMODE_NORMAL
            | AT91C_US_CLKS_CLOCK
            | AT91C_US_CHRL_8_BITS
            | AT91C_US_PAR_NONE
            | AT91C_US_NBSTOP_1_BIT
            | AT91C_US_CHMODE_NORMAL;
#endif
	PMC_EnablePeripheral(TEST_USART_ID);
	USART_Configure(TEST_USART_BASE, mode, 115200, BOARD_MCK);
	// Enable USART transmite and receive
	USART_SetTransmitterEnabled(TEST_USART_BASE, 1);
	USART_SetReceiverEnabled(TEST_USART_BASE, 1);

#if defined(HANDSHAKE)
	printf("Test USART2(use hardware handshaking)...\r\n");
#else
	printf("Test USART2(don't use hardware handshaking)...\r\n");
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
