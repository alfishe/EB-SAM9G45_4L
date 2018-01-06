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
//         Header
//------------------------------------------------------------------------------

#include <board.h>
#include <irq/irq.h>
#include <pio/pio.h>
#include <dbgu/dbgu.h>
#include <utility/assert.h>
#include <utility/trace.h>
#include <sdmmc/sdmmc_mci.h>

#include <string.h>
#include "tc/tc.h"
#if defined(MCI2_INTERFACE)
#include "dmad/dmad.h"
#endif

//------------------------------------------------------------------------------
//         Local consts
//------------------------------------------------------------------------------
/// Maximum number of blocks read once (for performance test)
#define NB_MULTI_BLOCKS     16

/// Split R/W to 2, first R/W 4 blocks then remaining
#define NB_SPLIT_MULTI      4

/// Test settings
#define TEST_BLOCK_START    (128 * 2)   // From 128K
//#define TEST_BLOCK_END      15100
#define TEST_BLOCK_END      SD_TOTAL_BLOCK(&sdDrv) // Whole SD Card

#define TEST_BLOCK_SKIP     (100 * 1024 * 2)    // 100M

#define TEST_PERFORMENCT_SIZE   (4*1024*1024)

#define TEST_FILL_VALUE_U32     (0x5A6C1439)

/// Number of errors displayed
#define NB_ERRORS       15
#define NB_BAD_BLOCK    200

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

/// MCI driver instance.
static Mci mciDrv;

/// SDCard driver instance.
static SdCard sdDrv;

/// Current SD speed
unsigned int sdSpeed = MCI_INITIAL_SPEED;

/// SD card pins.
static const Pin pinsSd[] = {BOARD_SD_PINS};

#if MCI_BUSY_CHECK_FIX && defined(BOARD_SD_DAT0)
/// SD DAT0 pin
static const Pin pinSdDAT0 = BOARD_SD_DAT0;
#endif
/// Date buffer
#if defined(BOARD_EBI_PSRAM)
static unsigned char * pBuffer = (unsigned char*)(BOARD_EBI_PSRAM
                                                   + 1024*1024
                                                   - SD_BLOCK_SIZE
                                                       * NB_MULTI_BLOCKS);
#else
static unsigned char pBuffer[SD_BLOCK_SIZE * NB_MULTI_BLOCKS];
#endif

static unsigned int nbErrors;

static unsigned int performanceMultiBlock = NB_MULTI_BLOCKS;

static volatile unsigned int tick100u = 0;

/// Test MCI clock speed (in MHz)
static const unsigned char speedList[] = {5, 10, 15, 20, 25, 30, 50, 60};

//------------------------------------------------------------------------------
//         Local macros
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Interrupt handler for timer.
//------------------------------------------------------------------------------
void TC0_IrqHandler(void)
{
    volatile unsigned int dummy;
    // Clear status bit to acknowledge interrupt
    dummy = AT91C_BASE_TC0->TC_SR;

    tick100u ++;
}

//------------------------------------------------------------------------------
/// Configure Timer Counter 0 to generate an interrupt every 0.1ms.
//------------------------------------------------------------------------------
void ConfigureTc0(void)
{
    unsigned int div;
    unsigned int tcclks;

    // Enable peripheral clock
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TC0;

    // Configure TC for a 10000Hz frequency and trigger on RC compare
    TC_FindMckDivisor(10000, BOARD_MCK, &div, &tcclks);
    TC_Configure(AT91C_BASE_TC0, tcclks | AT91C_TC_CPCTRG);
    AT91C_BASE_TC0->TC_RC = (BOARD_MCK / div) / 10000; // timerFreq/desiredFreq

    // Configure and enable interrupt on RC compare
    IRQ_ConfigureIT(AT91C_ID_TC0, 1, TC0_IrqHandler);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    IRQ_EnableIT(AT91C_ID_TC0);

    tick100u = 0;
    TC_Start(AT91C_BASE_TC0);
}

//------------------------------------------------------------------------------
/// MCI0 interrupt handler. Forwards the event to the MCI driver handler.
//------------------------------------------------------------------------------
void MCI0_IrqHandler(void)
{
    MCI_Handler(&mciDrv);
}

//------------------------------------------------------------------------------
//         Optional: SD card detection
//------------------------------------------------------------------------------

#ifdef BOARD_SD_PIN_CD

/// SD card detection pin instance.
static const Pin pinCardDetect = BOARD_SD_PIN_CD;

//------------------------------------------------------------------------------
/// Configure for SD detect pin
//------------------------------------------------------------------------------
static void CardDetectConfigure(void)
{
    PIO_Configure(&pinCardDetect, 1);
}

