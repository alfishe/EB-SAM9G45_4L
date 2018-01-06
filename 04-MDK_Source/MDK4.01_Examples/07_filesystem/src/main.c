/***************************************************************************************
* File Name          : main.c
* Date First Issued  : 01/10/2010
* Description        : Main program body
***************************************************************************************
***************************************************************************************
* History:
* 01/10/2010:          V1.0
**************************************************************************************/

//-----------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>

#include <board.h>
#include <board_lowlevel.h>
#include <board_memories.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <pit/pit.h>
#include <irq/irq.h>
#include <dbgu/dbgu.h>
#include <utility/trace.h>
#include <utility/assert.h>
#include <utility/led.h>
#include <usb/common/core/USBConfigurationDescriptor.h>
#include <usb/device/core/USBD.h>
#include <usb/device/massstorage/MSDDriver.h>
#include <usb/device/massstorage/MSDLun.h>
#include <usb/device/core/USBDCallbacks.h>
#include <memories/Media.h>
#include <memories/MEDFlash.h>
#include <memories/MEDSdram.h>
#include <memories/MEDDdram.h>

//------------------------------------------------------------------------------
//         Internal definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// EFSL, to avoid "BOOL" definition confliction on compiling
#define _INTEGER
/* These types must be 16-bit, 32-bit or larger integer */
typedef int             INT;
typedef unsigned int    UINT;

/* These types must be 8-bit integer */
typedef signed char     CHAR;
typedef unsigned char   UCHAR;
typedef unsigned char   BYTE;

/* These types must be 16-bit integer */
typedef short           SHORT;
typedef unsigned short  USHORT;
typedef unsigned short  WORD;

/* These types must be 32-bit integer */
typedef long            LONG;
typedef unsigned long   ULONG;
typedef unsigned long   DWORD;
#include <efs.h>
#include <fs.h>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// FAT FS
#include "fatfs_config.h"
#if _FATFS_TINY != 1
#include <fatfs/src/ff.h>
#else
#include <fatfs/src/tff.h>
#endif
//------------------------------------------------------------------------------

/// Maximum number of LUNs which can be defined.
#define MAX_LUNS            3

/// Delay for pushbutton debouncing (ms)
#define DEBOUNCE_TIME       10

/// PIT period value (seconds)
#define PIT_PERIOD          1000

/// Maximum code size reserved for running in SDRAM and FLASH
#define CODE_SIZE           128*1024

/// Size of one block in bytes.
#define BLOCK_SIZE          512

/// Size of the RAM Disk in bytes.
#define RAMDISK_SIZE        (10*1024*1024)

/// Size of the MSD IO buffer in bytes (2K, more the better).
#define MSD_BUFFER_SIZE     (4*BLOCK_SIZE)

/// Name of the test file.
#define FILE_1              "test.bin"
#define FILE_2              "copy.bin"

/// Size of the test file.
#define TESTFILE_SIZE       (4*1024*1024)

/// Media index for memories
#define RAMDISK_I           0
#define SDCARD_I            1

//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------

/// File System Functions for test
typedef struct _FSFunctions {

    /// Function to Initialize and Mount the File System
    int (*fs_init)(void);
    /// Function to format the Disk
    int (*fs_format)(void);
    /// Function to Open a File
    int (*fs_open)(void* file, const char* path, unsigned char forWrite);
    /// Function to Write to a File
    int (*fs_write)(void* file, const void* buffer, int size);
    /// Function to Read from a File
    int (*fs_read)(void* file, void* buffer, int size);
    /// Function to Close an openned File
    int (*fs_close)(void* file);
} FSFunctions;

/// File Object union for File System general test
typedef union _FileObj {

    /// FAT FS File Object
    FIL             fatfsObj;
    /// EFSL File Object
    EmbeddedFile    efslObj;
} FileObj;

//------------------------------------------------------------------------------
//         Exported variables
//------------------------------------------------------------------------------

