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

#include <stdio.h>
#include <string.h>
#include <board.h>
#include <board_memories.h>
#include <utility/trace.h>
#include <utility/assert.h>
#if defined(BOARD_SDRAM_SIZE) || defined (BOARD_DDRAM_SIZE)
#include <memories/MEDSdram.h>
#include <memories/MEDDdram.h>
#elif defined(BOARD_PSRAM_SIZE)
#include <memories/MEDRamDisk.h>
#endif

#include "fatfs_config.h"
#if _FATFS_TINY != 1
#include <drivers\fat\fatfs\src\ff.h>
#else
#include <drivers\fat\fatfs\src\tff.h>
#endif
#include <drivers\fat\fatfs\src\ff_util.h>


//------------------------------------------------------------------------------
//         Local constants
//------------------------------------------------------------------------------
/// Maximum number of LUNs which can be defined.
#define MAX_LUNS        1

/// Size of bytes in a block
#define BLOCK_SIZE      512

/// Size of the RAM Disk in bytes.
#define RAMDISK_SIZE    (1024*1024)

/// Size of buffer used for create and test a file
#define DATA_SIZE       512

#if defined(sdram) || defined(ddram) || defined(psram)
// The code is launch in sdram
#define CODE_SIZE           50*1024
#else
// Entire sdram is reserved for the FAT
#define CODE_SIZE           0
#endif

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

#if _FATFS_TINY == 0
#define STR_ROOT_DIRECTORY "0:"
#else
#define STR_ROOT_DIRECTORY ""
#endif

/// Available medias.
Media medias[MAX_LUNS];

/// File name
const char* FileName = STR_ROOT_DIRECTORY "Basic.bin";

/// Buffer for create and use a file
unsigned char data[DATA_SIZE];

//------------------------------------------------------------------------------
/// main
//------------------------------------------------------------------------------
int main( void )
{
    unsigned int i;
    unsigned int ByteToRead;
    unsigned int ByteRead;
    unsigned int ByteWritten;
    FATFS fs;             // File system object
    FIL FileObject;
    FRESULT res;

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
#if _FATFS_TINY != 1
    printf("-- Basic FatFS Full Version with External RAM Project %s --\n\r", SOFTPACK_VERSION);
#else
    printf("-- Basic FatFS Tiny Version with External RAM Project %s --\n\r", SOFTPACK_VERSION);
    printf("(Full Version has to be launched before to pass test)\n\r");
#endif
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

#if !defined(ddram) && defined(PINS_DDRAM)
    //configure DDRAM for use
    BOARD_ConfigureDdram(0, BOARD_DDRAM_BUSWIDTH);
#endif

    // Init Disk
#if defined(BOARD_SDRAM_SIZE)
    MEDSdram_Initialize(&(medias[0]),
                        BLOCK_SIZE,
                        (unsigned int)(AT91C_EBI_SDRAM + CODE_SIZE)/BLOCK_SIZE,
                        RAMDISK_SIZE / BLOCK_SIZE);
#elif defined (BOARD_DDRAM_SIZE)
    MEDDdram_Initialize(&(medias[0]),
                        BLOCK_SIZE,
                        (unsigned int)(AT91C_DDR2 + CODE_SIZE) / BLOCK_SIZE,
                        RAMDISK_SIZE / BLOCK_SIZE);
#elif defined (BOARD_PSRAM_SIZE)
   BOARD_ConfigurePsram();
   MEDRamDisk_Initialize(&(medias[0]),
                         BLOCK_SIZE,
                         (unsigned int)(BOARD_EBI_PSRAM + CODE_SIZE) / BLOCK_SIZE,
                         RAMDISK_SIZE / BLOCK_SIZE);

#else
    #error Pb no sdram
#endif

    numMedias = 1;

    // Mount Disk
    printf("-I- Mount disk 0\n\r");
    memset(&fs, 0, sizeof(FATFS));  // Clear file system object
    res = f_mount(0, &fs);
    if( res != FR_OK ) {
        printf("-E- f_mount pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }

#if _FATFS_TINY != 1
    // Format disk
    printf("-I- Format disk 0\n\r");
    printf("-I- Please wait a moment during formating...\n\r");
    res = f_mkfs(0,    // Drv
                 0,    // FDISK partition
                 512); // AllocSize
    printf("-I- Format disk finished !\n\r");
    if( res != FR_OK ) {
        printf("-E- f_mkfs pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }

    // Create a new file
    printf("-I- Create a file : \"%s\"\n\r", FileName);
    res = f_open(&FileObject, FileName, FA_CREATE_ALWAYS|FA_WRITE);
    if( res != FR_OK ) {
        printf("-E- f_open create pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }

    // Write a checkerboard pattern in the buffer
    for (i=0; i < sizeof(data); i++) {
        if ((i & 1) == 0) {
            data[i] = (i & 0x55);
        }
        else {
            data[i] = (i & 0xAA);
        }
    }
    printf("-I- Write file\n\r");
    res = f_write(&FileObject, data, DATA_SIZE, &ByteWritten);
    printf("-I- ByteWritten=%d\n\r", (int)ByteWritten);
    if( res != FR_OK ) {
        printf("-E- f_write pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }
    else {
        printf("-I- f_write ok: ByteWritten=%d\n\r", (int)ByteWritten);
    }

    // Close the file
    printf("-I- Close file\n\r");
    res = f_close(&FileObject);
    if( res != FR_OK ) {
        printf("-E- f_close pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }
#endif

    // Open the file
    printf("-I- Open file : %s\n\r", FileName);
    res = f_open(&FileObject, FileName, FA_OPEN_EXISTING|FA_READ);
    if( res != FR_OK ) {
        printf("-E- f_open read pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }

    ASSERT( FileObject.fsize == DATA_SIZE,  "File size value not expected!\n\r");

    // Read file
    printf("-I- Read file\n\r");
    memset(data, 0, DATA_SIZE);
    ByteToRead = FileObject.fsize;

    res = f_read(&FileObject, data, ByteToRead, &ByteRead);
    if(res != FR_OK) {
        printf("-E- f_read pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }
    ASSERT( FileObject.fsize == DATA_SIZE,  "File size value not expected!\n\r");

    // Close the file
    printf("-I- Close file\n\r");
    res = f_close(&FileObject);
    if( res != FR_OK ) {
        printf("-E- f_close pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }

    // compare read data with the expected data
    for (i=0; i < sizeof(data); i++) {
        ASSERT((((i & 1) == 0) && (data[i] == (i & 0x55)))
               || (data[i] == (i & 0xAA)),
               "\n\r-F- Invalid data at data[%u] (expected 0x%02X, read 0x%02X)\n\r",
               i, ((i & 1) == 0) ? (i & 0x55) : (i & 0xAA), data[i]);
    }
    printf("-I- File data Ok !\n\r");

    printf("-I- Test passed !\n\r");

    return 0;
}