static unsigned char CardIsConnected(unsigned char slot)
{
    return PIO_Get(&pinCardDetect) ? 0 : 1;			
}
#else

#define CardDetectConfigure()   \
    printf("-I- No Card Detect Pin, default connected\n\r")

#define CardIsConnected(slot)       1
#endif

//------------------------------------------------------------------------------
//         Optional: Write protection status
//------------------------------------------------------------------------------

#ifdef BOARD_SD_PIN_WP

/// Write protection status pin instance.
static const Pin pinMciWriteProtect = BOARD_SD_PIN_WP;

//------------------------------------------------------------------------------
/// Checks if the device is write protected.
//------------------------------------------------------------------------------
void CheckProtection(unsigned char slot)
{
    PIO_Configure(&pinMciWriteProtect, 1);
    if (PIO_Get(&pinMciWriteProtect) != 0) {

        printf("-I- SD card is write-protected\n\r");
    }
    else {

        printf("-I- SD card is NOT write-protected.\n\r");
    }
}

#else

//------------------------------------------------------------------------------
/// Dummy implementation.
//------------------------------------------------------------------------------
void CheckProtection(unsigned char slot)
{
    printf("-I- Cannot check if SD card is write-protected\n\r");
}

#endif

//------------------------------------------------------------------------------
/// Get Input
//------------------------------------------------------------------------------
static char GetDecInput(unsigned char numChar, unsigned int *pInt)
{
    unsigned char key;
    unsigned int  i;
    unsigned int  result = 0;
    for (i = 0; i < numChar;) {
        key = DBGU_GetChar();
        if (key == 27) {
            printf(" Canceled\n\r");
            return key;
        }
        if (key > '9' || key < '0') continue;
        DBGU_PutChar(key);
        result = result * 10 + (key - '0');
        i ++;
    }
    if (pInt) *pInt = result;
    return 0;
}
static char GetHexInput(unsigned char numChar, unsigned int * pInt)
{
    unsigned char key, num;
    unsigned int  i;
    unsigned int  result = 0;
    for (i = 0; i < numChar;) {
        key = DBGU_GetChar();
        if (key == 27) {
            printf(" Canceled\n\r");
            return key;
        }
        if (key >= '0' && key <= '9') num = key - '0';
        else if (key >= 'a' && key <= 'f') num = key - 'a' + 0xA;
        else if (key >= 'A' && key <= 'F') num = key - 'A' + 0xA;
        else continue;
        DBGU_PutChar(key);
        result = (result << 4) + num;
        i ++;
    }
    if (pInt) *pInt = result;
    return 0;
}

//------------------------------------------------------------------------------
/// Get Delayed number of tick
//------------------------------------------------------------------------------
unsigned int GetDelayInTicks(unsigned int startTick, unsigned int endTick)
{
    if (endTick > startTick) return (endTick - startTick);
    return (endTick + (0xFFFFFFFF - startTick));
}


//------------------------------------------------------------------------------
/// Delay ms
//------------------------------------------------------------------------------
void DelayMs(unsigned char ms)
{
    unsigned int oldTick;
    unsigned int cnt;

    while(ms --) {
        for(cnt = 0; cnt < 10; cnt ++) {
            oldTick = tick100u;
            while(oldTick == tick100u);
        }
    }
}