/// Available medias.
Media medias[MAX_LUNS];

//------------------------------------------------------------------------------
//         Internal variables
//------------------------------------------------------------------------------

/// Device LUNs.
static MSDLun luns[MAX_LUNS];

/// LUN read/write buffer.
static unsigned char msdBuffer[BLOCK_SIZE];

/// Time Tick 1ms
static unsigned int timeTick;

/// EFSL Used Data
static EmbeddedFileSystem efs;

/// FAT FS Used Data
static FATFS fatFs;

//------------------------------------------------------------------------------
//         PIT Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Handler for PIT interrupt. Increments the timeTick counter.
//------------------------------------------------------------------------------
static void ISR_Pit(void)
{
    unsigned long pisr = 0;

    // Read the PISR
    pisr = PIT_GetStatus() & AT91C_PITC_PITS;

    if (pisr != 0) {

        // Read the PIVR. It acknowledges the IT
        PIT_GetPIVR();
    }

    timeTick ++;
}

//------------------------------------------------------------------------------
/// Configures the PIT to generate 1ms ticks.
//------------------------------------------------------------------------------
static void ConfigurePit(void)
{
    // Initialize and enable the PIT
    PIT_Init(PIT_PERIOD, BOARD_MCK / 1000000);

    // Disable the interrupt on the interrupt controller
    IRQ_DisableIT(AT91C_ID_SYS);

    // Reset time tick
    timeTick = 0;

    // Configure the AIC for PIT interrupts
    IRQ_ConfigureIT(AT91C_ID_SYS, 0, ISR_Pit);

    // Enable the interrupt on the interrupt controller
    IRQ_EnableIT(AT91C_ID_SYS);

    // Enable the interrupt on the pit
    PIT_EnableIT();

    // Enable the pit
    PIT_Enable();
}

//------------------------------------------------------------------------------
//         VBus monitoring (optional)
//------------------------------------------------------------------------------
#if defined(PIN_USB_VBUS)

#define VBUS_CONFIGURE()  VBus_Configure()

/// VBus pin instance.
static const Pin pinVbus = PIN_USB_VBUS;

//------------------------------------------------------------------------------
/// Handles interrupts coming from PIO controllers.
//------------------------------------------------------------------------------
static void ISR_Vbus(const Pin *pPin)
{
    // Check current level on VBus
    if (PIO_Get(&pinVbus)) {

        TRACE_INFO("VBUS conn\n\r");
        USBD_Connect();
    }
    else {

        TRACE_INFO("VBUS discon\n\r");
        USBD_Disconnect();
    }
}

//------------------------------------------------------------------------------
/// Configures the VBus pin to trigger an interrupt when the level on that pin
/// changes.
//------------------------------------------------------------------------------
static void VBus_Configure( void )
{
    TRACE_INFO("VBus configuration\n\r");

    // Configure PIO
    PIO_Configure(&pinVbus, 1);
    PIO_ConfigureIt(&pinVbus, ISR_Vbus);
    PIO_EnableIt(&pinVbus);

    // Check current level on VBus
    if (PIO_Get(&pinVbus)) {

        // if VBUS present, force the connect
        TRACE_INFO("conn\n\r");
        USBD_Connect();
    }
    else {
        USBD_Disconnect();
    }           
}

#else
    #define VBUS_CONFIGURE()    USBD_Connect()
#endif //#if defined(PIN_USB_VBUS)

