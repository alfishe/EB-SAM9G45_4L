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

#include <boards/board.h>
#include <pio/pio.h>
#include <irq/irq.h>
#include <utility/assert.h>
#include <utility/trace.h>
#include <spi-flash/at45.h>

#include <string.h>

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------

/// SPI clock frequency, in Hz.
#define SPCK        1000000

/// If there is no on-board dataflash socket, use SPI0/NPCS0.
#ifndef BOARD_AT45_A_SPI_PINS
    #ifdef AT91C_BASE_SPI0
        #define BOARD_AT45_A_SPI_BASE           AT91C_BASE_SPI0
        #define BOARD_AT45_A_SPI_ID             AT91C_ID_SPI0
        #define BOARD_AT45_A_SPI_PINS           PINS_SPI0
        #define BOARD_AT45_A_NPCS_PIN           PIN_SPI0_NPCS0
    #else
        #define BOARD_AT45_A_SPI_BASE           AT91C_BASE_SPI
        #define BOARD_AT45_A_SPI_ID             AT91C_ID_SPI
        #define BOARD_AT45_A_SPI_PINS           PINS_SPI
        #define BOARD_AT45_A_NPCS_PIN           PIN_SPI_NPCS0
    #endif
    #define BOARD_AT45_A_SPI                    0
    #define BOARD_AT45_A_NPCS                   0
#endif

//------------------------------------------------------------------------------
//         Internal variables
//------------------------------------------------------------------------------

/// SPI driver instance.
static Spid spid;

/// AT45 driver instance.
static At45 at45;

/// Pins used by the application.
static const Pin pins[]  = {BOARD_AT45_A_SPI_PINS, BOARD_AT45_A_NPCS_PIN};

/// Page buffer.
static unsigned char pBuffer[2112];

//------------------------------------------------------------------------------
//         Internal functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// SPI interrupt handler. Invokes the SPI driver handler to check for pending
/// interrupts.
//------------------------------------------------------------------------------
static void ISR_Spi(void)
{
    SPID_Handler(&spid);
}

//------------------------------------------------------------------------------
/// Retrieves and returns the At45 current status, or 0 if an error
/// happened.
/// \param pAt45  Pointer to a At45 driver instance.
//------------------------------------------------------------------------------
static unsigned char AT45_GetStatus(At45 *pAt45)
{
    unsigned char error;
    unsigned char status;

    // Sanity checks
    ASSERT(pAt45, "-F- AT45_GetStatus: pAt45 is null\n\r");

    // Issue a status register read command
    error = AT45_SendCommand(pAt45, AT45_STATUS_READ, 1, &status, 1, 0, 0, 0);
    ASSERT(!error, "-F- AT45_GetStatus: Failed to issue command.\n\r");

    // Wait for command to terminate
    while (AT45_IsBusy(pAt45));

    return status;
}

//------------------------------------------------------------------------------
/// Waits for the At45 to be ready to accept new commands.
/// \param pAt45  Pointer to a At45 driver instance.
//------------------------------------------------------------------------------
static void AT45_WaitReady(At45 *pAt45) 
{
    unsigned char ready = 0;

    // Sanity checks
    ASSERT(pAt45, "-F- AT45_WaitUntilReady: pAt45 is null\n\r");

    // Poll device until it is ready
    while (!ready) {

        ready = AT45_STATUS_READY(AT45_GetStatus(pAt45));
    }
}

//------------------------------------------------------------------------------
/// Reads and returns the JEDEC identifier of a At45.
/// \param pAt45  Pointer to a At45 driver instance.
//------------------------------------------------------------------------------
static unsigned int AT45_GetJedecId(At45 *pAt45)
{
    unsigned char error;
    unsigned int id;

    // Sanity checks
    ASSERT(pAt45, "-F- AT45_GetJedecId: pAt45 is null\n\r");

    // Issue a manufacturer and device ID read command
    error = AT45_SendCommand(pAt45, AT45_ID_READ, 1, (void *) &id, 4, 0, 0, 0);
    ASSERT(!error, "-F- AT45_GetJedecId: Could not issue command.\n\r");

    // Wait for transfer to finish
    while (AT45_IsBusy(pAt45));

    return id;
}

//------------------------------------------------------------------------------
/// Reads data from the At45 inside the provided buffer. Since a continuous
/// read command is used, there is no restriction on the buffer size and read
/// address.
/// \param pAt45  Pointer to a At45 driver instance.
/// \param pBuffer  Data buffer.
/// \param size  Number of bytes to read.
/// \param address  Address at which data shall be read.
//------------------------------------------------------------------------------
static void AT45_Read(
    At45 *pAt45,
    unsigned char *pBuffer,
    unsigned int size,
    unsigned int address) 
{
    unsigned char error;

    // Sanity checks
    ASSERT(pAt45, "-F- AT45_Read: pAt45 is null\n\r");
    ASSERT(pBuffer, "-F- AT45_Read: pBuffer is null\n\r");

    // Issue a continuous read array command
    error = AT45_SendCommand(pAt45, AT45_CONTINUOUS_READ_LEG, 8, pBuffer, size, address, 0, 0);
    ASSERT(!error, "-F- AT45_Read: Failed to issue command\n\r");

    // Wait for the read command to execute
    while (AT45_IsBusy(pAt45));
}