//------------------------------------------------------------------------------
/// Max Error Break
//------------------------------------------------------------------------------
unsigned char MaxErrorBreak(unsigned char halt)
{
    if (NB_ERRORS) {
        if (nbErrors ++ > NB_ERRORS) {

            while(halt);

            nbErrors = 0;
            return 1;
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
/// Dump Splitting row
//------------------------------------------------------------------------------
static void DumpSeperator(void)
{
    printf("\n\r==========================================\n\r");
}

//------------------------------------------------------------------------------
/// Dump main menu
//------------------------------------------------------------------------------
void DumpMenu(void)
{
    DumpSeperator();
    printf("# 0,1,2 : Block read test\n\r");
    printf("# w,W   : Write block test(With data or 0)\n\r");
#if defined(MCI2_INTERFACE)
    printf("# b,B   : eMMC boot mode or access boot partition change\n\r");
#endif
    printf("# i,I   : Re-initialize card\n\r");
    printf("# t     : Disk R/W/Verify test\n\r");
    printf("# T     : Disk performance test\n\r");
    printf("# p     : Change number of blocks in one access for test\n\r");
    printf("# s     : Change MCI Clock for general test\n\r");
  #if defined(MCI2_INTERFACE)
    printf("# h     : Auto Switch to HS mode on init On/Off\n\r");
  #endif
}

//------------------------------------------------------------------------------
/// Dump block & information
//------------------------------------------------------------------------------
void DumpBlock(unsigned char * pData, unsigned int block)
{
    unsigned int i;
    //printf("-I- Block %d: %s .. %s..", block, pData, &pData[8]);
    printf("-I- Block %d: %c .. %c .. %c .. %c..",
            block, pData[0], pData[3], pData[8], pData[8+5]);
    for (i = 0; i < 512; i ++) {
        if((i % 16) == 0) printf("\n\r%3x:", i);
        printf(" %02X", pData[i]);
    }
    printf("\n\r");
}

//------------------------------------------------------------------------------
/// Dump card registers
//------------------------------------------------------------------------------
static void DumpCardInfo(unsigned char slot)
{
    SD_DisplayRegisterCID(&sdDrv);
    SD_DisplayRegisterCSD(&sdDrv);
    SD_DisplayRegisterECSD(&sdDrv);
    SD_DisplayRegisterSCR(&sdDrv);
    SD_DisplaySdStatus(&sdDrv);
}

//------------------------------------------------------------------------------
/// Run boot access test on the inserted card
/// \param slot     The slot used for card accessing.
/// \param newSpeed Target speed.
//------------------------------------------------------------------------------
unsigned char ChangeMciSpeed(unsigned char slot, unsigned int newSpeed)
{
    unsigned char overrun = 0;
    unsigned int  mciDiv;

    MCI_SetSpeed(&mciDrv, newSpeed, 0, BOARD_MCK);
    sdSpeed = MCI_GetSpeed(&mciDrv, &mciDiv);
    printf("-I- MCI DIV -> %d, Speed %d\n\r", mciDiv, sdSpeed);
    if (sdSpeed > sdDrv.transSpeed) {
        overrun = 1;
        printf("-W- Overrun speed!!!!\n\r");
    }

    return overrun;
}

//------------------------------------------------------------------------------
/// Run tests on the inserted card
//------------------------------------------------------------------------------
void CardInit(unsigned char slot)
{
    unsigned char error;
    unsigned int mciSpeed;
    unsigned int mciDiv;

    DumpSeperator();

    MCI_Init(&mciDrv, BOARD_SD_MCI_BASE, BOARD_SD_MCI_ID, BOARD_SD_SLOT);
#if MCI_BUSY_CHECK_FIX && defined(BOARD_SD_DAT0)
    MCI_SetBusyFix(&mciDrv, &pinSdDAT0);
#endif
    error = SD_Init(&sdDrv, (SdDriver *)&mciDrv);
    if (error) {

        printf("-E- SD/MMC card initialization failed: %d\n\r", error);
        return;
    }
    else {
        volatile unsigned int sizeInMB, sizeOfBlock;

        if (SD_TOTAL_SIZE(&sdDrv) == 0xFFFFFFFF) {
            sizeInMB = SD_TOTAL_BLOCK(&sdDrv) / (1024 * 2);
            sizeOfBlock = 512;
        }
        else {
            sizeInMB = SD_TOTAL_SIZE(&sdDrv) / (1024 * 1024);
            sizeOfBlock = SD_TOTAL_SIZE(&sdDrv) / SD_TOTAL_BLOCK(&sdDrv);
        }

        printf("-I- SD/MMC card initialization successful\n\r");
        printf("-I- Card size: %u MB, %u * %dB\n\r",
               sizeInMB, SD_TOTAL_BLOCK(&sdDrv), sizeOfBlock);
    }

    DumpCardInfo(slot);

    mciSpeed = 10000000;
    ChangeMciSpeed(slot, mciSpeed);

    MCI_GetSpeed(&mciDrv, &mciDiv);
    printf("-I- MCK %dK Hz, MCI Speed %dK, divisor %d. \r\n",
           BOARD_MCK/1000, mciSpeed/1000, mciDiv);

}

//------------------------------------------------------------------------------
/// Block test (read)
//------------------------------------------------------------------------------
void BlockTest(unsigned char slot, unsigned int block)
{
    DumpSeperator();

    block *= 256;
    printf("-I- Read Block %d: %d\n\r",
            block, SD_ReadBlock(&sdDrv, block, 1, pBuffer));
    DumpBlock(pBuffer, block);

    block ++;
    printf("-I- Read Block %d: %d\n\r",
            block, SD_ReadBlock(&sdDrv, block, 1, pBuffer));
    DumpBlock(pBuffer, block);
}

//------------------------------------------------------------------------------
/// Block Dump (read)
//------------------------------------------------------------------------------
void BlockDump(unsigned char slot)
{
    unsigned int block;
    DumpSeperator();
    printf("-!- Input block:");
    if (GetDecInput(5, &block))
        return;
    printf("-I- Dump Block %d: %d\n\r",
            block, SD_ReadBlock(&sdDrv, block, 1, pBuffer));
    DumpBlock(pBuffer, block);
}

//------------------------------------------------------------------------------
/// Write test
//------------------------------------------------------------------------------
void WriteTest(unsigned char slot, unsigned char part, unsigned char opt)
{
    unsigned int i, block;
    const char* str[] = {"USER 0",
                         "BOOT 1",
                         "BOOT 2"};
    unsigned char rBuffer[512];

    DumpSeperator();

    printf("-I- Write first 128K with %s\n\r", opt ? "0x00" : "data");


    for (block = 0; block < 256; block ++) {

        // Prepare data
        memset(pBuffer, 0, 512);
        if (opt == 0) {
            sprintf((char*)pBuffer, "%04u", block);
            memcpy(&pBuffer[8], str[part], 6);

            for (i = 16; i < 512; i ++) {
                pBuffer[i] = (unsigned char)i;
            }
        }

        printf("-I- Write Block(%d) %s .. %x ..: %d\n\r",
                block, pBuffer, pBuffer[8],
                SD_WriteBlock(&sdDrv, block, 1, pBuffer));

        memset(rBuffer, 0xFF, 512);
        printf("-I- Read Block(%d):%d, ",
                block,
                SD_ReadBlock(&sdDrv, block, 1, rBuffer));

        printf("Verify ..");
        for (i = 0; i < 512; i ++) {
            if (rBuffer[i] != pBuffer[i]) {
                printf("Fail@ %d ..", i);
                break;
            }
        }
        printf("Done\n\r");
    }
    
}

//------------------------------------------------------------------------------
/// Disk test
//------------------------------------------------------------------------------
void DiskTest(unsigned char slot,
              unsigned char clr,
              unsigned char wr,
              unsigned char rd)
{
    unsigned char error = 0;
    unsigned int i, errcnt = 0;
    unsigned int multiBlock, block, splitMulti;

    DumpSeperator();

    // Perform tests on each block
    multiBlock = 0;
    for (block = TEST_BLOCK_START;
         block < TEST_BLOCK_END;
         block += multiBlock) {

        // Perform different single or multiple bloc operations
        if (++multiBlock > NB_MULTI_BLOCKS)
            multiBlock = 1;

        // Multi-block adjustment
        if (block + multiBlock > TEST_BLOCK_END) {
            multiBlock = TEST_BLOCK_END - block;
        }

        // ** Perform single block or multi block transfer
        printf("\r-I- Testing block [%6u - %6u] ...",
               block, (block + multiBlock -1));

        if (clr) {
            // - Clear the block
            memset(pBuffer, 0, SD_BLOCK_SIZE * multiBlock);
            for (i=0; i < SD_BLOCK_SIZE * multiBlock; i++) {
                if (pBuffer[i] != 0) {
                    printf("\n\r-E- Data @ %u for write : 0x00 <> 0x%02x\n\r",
                           i, pBuffer[i]);
                    if(MaxErrorBreak(0)) return;
                    // Only find first verify error.
                    continue;
                }
            }
            error = SD_WriteBlock(&sdDrv, block, multiBlock, pBuffer);
            if (error) {
                printf("\n\r-E- 1. Write block (%d) #%u\n\r", error, block);
                if(MaxErrorBreak(0)) return;
                // Skip following test
                continue;
            }
            // - Read back the data to check the write operation
            memset(pBuffer, 0xFF, SD_BLOCK_SIZE * multiBlock);
            error = SD_ReadBlock(&sdDrv, block, multiBlock, pBuffer);
            if (error) {
                printf("\n\r-E- 1. Read block (%d) #%u\n\r", error, block);
                if(MaxErrorBreak(0)) return;
                // Skip following test
                continue;
            }
            for (i=0; i < SD_BLOCK_SIZE * multiBlock; i++) {
                if (pBuffer[i] != 0) {
                    printf("\n\r-E- 1. B%u.D[%u] : 0 <> 0x%02X\n\r",
                           block,
                           i, pBuffer[i]);
                    if(MaxErrorBreak(0)) return;
                    // Only find first verify error.
                    break;
                }
            }
        }

        if (wr) {
            // - Write a checkerboard pattern on the block
            for (i=0; i < SD_BLOCK_SIZE * multiBlock; i++) {
                if ((i & 1) == 0)  pBuffer[i] = (i & 0x55);
                else               pBuffer[i] = (i & 0xAA);
            }
            for (i = 0; i < multiBlock; ) {
                splitMulti = ((multiBlock - i) > NB_SPLIT_MULTI) ?
                                        NB_SPLIT_MULTI : (multiBlock - i);
                error = SD_WriteBlock(&sdDrv,
                                      block + i,
                                      splitMulti,
                                      &pBuffer[i * SD_BLOCK_SIZE]);
                if (error) break;
                i += splitMulti;
            }
            ASSERT(i == multiBlock, "Unexpected W, %u!", i);
            if (error) {
                printf("\n\r-E- 2. Write block #%u(%u+%u)\n\r",
                       block+i, block, i);
                if(MaxErrorBreak(0)) return;
                // Skip Following Test
                continue;
            }
        }

        if (rd) {
            // - Read back the data to check the write operation
            memset(pBuffer, 0, SD_BLOCK_SIZE * multiBlock);
            for (i = 0; i < multiBlock; ) {
                splitMulti = ((multiBlock - i) > NB_SPLIT_MULTI) ?
                                        NB_SPLIT_MULTI : (multiBlock - i);
                error = SD_ReadBlock(&sdDrv,
                                     block + i,
                                     splitMulti,
                                     &pBuffer[i * SD_BLOCK_SIZE]);
                if (error) break;
                i += splitMulti;
            }
            ASSERT(i == multiBlock, "Unexpected R, %u!", i);
            if (error) {
                printf("\n\r-E- 2. Read block #%u(%u+%u)\n\r",
                       block + i, block, i);
                if(MaxErrorBreak(0)) return;
                // Skip Following Test
                continue;
            }
            errcnt = 0;
            for (i=0; i < SD_BLOCK_SIZE * multiBlock; i++) {

                if (!(((i & 1) == 0) && (pBuffer[i] == (i & 0x55))) &&
                    !(((i & 1) != 0) && (pBuffer[i] == (i & 0xAA))) ) {
                    unsigned int j, js;
                    printf("\n\r-E- 2.%d. Data @ %u (0x%x)\n\r", errcnt, i, i);
                    printf("  -Src:");
                    js = (i > 8) ? (i - 8) : 0;
                    for (j = js; j < i + 8; j ++)
                        printf(" %02x", ((j & 1)!= 0) ? (j & 0xAA):(j & 0x55));
                    printf("\n\r  -Dat:");
                    for (j = js; j < i + 8; j ++)
                        printf("%c%02x", (i == j) ? '!' : ' ', pBuffer[j]);
                    printf("\n\r");
                    if(MaxErrorBreak(0)) return;
                    // Only find first 3 verify error.
                    if (errcnt ++ >= 3)
                        break;
                }
            }
        }

        if (DBGU_IsRxReady()) {
            switch(DBGU_GetChar()) {
                // Skip 100M
                case 'k':
                    block += TEST_BLOCK_SKIP;
                    if (block > TEST_BLOCK_END) {
                        block -= 5 + multiBlock;
                    }
                    printf("\n\r");
                    break;
                // Cancel
                case 'c':
                    return;
            }
        }
    }

    printf("All block tested!\n\r");
}

//------------------------------------------------------------------------------
/// Run performence test
/// R/W test can be masked to verify previous written data only
//------------------------------------------------------------------------------
void PerformanceTest(unsigned char slot,
                     unsigned char wr,
                     unsigned char rd,
                     unsigned char errDetail)
{
    unsigned char error = 0;
    unsigned char round, pause = 0;
    unsigned int  block, i, nBadBlock = 0, nErrors;
    unsigned int  speed = sdSpeed;
    unsigned int  tickStart, tickEnd, ticks, rwSpeed;

    DumpSeperator();

    printf("-I- Performence test, size %dK, Multi %d, MCK %dMHz\n\r",
                                    TEST_PERFORMENCT_SIZE/1024,
                                    performanceMultiBlock,
                                    BOARD_MCK/1000000);
    printf("-I- RW block by block, block size %d\n\r", SD_BLOCK_SIZE);

    for(round = 0;; round ++) {

        printf("-I- Round %d:\n\r", round);

        if (round > 0) {
            pause = 1;
            printf("-I- (c)ancel? any key to continue:");
        }

        // Selection, to start another loop
        while(pause) {
            // UI, Cancel accepted
            if (DBGU_IsRxReady()) {
                switch(DBGU_GetChar()) {
                    // Cancel
                    case 'c':
                        printf("c\n\r");
                        return;
                }
                printf("\n\r");
                break;
            }
        }

        // Decrease speed
        if (error) {
            while(1) {
                if      (speed >= 50000000) speed = 30000000;
                else if (speed >= 30000000) speed = 25000000;
                else if (speed >= 25000000) speed = 20000000;
                else if (speed >= 20000000) speed = 15000000;
                else if (speed >= 15000000) speed = 10000000;
                else if (speed >= 10000000) speed = 5000000;
                else if (speed >= 5000000)  speed = 2000000;
                else {
                    printf("-!- Fail\n\r");
                    break;
                }
                ChangeMciSpeed(slot, speed);
                speed = sdSpeed;
            }
        }
        printf("-I- MCI Clock %d\n\r", speed);

        if (wr) {
            printf("--- Write test .. ");
            for (i = 0; i < SD_BLOCK_SIZE * performanceMultiBlock; i += 4) {
                *(unsigned int*)&pBuffer[i] = TEST_FILL_VALUE_U32;
            }
            nBadBlock = 0;
            tickStart = tick100u;
            for (block = TEST_BLOCK_START;
                 block < (TEST_PERFORMENCT_SIZE/SD_BLOCK_SIZE)
                            + TEST_BLOCK_START;
                 block += performanceMultiBlock) {

                *(unsigned int*)pBuffer = block;
                error = SD_WriteBlock(&sdDrv,
                                      block, performanceMultiBlock,
                                      pBuffer);
                if (error) {
                    if (nBadBlock ++ >= NB_BAD_BLOCK) {
                        printf("-E- WR_B(%u)\n\r", block);
                        break;
                    }
                    else error = 0;
                }
            }
            if (error) continue;
            tickEnd = tick100u;
            ticks = GetDelayInTicks(tickStart, tickEnd);
            rwSpeed = (TEST_PERFORMENCT_SIZE
                        - nBadBlock * performanceMultiBlock * SD_BLOCK_SIZE)
                            * 10 / ticks;
            printf("Done, Bad %u, Speed %uK\n\r", nBadBlock, rwSpeed);
        }

        if (rd) {
            printf("--- Read test .. ");
            nBadBlock = 0;
            tickStart = tick100u;
            for (block = TEST_BLOCK_START;
                 block < (TEST_PERFORMENCT_SIZE/SD_BLOCK_SIZE)
                            + TEST_BLOCK_START;
                 block += performanceMultiBlock) {

                error = SD_ReadBlock(&sdDrv,
                                     block, performanceMultiBlock,
                                     pBuffer);
                if (error) {
                    if (nBadBlock ++ >= NB_BAD_BLOCK) {
                        printf("-E- RD_B(%u)\n\r", block);
                        break;
                    }
                    else error = 0;
                }
                if (error) break;
            }
            if (error) continue;
            tickEnd = tick100u;
            ticks = GetDelayInTicks(tickStart, tickEnd);
            rwSpeed = (TEST_PERFORMENCT_SIZE
                        - nBadBlock * performanceMultiBlock * SD_BLOCK_SIZE)
                            * 10 / ticks;
            printf("Done, Bad %u, Speed %uK\n\r", nBadBlock, rwSpeed);
        }

        printf("--- Data verify .. ");
        nErrors = 0;
        for (block = TEST_BLOCK_START;
             block < (TEST_PERFORMENCT_SIZE/SD_BLOCK_SIZE) + TEST_BLOCK_START;
             block += performanceMultiBlock) {

            memset(pBuffer, 0x00, SD_BLOCK_SIZE * performanceMultiBlock);
            error = SD_ReadBlock(&sdDrv,
                                 block, performanceMultiBlock,
                                 pBuffer);
            if (error) {
                printf("-E- RD_B(%u)\n\r", block);
                break;
            }
            if (*(unsigned int*)pBuffer != block) {
                if (errDetail) {
                    if (nErrors ++ < NB_ERRORS) {
                        printf("-E- Blk(%u)[0](%08x<>%08x)\n\r",
                               block, block, *(unsigned int*)pBuffer);
                    }
                }
                else {
                    printf("-E- BlkN(%x<>%x)\n\r",
                           block, *(unsigned int*)pBuffer);
                    error = 1;
                    break;
                }
            }
            for (i = 4; i < SD_BLOCK_SIZE * performanceMultiBlock; i += 4) {
                if ( (*(unsigned int*)&pBuffer[i]) != TEST_FILL_VALUE_U32) {
                    if (errDetail) {
                        // Dump 10 errors only
                        if (nErrors ++ < NB_ERRORS) {
                            unsigned int j;
                            printf("-E- Blk(%u)[%u](%08x.. <>",
                                    block, i, TEST_FILL_VALUE_U32);
                            for (j = (i > 4) ? (i - 4) : i;
                                 j <= i + 4;
                                 j += 4) {
                                printf("%c%08X",
                                        (i == j) ? '!' : ' ',
                                        *(unsigned int*)&pBuffer[j]);
                            }
                            printf(")\n\r");
                        }
                    }
                    else {
                    printf("-E- Blk(%u)[%u](%x<>%x)\n\r", block, i,
                            TEST_FILL_VALUE_U32,
                            *(unsigned int*)&pBuffer[i]);
                    error = 1;
                    break;
                    }
                }
            }
            if (error) break;
        }
        if (errDetail && nErrors) {
            printf("-I- %u u32 ERRORS found!\n\r", nErrors);
        }
        // Continue to next round if any error happened
        if (error) continue;
        printf("OK\n\r");

        break;
    }
}


#if defined(MCI2_INTERFACE)&& defined(AT91C_MCI_SPCMD_BOOTREQ)
//------------------------------------------------------------------------------
/// Change the partition to access (USER/BOOT 1/BOOT 2)
/// \param slot     The slot used for card accessing.
/// \param accPart  Target partition to access.
//------------------------------------------------------------------------------
unsigned char ChangeAccPartition(unsigned char slot, unsigned char accPart)
{
    unsigned char success = 1;

    DumpSeperator();

    printf("-I- Boot access configure to %d:\n\r", accPart);
    if (MMC_SetupBootMode(&sdDrv, 0, 1, 0, accPart, 0)) {
        printf("-E- Can not set to boot access %d\n\r", accPart);
        return 0;
    }
    MCI_SetSpeed(&mciDrv, MCI_INITIAL_SPEED, sdDrv.transSpeed, BOARD_MCK);
    if(SD_Init(&sdDrv, (SdDriver *)&mciDrv)) {
        printf("-E- Re-init error\n\r");
        success = 0;
    }
    MCI_SetSpeed(&mciDrv, sdSpeed, sdDrv.transSpeed, BOARD_MCK);
    return success;
}

#define PIN_MCI0_CMD0 {1 <<  1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_MCI0_CMD  {1 <<  1, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_MCI1_CMD0 {1 << 22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_MCI1_CMD  {1 << 22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
static const Pin pinMmcCmd0[] = {PIN_MCI0_CMD0, PIN_MCI1_CMD0};
static const Pin pinMmcCmd[] =  {PIN_MCI0_CMD, PIN_MCI1_CMD};

//------------------------------------------------------------------------------
/// Run boot tests on the inserted card
//------------------------------------------------------------------------------
void BootTest(unsigned char slot, unsigned char bootPart)
{
    unsigned int i;

    DumpSeperator();

    printf("-I- Boot mode configure:\n\r");
    if (MMC_SetupBootMode(&sdDrv, 0, 1, bootPart, 0, 0)) {
        printf("-E- Fail to set boot configure\n\r");
        //return;
    }

    PIO_Configure(pinMmcCmd0, PIO_LISTSIZE(pinMmcCmd0));

    printf("-I- Waiting for card disconnect\n\r");
    while(CardIsConnected(slot)) {
        if(DBGU_IsRxReady()) {
            DBGU_GetChar();
            break;
        }
    }
    printf("-I- Waiting for card connect\n\r");
    while(!CardIsConnected(slot)) {
        if(DBGU_IsRxReady()) {
            DBGU_GetChar();
            break;
        }
    }

    DelayMs(200);
    PIO_Clear(&pinMmcCmd0[0]); PIO_Clear(&pinMmcCmd0[1]);

    MCI_SetSpeed(&mciDrv, MCI_INITIAL_SPEED, 0, BOARD_MCK);
    printf("-I- Boot init : %d\n\r", MMC_BootInit(&sdDrv));

    for(i = 0; i < 3; i ++) {
        memset(pBuffer, 0, 512);
        printf("-I- Read: %d\n\r", MMC_BootRead(&sdDrv, 1, pBuffer));
        DumpBlock(pBuffer, i);
    }

    printf("-I- End: %d\n\r", MMC_BootStop(&sdDrv));


    PIO_Configure(pinMmcCmd, PIO_LISTSIZE(pinMmcCmd));
    printf("--- ReInit: %d\n\r", SD_Init(&sdDrv, (SdDriver *)&mciDrv));
    MCI_SetSpeed(&mciDrv, sdSpeed, 0, BOARD_MCK);
}
#endif

//------------------------------------------------------------------------------
/// 
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Main function
//------------------------------------------------------------------------------
int main(void)
{
    unsigned int i;
    unsigned char connected = 0;
    unsigned char speedNdx = 0, accPart = 0;
  #if defined(MCI2_INTERFACE)
    unsigned char nextAccPart = 0;
    unsigned char hsMode = 0, mciHsEnable = 0;
  #endif

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic SD/MMC MCI Mode Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    printf("-- Buffer@%x,size 0x%x\n\r",
           (unsigned int)pBuffer, sizeof(pBuffer));

    // Configure SDcard pins
    PIO_Configure(pinsSd, PIO_LISTSIZE(pinsSd));

    // Wait for SD card connection (if supported)
    CardDetectConfigure();

    // Check if card is write-protected (if supported)
    CheckProtection(0);

    // Initialize the SD card driver
    // Initialize the MCI driver
    #if defined(MCI2_INTERFACE)
    DMAD_Initialize(BOARD_MCI_DMA_CHANNEL, DMAD_NO_DEFAULT_IT);
    #endif
    IRQ_ConfigureIT(BOARD_SD_MCI_ID,  1, MCI0_IrqHandler);
    IRQ_EnableIT(BOARD_SD_MCI_ID);
    //printf("-I- TC Start ... ");
    ConfigureTc0();
    i = tick100u;
    while(i == tick100u);
	// Card insert detection loop
    for(;;) {
        if (CardIsConnected(0)) {
            if (connected == 0) {
                connected = 1;

                // Delay before card initialize
                DelayMs(200);

                // Do card test
                CardInit(0);
              #if defined(MCI2_INTERFACE)
                hsMode = SD_HighSpeedMode(&sdDrv, 0xFF);
                mciHsEnable = 0;
              #endif
            }
        }
        else if (connected) {
            connected = 0;
            printf("** Card Disconnected\n\r");
            SD_Stop(&sdDrv, (SdDriver *)&mciDrv);
        }

        if (connected) {

            if (DBGU_IsRxReady()) {
	                switch(DBGU_GetChar()) {
                    case '0':  BlockTest(0, 0);             break;
                    case '1':  BlockTest(0, 1);             break;
                    case '2':  BlockTest(0, 2);             break;
                    case 'd':  BlockDump(0);                break;
                    case 'w':  WriteTest(0, accPart, 0);    break;
                    case 'W':  WriteTest(0, accPart, 1);    break;

#if defined(MCI2_INTERFACE) && defined(AT91C_MCI_SPCMD_BOOTREQ)
                    // Run boot test
                    case 'b':
                        BootTest(0, 1);
                        break;

                    // Change access partition
                    case 'B':
                        //nextAccPart = accPart;
                        nextAccPart ++;
                        if (nextAccPart >= 3) nextAccPart = 0;
                        if(ChangeAccPartition(0, nextAccPart))
                            accPart = nextAccPart;
                        break;
#endif

                    // Initialize the card again
                    case 'I':  case 'i':
                        SD_Stop(&sdDrv, (SdDriver *)&mciDrv);
                        CardInit(0);
                      #if defined(MCI2_INTERFACE)
                        hsMode = SD_HighSpeedMode(&sdDrv, 0xFF);
                        mciHsEnable = 0;
                      #endif
                        break;

                    // Run test on whole disk
                    case 't':
                        DiskTest(0, 1, 1, 1);
                        printf("\n\r");
                        break;

                    // Run performence test
                    case 'T':
                        PerformanceTest(0, 1, 1, 0);
                        printf("\n\r");
                        break;

                    // Read/Verify ONLY test
                    case 'v':
                        DiskTest(0, 0, 0, 1);
                        printf("\n\r");
                        break;
                    case 'V':
                        PerformanceTest(0, 0, 1, 1);
                        printf("\n\r");
                        break;

                    // HS mode switch
                  #if defined(MCI2_INTERFACE)
                    case 'H':
                        if (0 == SD_HighSpeedMode(&sdDrv, !hsMode)) {
                            hsMode = !hsMode;
                            printf("-!- Card HS Mode -> %d\n\r", hsMode);
                            DumpCardInfo(0);
                        }
                        break;
                    case 'h':
                        MCI_EnableHsMode(&mciDrv, !mciHsEnable);
                        mciHsEnable = !mciHsEnable;
                        printf("-!- MCI HS Mode -> %d\n\r", mciHsEnable);
                        break;
                  #endif

                    // Change card speed
                    case 's':
                    {   unsigned int newSpeed = speedList[speedNdx] * 1000000;
                        if (ChangeMciSpeed(0, newSpeed)) {
                            speedNdx = 0;
                        }
                        else {
                            speedNdx ++;
                            if (speedNdx > sizeof(speedList)) speedNdx = 0;
                        }
                        printf("-!- Change MCI CLOCK -> %d\n\r", sdSpeed);
                    }
                        break;

                    // Change performance test block size
                    case 'p':
                    {   if (performanceMultiBlock >= NB_MULTI_BLOCKS)
                            performanceMultiBlock = 1;
                        else
                            performanceMultiBlock <<= 1;
                        printf("-!- Performance Multi set to %d\n\r",
                            performanceMultiBlock);
                    }
                        break;

                    // Show help information
                    default:    DumpMenu();
                }
            }
        }
    }

    return 0;
}

