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

/// USART1 TXD pin definition.
#define PIN_USART1_TXD  {1 << 4, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
/// USART1 RXD pin definition.
#define PIN_USART1_RXD  {1 << 5, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_RTS1  {1 << 16, AT91C_BASE_PIOD, AT91C_ID_PIOD_E, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_CTS1  {1 << 17, AT91C_BASE_PIOD, AT91C_ID_PIOD_E, PIO_PERIPH_A, PIO_DEFAULT}

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
#define TEST_USART_BASE	AT91C_BASE_US1	
#define TEST_USART_ID	AT91C_ID_US1
const Pin pinsUsart[] = {PIN_USART1_TXD, PIN_USART1_RXD,PIN_USART1_RTS1,PIN_USART1_CTS1};

        
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
	printf("Test USART1(use hardware handshaking)...\r\n");
#else
	printf("Test USART1(don't use hardware handshaking)...\r\n");
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
