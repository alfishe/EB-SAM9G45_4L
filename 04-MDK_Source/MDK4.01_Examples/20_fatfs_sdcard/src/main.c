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
#include <utility/trace.h>
#include <utility/assert.h>
#include <utility/math.h>
#include <memories/MEDSdcard.h>

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
/// (Logical drive = physical drive = medium number)
#define MAX_LUNS        1

/// Available medias.
Media medias[MAX_LUNS];

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

#define ID_DRV DRV_MMC

#if _FATFS_TINY == 0
#define STR_ROOT_DIRECTORY "0:"
#else
#define STR_ROOT_DIRECTORY ""
#endif

#if defined(at91cap9stk)
#define MCI_ID 1 //no connector for MCIO/SPI0
#else
#define MCI_ID 0
#endif

const char* FileName = STR_ROOT_DIRECTORY "Basic.bin";

#if 0
//Bigger than this value will cause SD HC error now.
//#define DATA_SIZE (7*1024 + 512)
#define DATA_SIZE (9*1024)
//unsigned char data[DATA_SIZE];
unsigned char * data = (unsigned char*)0x60000400;
#else
#define DATA_SIZE 2064 // size of the file to write/read
                       // !!!minimum size 512 for erase operation !!!

unsigned char data[DATA_SIZE];
#endif

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Do file system tests
/// \return test result, 1: success.
//------------------------------------------------------------------------------
static unsigned char RunFsTest(void)
{
    unsigned int i;
    unsigned int ByteToRead;
    unsigned int ByteRead;
  #if _FATFS_TINY == 0
    unsigned int ByteWritten;
    char key;
  #endif

    FRESULT res;
    DIR dirs;
    FATFS fs;             // File system object
    FIL FileObject;

    // Init Disk
    printf("-I- Please connect a SD card ...\n\r");
    while(!MEDSdcard_Detect(&medias[ID_DRV], MCI_ID));
    printf("-I- SD card connection detected\n\r");

    printf("-I- Init media Sdcard\n\r");
    if (!MEDSdcard_Initialize(&medias[ID_DRV], MCI_ID)) {
        printf("-E- SD Init fail\n\r");
        return 0;
    }
    numMedias = 1;

    // Mount disk
    printf("-I- Mount disk %d\n\r", ID_DRV);
    memset(&fs, 0, sizeof(FATFS));      // Clear file system object
    res = f_mount(ID_DRV, &fs);
    if( res != FR_OK ) {

        printf("-E- f_mount pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }


    // Test if the disk is formated
    res = f_opendir (&dirs,STR_ROOT_DIRECTORY);
    if(res == FR_OK ){
        // erase sdcard to re-format it ?
        printf("-I- The disk is already formated.\n\r");

        // Display the file tree
        printf("-I- Display files contained on the SDcard :\n\r");
        FF_ScanDir(STR_ROOT_DIRECTORY);

      #if _FATFS_TINY == 0
        printf("-I- Do you want to erase the sdcard to re-format disk ? (y/n)!\n\r");

        key = DBGU_GetChar();
        if( (key == 'y') ||  (key == 'Y'))
        {
          for(i=0;i<100;i++) {
              MEDSdcard_EraseBlock(&medias[ID_DRV], i);
          }
          printf("-I- Erase the first 100 blocks complete !\n\r");
          res = FR_NO_FILESYSTEM;
        }
      #endif
    }

    if( res == FR_NO_FILESYSTEM ) {
      #if _FATFS_TINY == 0
        //printf("-I- Press 'y' to start format:\n\r");
        //if (DBGU_GetChar() != 'y') return 0;
        // Format disk
        printf("-I- Format disk %d\n\r", ID_DRV);
        printf("-I- Please wait a moment during formating...\n\r");
        res = f_mkfs(ID_DRV,    // Drv
                        0,    // FDISK partition
                        512); // AllocSize
        printf("-I- Format disk finished !\n\r");
        if( res != FR_OK ) {
            printf("-E- f_mkfs pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
            return 0;
        }
      #else
        printf("-I- Please run Full version FAT FS test first\n\r");
        return 0;
      #endif
    }

  #if _FATFS_TINY == 0
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
    printf("-I- Open the same file : \"%s\"\n\r", FileName);
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
               "Invalid data at data[%u] (expected 0x%02X, read 0x%02X)\n\r",
               i, ((i & 1) == 0) ? (i & 0x55) : (i & 0xAA), data[i]);
    }
    printf("-I- File data Ok !\n\r");

    return 1;
}

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// main
/// FatFs Full version
//------------------------------------------------------------------------------
int main( void )
{
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic FatFS Full Version with SDCard Project %s --\n\r",
           SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    while(1) {
        if (RunFsTest()) {
            printf("-I- Test passed !\n\r");
        }
        else {
            printf("-F- Test Failed !\n\r");
        }
        SD_Stop(MEDSdcard_GetDriver(MCI_ID),
                MEDSdcard_GetDriver(MCI_ID)->pSdDriver);

        printf("\n\r** any key to start test again **\n\r");
        DBGU_GetChar();
    }

    return 0;
}