//------------------------------------------------------------------------------
///        File System Test Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Mount EFSL file system
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int EFSL_mount(void)
{
    if (efs_init(&efs, (eint8*)&medias[0]) != 0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
/// Format disk with EFSL.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int EFSL_format(void)
{
    efs_init(&efs, (eint8*)&medias[0]);
    fs_initCurrentDir(&efs.myFs);
    if (mkfs_makevfat(&efs.myPart) != 0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
/// Open a file with EFSL file system
/// \param file     Pointer to a file object instance.
/// \param path     Pointer to the string that describes the path of file
/// \param forWrite 1 to create file for writing, 0 to open file for reading
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int EFSL_open(void* file, const char *path, unsigned char forWrite)
{
    if (forWrite) {

        // Try open existing file
        if (file_fopen(file, &efs.myFs, (eint8*)path, 'a') != 0) {

            // Try create it
            if(file_fopen(file, &efs.myFs, (eint8*)path, 'w') != 0) {

                return -1;
            }
        }
        else {

            // Append will open file with position at the end, reset it!
            file_setpos(file, 0);
        }
    }
    else if (file_fopen(file, &efs.myFs, (eint8*)path, 'r') != 0) {

        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
/// Read data from an openned file with EFSL File System
/// \param file     Pointer to a file object instance.
/// \param buffer   Pointer to the buffer to hold read data.
/// \param size     The size of the data by byte.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int EFSL_read(void* file, void* buffer, int size)
{
    file_read(file, size, buffer);
    return 0;
}

//------------------------------------------------------------------------------
/// Write data to an openned file with EFSL File System
/// \param file     Pointer to a file object instance.
/// \param buffer   Pointer to the buffer to hold data for writing.
/// \param size     The size of the data by byte.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int EFSL_write(void* file, const void* buffer, int size)
{
    file_write(file, size, (euint8*)buffer);
    return 0;
}

//------------------------------------------------------------------------------
/// Close the file with EFSL File System
/// \param file     Pointer to a file object instance.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int EFSL_close(void* file)
{
    if (file_fclose(file) != 0) return -1;
    return 0;
}

//------------------------------------------------------------------------------
/// Mount FAT file system
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int FATFS_mount(void)
{
    FRESULT rc;
    memset(&fatFs, 0, sizeof(FATFS));
    rc = f_mount(0, &fatFs);
    if (rc != FR_OK) return -1;
    return 0;
}

//------------------------------------------------------------------------------
/// Format disk with FAT FS.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int FATFS_format(void)
{
    FRESULT rc;
    rc = f_mkfs(0,    // Drv
                0,    // FDISK partition
                512); // AllocSize
    if (rc != FR_OK) return -1;
    return 0;
}

//------------------------------------------------------------------------------
/// Open a file with FAT File System
/// \param file     Pointer to a file object instance.
/// \param path     Pointer to the string that describes the path of file
/// \param forWrite 1 to create file for writing, 0 to open file for reading
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int FATFS_open(void* file, const char *path, unsigned char forWrite)
{
    FRESULT rc;
    rc = f_open(file, path, forWrite ? (FA_CREATE_ALWAYS|FA_WRITE) : FA_READ);
    if (rc != FR_OK) return -1;
    return 0;
}

//------------------------------------------------------------------------------
/// Read data from an openned file with FAT File System
/// \param file     Pointer to a file object instance.
/// \param buffer   Pointer to the buffer to hold read data.
/// \param size     The size of the data by byte.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int FATFS_read(void* file, void* buffer, int size)
{
    FRESULT rc;
    UINT br;
    rc = f_read(file, buffer, size, &br);
    return (rc == FR_OK) ? 0 : -1;
}

//------------------------------------------------------------------------------
/// Write data to an openned file with EFSL File System
/// \param file     Pointer to a file object instance.
/// \param buffer   Pointer to the buffer to hold data for writing.
/// \param size     The size of the data by byte.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int FATFS_write(void* file, const void* buffer, int size)
{
    FRESULT rc;
    UINT bw;
    rc = f_write(file, buffer, size, &bw);
    return (rc == FR_OK) ? 0 : -1;
}

//------------------------------------------------------------------------------
/// Close the file with FAT File System
/// \param file     Pointer to a file object instance.
/// \return 0 for success, -1 with errors.
//------------------------------------------------------------------------------
static int FATFS_close(void* file)
{
    if (FR_OK != f_close(file)) {

        return -1;
    }
    return 0;
}

//------------------------------------------------------------------------------
/// Select used File System for disk accessing
/// \param pFF      Pointer to the collection of FS functions used.
/// \param fsType   The File System type ID (EFSL: 0, FAT FS: 1).
/// \return 1 if the File System is valid.
//------------------------------------------------------------------------------
static unsigned char FileSystemSelect(FSFunctions *pFF, unsigned char fsType)
{
    if (fsType == 0xFF) {

        fsType = (pFF->fs_init == EFSL_mount) ? 1 : 0;
    }
    if (fsType == 0) {

        printf("*** Using EFSL ***\n\r");
        pFF->fs_init    = EFSL_mount;
        pFF->fs_format  = EFSL_format;
        pFF->fs_open    = EFSL_open;
        pFF->fs_close   = EFSL_close;
        pFF->fs_read    = EFSL_read;
        pFF->fs_write   = EFSL_write;
    }
    else {

        printf("*** Using FAT FS ***\n\r");
        pFF->fs_init    = FATFS_mount;
        pFF->fs_format  = FATFS_format;
        pFF->fs_open    = FATFS_open;
        pFF->fs_close   = FATFS_close;
        pFF->fs_read    = FATFS_read;
        pFF->fs_write   = FATFS_write;
    }

    if (pFF->fs_init() == -1) {

        return 0;
    }

    return 1;
}

//------------------------------------------------------------------------------
/// Format disk
//------------------------------------------------------------------------------
static void FormatDisk(FSFunctions* pFF)
{
    printf("\n\r--- Format Disk (%s) ---\n\r",
        (pFF->fs_init == EFSL_mount) ? "EFSL" : "FATFS");
    if (pFF->fs_format) {
        pFF->fs_format();
    }
    printf(" - Done\n\r");
}

//------------------------------------------------------------------------------
/// Execute to test the following on assigned File System:
/// -# File System Mount.
/// -# File creation and writing speed.
/// -# File copying speed.
/// -# File content verifying speed.
/// -# File raw reading speed.
/// \param pFF  Pointer to the collection of FS functions used.
//------------------------------------------------------------------------------
static void FileSystemTest(FSFunctions* pFF)
{
    int i, j, block;
    unsigned int startTick, endTick;
    FileObj fileRead, fileWrite;
    unsigned char error = 0;
    unsigned char sec[BLOCK_SIZE];

    printf("\n\r--- File System Test (%s) ---\n\r",
        (pFF->fs_init == EFSL_mount) ? "EFSL" : "FATFS");

    printf("1. FS Mount:");
    if (-1 == pFF->fs_init()) {

        printf("FAIL\n\r");
        return;
    }
    else {

        printf("PASS\n\r");
    }

    printf("2. Creat file %s:", FILE_1);
    if (-1 == pFF->fs_open(&fileWrite, FILE_1, 1)) {

        printf("fopen FAIL\n\r");
        return;
    }
    else {

        printf("OK\n\r");
    }

    for (i = 0; i < BLOCK_SIZE; i ++) sec[i] = (unsigned char)i;
    printf("3. Writ %d bytes:", TESTFILE_SIZE);
    startTick = timeTick;
    for (i = 0; i < TESTFILE_SIZE; ) {

        block = (TESTFILE_SIZE-i > BLOCK_SIZE) ? BLOCK_SIZE : (TESTFILE_SIZE-i);
        if (-1 == pFF->fs_write(&fileWrite, sec, block)) {
            printf("FAIL\n\r");
            return;
        }
        i += block;
    }
    pFF->fs_close(&fileWrite);
    endTick = timeTick;
    printf("Done, Speed %d KB/s\n\r", TESTFILE_SIZE/(endTick - startTick));

    printf("4. Copy file %s to %s:", FILE_1, FILE_2);
    startTick = timeTick;
    if (-1 == pFF->fs_open(&fileRead, FILE_1, 0)) {

        printf("open source file FAIL\n\r");
        return;
    }
    if (-1 == pFF->fs_open(&fileWrite, FILE_2, 1)) {

        printf("open destination file FAIL\n\r");
        pFF->fs_close(&fileRead);
        return;
    }
    for(i = 0; i < TESTFILE_SIZE; ) {

        error = 0;
        block = (TESTFILE_SIZE-i > BLOCK_SIZE) ? BLOCK_SIZE : (TESTFILE_SIZE-i);
        if (-1 == pFF->fs_read(&fileRead, sec, block)) {

            printf("read FAIL\n\r");
            error = 1;
        }
        else if (-1 == pFF->fs_write(&fileWrite, sec, block)) {

            printf("write FAIL\n\r");
            error = 1;
        }
        if (error) {
            pFF->fs_close(&fileRead);
            pFF->fs_close(&fileWrite);
            return;
        }
        i += block;
    }
    pFF->fs_close(&fileRead);
    pFF->fs_close(&fileWrite);
    endTick = timeTick;
    printf("Done, Speed %d KB/s\n\r", TESTFILE_SIZE/(endTick - startTick));

    printf("5. Verify file %s:", FILE_2);
    startTick = timeTick;
    if (-1 == pFF->fs_open(&fileRead, FILE_2, 0)) {

        printf("fopen FAIL\n\r");
        return;
    }
    for(i = 0;i < TESTFILE_SIZE; ) {

        block = (TESTFILE_SIZE-i > BLOCK_SIZE) ? BLOCK_SIZE : (TESTFILE_SIZE-i);
        if (-1 == pFF->fs_read(&fileRead, sec, block)) {

            printf("read FAIL\n\r");
            pFF->fs_close(&fileRead);
            return;
        }
        i += block;

        for (j = 0; j < BLOCK_SIZE; j ++) {

            if ((unsigned char)j != sec[j]) {

                printf("data ERROR\n\r");
                pFF->fs_close(&fileRead);
                return;
            }
        }
    }
    pFF->fs_close(&fileRead);
    endTick = timeTick;
    printf("OK, Speed %d KB/s\n\r", TESTFILE_SIZE/(endTick - startTick));

    printf("6. Read file %s:", FILE_1);
    startTick = timeTick;
    if (-1 == pFF->fs_open(&fileRead, FILE_1, 0)) {

        printf("fopen FAIL\n\r");
        return;
    }
    for(i = 0;i < TESTFILE_SIZE; ) {

        block = (TESTFILE_SIZE-i > BLOCK_SIZE) ? BLOCK_SIZE : (TESTFILE_SIZE-i);
        if (-1 == pFF->fs_read(&fileRead, sec, block)) {

            printf("read FAIL\n\r");
            pFF->fs_close(&fileRead);
            return;
        }
        i += block;
    }
    pFF->fs_close(&fileRead);
    endTick = timeTick;
    printf("OK, Speed %d KB/s\n\r", TESTFILE_SIZE/(endTick - startTick));

    printf("-------------------------------\n\r");
    printf(" F to change File System Type\n\r");
    printf(" R to run the test again\n\r");
    printf("-------------------------------\n\r");
}

//------------------------------------------------------------------------------
//         Callbacks re-implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Internal functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Interrupt handler for all media types.
//------------------------------------------------------------------------------
void ISR_Media(void)
{
    MED_HandleAll(medias, numMedias);
}

//------------------------------------------------------------------------------
/// Initialize memory for RAM Disk, SD ...
//------------------------------------------------------------------------------
static void MemoryInitialization(void)
{
    // Memory initialization
#if defined(AT91C_BASE_DDR2C)
    TRACE_DEBUG("LUN DDR2\n\r");
    BOARD_ConfigureDdram(0, BOARD_DDRAM_BUSWIDTH); // Micron, 16 bit data bus

    MEDDdram_Initialize(&(medias[RAMDISK_I]),
                        BLOCK_SIZE,
                        (unsigned int)(AT91C_DDR2 + CODE_SIZE) / BLOCK_SIZE,
                        RAMDISK_SIZE / BLOCK_SIZE);
    LUN_Init(&(luns[RAMDISK_I]),
             &(medias[RAMDISK_I]),
             msdBuffer, MSD_BUFFER_SIZE,
             0, RAMDISK_SIZE / BLOCK_SIZE, 1,
             0, 0);

#elif defined(AT91C_EBI_SDRAM)

    TRACE_DEBUG("LUN SDRAM\n\r");
#if !defined(sdram)
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);
#endif
    
    MEDSdram_Initialize(&(medias[RAMDISK_I]),
                        BLOCK_SIZE,
                        (unsigned int)(AT91C_EBI_SDRAM + CODE_SIZE)/BLOCK_SIZE,
                        RAMDISK_SIZE/BLOCK_SIZE);
    LUN_Init(&(luns[RAMDISK_I]),
             &(medias[RAMDISK_I]),
             msdBuffer, MSD_BUFFER_SIZE,
             0, RAMDISK_SIZE/BLOCK_SIZE, 1,
             0, 0);

#endif // AT91C_EBI_SDRAM

    numMedias = 1;

}

//------------------------------------------------------------------------------
/// Start/Stop the USB Mass Storage
/// \param start 1 to start, 0 to stop
//------------------------------------------------------------------------------
static void MSDStart(unsigned char start)
{
    if (start) {

        // BOT driver initialization
        MSDDriver_Initialize(luns, numMedias);

        // connect if needed
        VBUS_CONFIGURE();
    }
    else {

        USBD_Disconnect();
    }
}


//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Initializes the Mass Storage driver and runs it.
//------------------------------------------------------------------------------
int main(void)
{
    FSFunctions ff;
    unsigned char isFsValid = 0;
    unsigned char isMsdConnected = 0;
    unsigned char doFsTest = 0, delayUsbInit = 0;

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic File System Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    // If they are present, configure Vbus pins
    PIO_InitializeInterrupts(0);

    // Configure PIO for time ticks
    ConfigurePit();

    // If there is on board power, switch it off
  #ifdef PIN_USB_POWER_ENB
  { const Pin pinUsbPwr = PIN_USB_POWER_ENB;
    PIO_Configure(&pinUsbPwr, 1);
  }
  #endif

    // Confirm that the USB disconnected
    MSDStart(0);

    // Initialize memories (disks)
    MemoryInitialization();

    ASSERT(numMedias > 0, "Error: No media defined.\n\r");
    TRACE_DEBUG("%u medias defined\n\r", numMedias);

    // We use EFSL in intialization
    isFsValid = FileSystemSelect(&ff, 0);

    if (!isFsValid) {

        printf("No Valid Disk!\n\r");
        printf("Please format the disk with PC through MSD!\n\r");
        printf("Waiting connection from PC ...\n\r");

        MSDStart(1);
    }
    else {

        doFsTest = 1;
        delayUsbInit = 1;
    }

    // Infinite loop
    while (1) {

        if (USBD_GetState() >= USBD_STATE_CONFIGURED) {

            if (isMsdConnected == 0) {

                isMsdConnected = 1;
            }

            // Mass storage state machine
            MSDDriver_StateMachine();
        }
        else {

            if (isMsdConnected) {

                isMsdConnected = 0;
                doFsTest = 1;
            }

        }

        if (DBGU_IsRxReady()) {

            switch(DBGU_GetChar()) {

            case 'R': case 'r':
                doFsTest = 1;
                break;

            case 'F': case 'f':
                FileSystemSelect(&ff, 0xFF);
                break;
            
            case 'M': case 'm':
                FormatDisk(&ff);
                break;
            }
        }

        if (doFsTest) {

            doFsTest = 0;

            // Do File System Test
            FileSystemTest(&ff);

            if (delayUsbInit) {

                delayUsbInit = 0;

                // Start Mass Storage so that we can connect the disk
                MSDStart(1);
            }
        }
    }
}

