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
#include <board_memories.h>
#include <pio/pio.h>
#include <utility/trace.h>
#include <utility/math.h>
#include <memories/nandflash/SkipBlockNandFlash.h>

#include <string.h>

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------

/// Effective master clock for applets.
#define MCK     48000000
#if defined(at91sam9g45ek) || defined(at91sam9m10ek)
#define BUFFER_ADDRESS (AT91C_DDR2 + 0x00100000)
#elif defined(at91sam3uek)
#define BUFFER_ADDRESS (BOARD_EBI_PSRAM)
#else
#define BUFFER_ADDRESS (AT91C_EBI_SDRAM + 0x00100000)
#endif

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

/// Nandflash memory size.
static unsigned int memSize;
/// Number of blocks in nandflash.
static unsigned int numBlocks;
/// Size of one block in the nandflash, in bytes.
static unsigned int blockSize;
/// Size of one page in the nandflash, in bytes.
static unsigned int pageSize;
/// Number of page per block
static unsigned int numPagesPerBlock;
// Nandflash bus width
static unsigned char nfBusWidth = 16;


#ifdef PINS_NANDFLASH

/// Pins used to access to nandflash.
static const Pin pPinsNf[] = {PINS_NANDFLASH};
/// Nandflash device structure.
static struct SkipBlockNandFlash skipBlockNf;
/// Address for transferring command bytes to the nandflash.
static unsigned int cmdBytesAddr = BOARD_NF_COMMAND_ADDR;
/// Address for transferring address bytes to the nandflash.
static unsigned int addrBytesAddr = BOARD_NF_ADDRESS_ADDR;
/// Address for transferring data bytes to the nandflash.
static unsigned int dataBytesAddr = BOARD_NF_DATA_ADDR;
/// Nandflash chip enable pin.
static const Pin nfCePin = BOARD_NF_CE_PIN;
/// Nandflash ready/busy pin.
static const Pin nfRbPin = BOARD_NF_RB_PIN;


#else

/// Pins used to access to nandflash.
static const Pin pPinsNf[] = {{0, 0, 0, 0, 0}};
/// Nandflash device structure.
static struct SkipBlockNandFlash skipBlockNf;
/// Address for transferring command bytes to the nandflash.
static unsigned int cmdBytesAddr = 0;
/// Address for transferring address bytes to the nandflash.
static unsigned int addrBytesAddr = 0;
/// Address for transferring data bytes to the nandflash.
static unsigned int dataBytesAddr = 0;
/// Nandflash chip enable pin.
static const Pin nfCePin = {0, 0, 0, 0, 0};
/// Nandflash ready/busy pin.
static const Pin nfRbPin = {0, 0, 0, 0, 0};

#endif


//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Applet main entry. This function decodes received command and executes it.
/// \param argc  always 1
/// \param argv  Address of the argument area.
//------------------------------------------------------------------------------
int main()
{
    unsigned char testFailed;
    // Temporary buffer used for non block aligned read / write
    unsigned char * pBuffer;
    unsigned short block;
    unsigned int i;
    // Errors returned by SkipNandFlash functions
    unsigned char error = 0;

    // Configure the DBGU
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic NandFlash Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

#if !defined(ddram) && defined(BOARD_DDRAM_SIZE)
    BOARD_ConfigureDdram(0, BOARD_DDRAM_BUSWIDTH);
#endif

#if !defined(sdram) && defined(BOARD_SDRAM_SIZE)
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);
#endif

#if !defined(psram) && defined(BOARD_PSRAM_SIZE)
    BOARD_ConfigurePsram();
#endif

    // Configure SMC for Nandflash accesses (done each time applet is launched because of old ROM codes)
    BOARD_ConfigureNandFlash(nfBusWidth);
    PIO_Configure(pPinsNf, PIO_LISTSIZE(pPinsNf));

    memset(&skipBlockNf, 0, sizeof(skipBlockNf));

    if (SkipBlockNandFlash_Initialize(&skipBlockNf,
                                              0,
                                              cmdBytesAddr,
                                              addrBytesAddr,
                                              dataBytesAddr,
                                              nfCePin,
                                              nfRbPin)) {

        TRACE_ERROR("\tDevice Unknown\n\r");
        return 0;
    }

    // Check the data bus width of the NandFlash
    nfBusWidth = NandFlashModel_GetDataBusWidth((struct NandFlashModel *)&skipBlockNf);
	printf("BusWidth=%d\r\n", nfBusWidth);
    // Reconfigure bus width
    BOARD_ConfigureNandFlash(nfBusWidth);

    TRACE_INFO("\tNandflash driver initialized\n\r");

    // Get device parameters
    memSize = NandFlashModel_GetDeviceSizeInBytes(&skipBlockNf.ecc.raw.model);
    blockSize = NandFlashModel_GetBlockSizeInBytes(&skipBlockNf.ecc.raw.model);
    numBlocks = NandFlashModel_GetDeviceSizeInBlocks(&skipBlockNf.ecc.raw.model);
    pageSize = NandFlashModel_GetPageDataSize(&skipBlockNf.ecc.raw.model);
    numPagesPerBlock = NandFlashModel_GetBlockSizeInPages(&skipBlockNf.ecc.raw.model);

    TRACE_INFO("Size of the whole device in bytes : 0x%x \n\r",memSize);
    TRACE_INFO("Size in bytes of one single block of a device : 0x%x \n\r",blockSize);
    TRACE_INFO("Number of blocks in the entire device : 0x%x \n\r",numBlocks);
    TRACE_INFO("Size of the data area of a page in bytes : 0x%x \n\r",pageSize);
    TRACE_INFO("Number of pages in the entire device : 0x%x \n\r",numPagesPerBlock);
    TRACE_INFO("Bus width : 0x%x \n\r",nfBusWidth);

    // Test all blocks
    testFailed = 0;
    block = 0;
    pBuffer = (unsigned char *) BUFFER_ADDRESS;

    while (!testFailed && (block < numBlocks)) {
        TRACE_INFO("Test in progress on block: %6d\r", block);

        // Erase block
        error = SkipBlockNandFlash_EraseBlock(&skipBlockNf, block, NORMAL_ERASE);
        if (error == NandCommon_ERROR_BADBLOCK) {
            TRACE_INFO("Skip bad block %6d: \n\r", block);
            block++;
            continue;
        }

        // Verify that block has been erased correctly
        memset(pBuffer, 0, blockSize);
        SkipBlockNandFlash_ReadBlock(&skipBlockNf, block, pBuffer);
        for (i=0; i < blockSize; i++) {
            if (pBuffer[i] != 0xff) {
                TRACE_ERROR("Could not erase block %d\n\r", block);
                testFailed = 1;
                break;
            }
        }

        // Write block
        for (i=0; i < blockSize; i++) {

            pBuffer[i] = i & 0xFF;
        }
        // Write target block
        SkipBlockNandFlash_WriteBlock(&skipBlockNf, block, pBuffer);

        // Check that data has been written correctly
        memset(pBuffer, 0, blockSize);
        SkipBlockNandFlash_ReadBlock(&skipBlockNf, block, pBuffer);

        for (i=0; i < blockSize; i++) {
            if (pBuffer[i] != (i & 0xFF)) {
                TRACE_ERROR("Could not write block %d\n\r", block);
                testFailed = 1;
                break;
            }
        }
        block++;
    }
    // Display test result
    if (testFailed) {

        TRACE_ERROR("Test failed.\n\r");
    }
    else {

         TRACE_INFO("Test passed.\n\r");
    }

    return 0;
}