//------------------------------------------------------------------------------
/// Writes data on the At45 at the specified address. Only one page of
/// data is written that way; if the address is not at the beginning of the
/// page, the data is written starting from this address and wraps around to
/// the beginning of the page.
/// \param pAt45  Pointer to a At45 driver instance.
/// \param pBuffer  Buffer containing the data to write.
/// \param size  Number of bytes to write.
/// \param address  Destination address on the At45.
//------------------------------------------------------------------------------
static void AT45_Write(
    At45 *pAt45,
    unsigned char *pBuffer,
    unsigned int size,
    unsigned int address) 
{
    unsigned char error;

    // Sanity checks
    ASSERT(pAt45, "-F- AT45_Write: pAt45 is null.\n\r");
    ASSERT(pBuffer, "-F- AT45_Write: pBuffer is null.\n\r");
    ASSERT(size <= pAt45->pDesc->pageSize, "-F- AT45_Write: Size too big\n\r");

    // Issue a page write through buffer 1 command
    error = AT45_SendCommand(pAt45, AT45_PAGE_WRITE_BUF1, 4, pBuffer, size, address, 0, 0);
    ASSERT(!error, "-F- AT45_Write: Could not issue command.\n\r");

    // Wait until the command is sent
    while (AT45_IsBusy(pAt45));
    
    // Wait until the At45 becomes ready again
    AT45_WaitReady(pAt45);
}

//------------------------------------------------------------------------------
/// Erases a page of data at the given address in the At45.
/// \param pAt45  Pointer to a At45 driver instance.
/// \param address  Address of page to erase.
//------------------------------------------------------------------------------
static void AT45_Erase(At45 *pAt45, unsigned int address) 
{
    unsigned char error;

    // Sanity checks
    ASSERT(pAt45, "-F- AT45_Erase: pAt45 is null\n\r");
    
    // Issue a page erase command.
    error = AT45_SendCommand(pAt45, AT45_PAGE_ERASE, 4, 0, 0, address, 0, 0);
    ASSERT(!error, "-F- AT45_Erase: Could not issue command.\n\r");

    // Wait for end of transfer
    while (AT45_IsBusy(pAt45));

    // Poll until the At45 has completed the erase operation
    AT45_WaitReady(pAt45);
}

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Tests the At45 connected to the board by performing several command
/// on each of its pages.
//------------------------------------------------------------------------------
int main()
{
    unsigned int i;
    unsigned int page;
    unsigned char testFailed;
    const At45Desc *pDesc;

    // Configure pins
    PIO_Configure(pins, PIO_LISTSIZE(pins));

    // Configure traces
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic Dataflash Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    
    // SPI and At45 driver initialization
    TRACE_INFO("Initializing the SPI and AT45 drivers\n\r");
    IRQ_ConfigureIT(BOARD_AT45_A_SPI_ID, 0, ISR_Spi);
    SPID_Configure(&spid, BOARD_AT45_A_SPI_BASE, BOARD_AT45_A_SPI_ID);
    SPID_ConfigureCS(&spid, BOARD_AT45_A_NPCS, AT45_CSR(BOARD_MCK, SPCK));
    AT45_Configure(&at45, &spid, BOARD_AT45_A_NPCS);
    TRACE_INFO("At45 enabled\n\r");
    IRQ_EnableIT(BOARD_AT45_A_SPI_ID);
    TRACE_INFO("SPI interrupt enabled\n\r");

    // Identify the At45 device
    TRACE_INFO("Waiting for a dataflash to be connected ...\n\r");
    pDesc = 0;
    while (!pDesc) {

        pDesc = AT45_FindDevice(&at45, AT45_GetStatus(&at45));
    }
    TRACE_INFO("%s detected\n\r", at45.pDesc->name);

    // Output JEDEC identifier of device
    TRACE_INFO("Device identifier: 0x%08X\n\r", AT45_GetJedecId(&at45));

    // Test all pages
    testFailed = 0;
    page = 0;
    while (!testFailed && (page < AT45_PageNumber(&at45))) {

        TRACE_INFO("Test in progress on page: %6u\r", page);
        
        // Erase page
        AT45_Erase(&at45, page * AT45_PageSize(&at45));

        // Verify that page has been erased correctly
        memset(pBuffer, 0, AT45_PageSize(&at45));
        AT45_Read(&at45, pBuffer, AT45_PageSize(&at45), page * AT45_PageSize(&at45));
        for (i=0; i < AT45_PageSize(&at45); i++) {
        
            if (pBuffer[i] != 0xff) {

                TRACE_ERROR("Could not erase page %u\n\r", page);
                testFailed = 1;
                break;
            }
        }

        // Write page
        for (i=0; i < AT45_PageSize(&at45); i++) {
        
            pBuffer[i] = i & 0xFF;
        }
        AT45_Write(&at45, pBuffer, AT45_PageSize(&at45), page * AT45_PageSize(&at45));

        // Check that data has been written correctly
        memset(pBuffer, 0, AT45_PageSize(&at45));
        AT45_Read(&at45, pBuffer, AT45_PageSize(&at45), page * AT45_PageSize(&at45));
        for (i=0; i < AT45_PageSize(&at45); i++) {

            if (pBuffer[i] != (i & 0xFF)) {

                TRACE_ERROR("Could not write page %u\n\r", page);
                testFailed = 1;
                break;
            }
        }

        page++;    
    }

    // Display test result
	printf("\n\r");
    if (testFailed) {
    
        TRACE_ERROR("Test failed.\n\r");
    }
    else {
    
         TRACE_INFO("Test passed.\n\r");
    }
    
    return 0;
}

