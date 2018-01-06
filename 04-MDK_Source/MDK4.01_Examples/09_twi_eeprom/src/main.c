/***************************************************************************************
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
#include <irq/irq.h>
#include <dbgu/dbgu.h>
#include <twi/twi.h>
#include <utility/math.h>
#include <utility/assert.h>
#include <utility/trace.h>
#include <drivers/async/async.h>
#include <drivers/twi/twid.h>

#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------

/// TWI clock frequency in Hz.
#define TWCK            400000

/// Slave address of AT24C chips.
#define AT24C_ADDRESS   0x50

/// Page size of an AT24C1024 chip (in bytes)
#define PAGE_SIZE       64

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

/// Pio pins to configure.
static const Pin pins[] = {BOARD_PINS_TWI_EEPROM};

/// TWI driver instance.
static Twid twid;

/// Page buffer.
static unsigned char pData[PAGE_SIZE];

//------------------------------------------------------------------------------
///        Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// TWI interrupt handler. Forwards the interrupt to the TWI driver handler.
//------------------------------------------------------------------------------
void TWI1_IrqHandler(void)
{
    TWID_Handler(&twid);
}

//------------------------------------------------------------------------------
/// Dummy callback, to test asynchronous transfer modes.
//------------------------------------------------------------------------------
void TestCallback()
{
    printf("-I- Callback fired !\n\r");
}

//------------------------------------------------------------------------------
///        Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Main function
//------------------------------------------------------------------------------
int main()
{
    volatile unsigned int i;
    Async async;
    unsigned int numErrors;

    PIO_Configure(pins, PIO_LISTSIZE(pins));
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic TWI EEPROM Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    // Configure TWI
    // In IRQ mode: to avoid problems, the priority of the TWI IRQ must be max.
    // In polling mode: try to disable all IRQs if possible.
    // (in this example it does not matter, there is only the TWI IRQ active)
    AT91C_BASE_PMC->PMC_PCER = 1 << BOARD_ID_TWI_EEPROM;//AT91C_ID_TWI;
    TWI_ConfigureMaster(BOARD_BASE_TWI_EEPROM, TWCK, BOARD_MCK);
    TWID_Initialize(&twid, BOARD_BASE_TWI_EEPROM);
    IRQ_ConfigureIT(BOARD_ID_TWI_EEPROM, 0, TWI1_IrqHandler);
    IRQ_EnableIT(BOARD_ID_TWI_EEPROM);

    // Erase page #0 and #1
    memset(pData, 0, PAGE_SIZE);
    printf("-I- Filling page #0 with zeroes ...\n\r");
    TWID_Write(&twid, AT24C_ADDRESS, 0x0000, 2, pData, PAGE_SIZE, 0);

    // Wait at least 10 ms
    for (i=0; i < 1000000; i++);

    printf("-I- Filling page #1 with zeroes ...\n\r");
    TWID_Write(&twid, AT24C_ADDRESS, 0x0100, 2, pData, PAGE_SIZE, 0);

    // Wait at least 10 ms
    for (i=0; i < 1000000; i++);

    // Synchronous operation
    printf("-I- Read/write on page #0 (polling mode)\n\r");

    // Write checkerboard pattern in first page
    for (i=0; i < PAGE_SIZE; i++) {

        // Even
        if ((i & 1) == 0) {
        
            pData[i] = 0xA5;
        }
        // Odd
        else {

            pData[i] = 0x5A;
        }
    }
    TWID_Write(&twid, AT24C_ADDRESS, 0x0000, 2, pData, PAGE_SIZE, 0);

    // Wait at least 10 ms
    for (i=0; i < 1000000; i++);

    // Read back data
    memset(pData, 0, PAGE_SIZE);
    TWID_Read(&twid, AT24C_ADDRESS, 0x0000, 2, pData, PAGE_SIZE, 0);

    // Compare
    numErrors = 0;
    for (i=0; i < PAGE_SIZE; i++) {

        // Even
        if (((i & 1) == 0) && (pData[i] != 0xA5)) {

            printf("-E- Data mismatch at offset #%u: expected 0xA5, read 0x%02X\n\r", i, pData[i]);
            numErrors++;
        }
        // Odd
        else if (((i & 1) == 1) && (pData[i] != 0x5A)) {

            printf("-E- Data mismatch at offset #%u: expected 0x5A, read 0x%02X\n\r", i, pData[i]);
            numErrors++;
        }
    }
    printf("-I- %u comparison error(s) found\n\r", numErrors);

    // Asynchronous operation
    printf("-I- Read/write on page #1 (IRQ mode)\n\r");

    // Write checkerboard pattern in first page
    for (i=0; i < PAGE_SIZE; i++) {

        // Even
        if ((i & 1) == 0) {
        
            pData[i] = 0xA5;
        }
        // Odd
        else {

            pData[i] = 0x5A;
        }
    }
    memset(&async, 0, sizeof(async));
    async.callback = (void *) TestCallback;
    TWID_Write(&twid, AT24C_ADDRESS, 0x0100, 2, pData, PAGE_SIZE, &async);
    while (!ASYNC_IsFinished(&async));

    // Wait at least 10 ms
    for (i=0; i < 1000000; i++);

    // Read back data
    memset(pData, 0, PAGE_SIZE);
    memset(&async, 0, sizeof(async));
    async.callback = (void *) TestCallback;
    TWID_Read(&twid, AT24C_ADDRESS, 0x0100, 2, pData, PAGE_SIZE, &async);
    while (!ASYNC_IsFinished(&async));

    // Compare
    numErrors = 0;
    for (i=0; i < PAGE_SIZE; i++) {

        // Even
        if (((i & 1) == 0) && (pData[i] != 0xA5)) {

            printf("-E- Data mismatch at offset #%u: expected 0xA5, read 0x%02X\n\r", i, pData[i]);
            numErrors++;
        }
        // Odd
        else if (((i & 1) == 1) && (pData[i] != 0x5A)) {

            printf("-E- Data mismatch at offset #%u: expected 0x5A, read 0x%02X\n\r", i, pData[i]);
            numErrors++;
        }
    }
    printf("-I- %u comparison error(s) found\n\r", numErrors);

    return 0;
}

