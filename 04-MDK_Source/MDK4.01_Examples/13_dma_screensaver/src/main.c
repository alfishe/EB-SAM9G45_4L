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
#include <dma/dma.h>
#include <drivers/dmad/dmad.h>
#include <drivers/lcd/lcdd.h>
#include <drivers/lcd/draw.h>
#include <drivers/lcd/color.h>
#include <rtt/rtt.h>
#include <utility/trace.h>
#include <utility/bmp.h>
#include <utility/rand.h>

#include <string.h>
#include <stdio.h>

//------------------------------------------------------------------------------
//         Internal constants
//------------------------------------------------------------------------------

#if defined(PINS_DDRAM)
/// Address at which the image file is located.
#define IMAGE_LOAD_ADDRESS        (AT91C_DDR2 + 0x0100000)
///  Base address which to store the decoded image
#define IMAGE_BASE_ADDRESS        (AT91C_DDR2 + 0x0200000)
/// Address at which to display screen saver.
#define DISPLAY_ADDRESS           (AT91C_DDR2 + 0x0300000)
#elif defined(PINS_SDRAM)
/// Address at which the image file is located.
#define IMAGE_LOAD_ADDRESS        (AT91C_EBI_SDRAM + 0x0100000)
///  Base address which to store the decoded image
#define IMAGE_BASE_ADDRESS        (AT91C_EBI_SDRAM + 0x0200000)
/// Address at which to display screen saver.
#define DISPLAY_ADDRESS           (AT91C_EBI_SDRAM + 0x0300000)
#else
    #error Not define PINS_SDRAM or PINS_DDRAM
#endif

/// DMA transfer WORD width.
#define DMA_TRANSFER_WIDTH      2

/// Numbers of color channel of image (RGB).
#define PICTURE_COLOR_CHANNEL   3

/// Width of LCD screen.
#define IMAGE_WIDTH             (BOARD_LCD_WIDTH * PICTURE_COLOR_CHANNEL)
/// Height of LCD screen.
#define IMAGE_HEIGHT            BOARD_LCD_HEIGHT

/// Width of picture ATMEL logo in pixel.
#define PICTURE_WIDTH           (120 * PICTURE_COLOR_CHANNEL)

/// Height of picture ATMEL logo in pixel.
#define PICTURE_HEIGHT          56

/// Row offset of atmel picture.
#define PICTURE_ROW             (IMAGE_HEIGHT - PICTURE_HEIGHT)/2

/// Atmel logo picture address offset (centre of the image).
#define ATMEL_OFFSET_IN_IMAGE    ((IMAGE_WIDTH -  PICTURE_WIDTH) /2  +  PICTURE_ROW * IMAGE_WIDTH)

/// Black logo picture address offset (centre of the image).
#define BLACK__OFFSET_IN_IMAGE  0

/// Calculate the number of width transfers.
#define DMA_TRANSFER_SIZE(size)  ((size) >> DMA_TRANSFER_WIDTH)

/// Sceeensaver delay time in second.
#define DELAY                   3

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

/// Linked lists for multi transfer buffer chaining structure instance.
static DmaLinkList dmaLinkList[PICTURE_HEIGHT];
/// Picture-In-Picture structure instance.
static PictureInPicture pip;

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Dummy callback, to test transfer modes.
//------------------------------------------------------------------------------
void TestCallback()
{
    printf("-I- Callback fired !\n\r");
}

//------------------------------------------------------------------------------
/// Returns coordinate of point along the X-axis and Y-axis of the screen.
/// \param x X-axis position.
/// \param y Y-axis position.
//------------------------------------------------------------------------------
static void generateImagePosition(unsigned int *x, unsigned int *y)
{
    unsigned int number;
    unsigned int row, col;
    
    number = rand();    
    row = (number % BOARD_LCD_WIDTH) /8 * 8;
    row = (row > PICTURE_WIDTH)? PICTURE_WIDTH: row;
    col = (number & 0xFF);
    col = (col > (IMAGE_HEIGHT - PICTURE_HEIGHT))? IMAGE_HEIGHT - PICTURE_HEIGHT: col;
    *x = row;
    *y = col;
}    

//------------------------------------------------------------------------------
/// Decode bmp file.
//------------------------------------------------------------------------------
static void DecodeImage(void) 
{
    // Decode new image
    BMP_Decode((void *) IMAGE_LOAD_ADDRESS,
               (unsigned char *)IMAGE_BASE_ADDRESS,
               BOARD_LCD_WIDTH,
               BOARD_LCD_HEIGHT,
               24);
}               

//------------------------------------------------------------------------------
/// Configure the DMA source and destination with Linker List mode.
/// \param sourceAddress  Start source address.
/// \param destAddress  Start destination address.
//------------------------------------------------------------------------------
static void configureLinkList(unsigned int sourceAddress, unsigned int destAddress)
{
    unsigned int i;
    for(i = 0; i < PICTURE_HEIGHT; i++){
        dmaLinkList[i].sourceAddress = sourceAddress + IMAGE_WIDTH * i;
        dmaLinkList[i].destAddress = destAddress  + IMAGE_WIDTH * i;
        dmaLinkList[i].controlA = DMA_TRANSFER_SIZE(PICTURE_WIDTH ) | DMA_TRANSFER_WIDTH << 24 | DMA_TRANSFER_WIDTH << 28;
        if (i == PICTURE_HEIGHT - 1) {
            dmaLinkList[i].controlB = AT91C_SRC_DSCR | AT91C_DST_DSCR;
            dmaLinkList[i].descriptor = 0;
        }
        else {
            dmaLinkList[i].controlB = 0x0;
            dmaLinkList[i].descriptor = (unsigned int)(&dmaLinkList[i+1]);
        }
    }    
}

//------------------------------------------------------------------------------
/// Transfer part of image(atmel logo) using multi-buffer transfer with Linked 
/// List for both source and destination.
/// \param x X-axis offset for destination
/// \param y Y-axis offset for destination.
//------------------------------------------------------------------------------
static void transferImageWithMultiBufferLli(unsigned int x, unsigned int y)
{
    unsigned int bufferSize;
    unsigned int startSourceAddr;
    unsigned int startDestAddr;
    // Initialize DMA controller using channel 1.
    DMAD_Initialize(DMA_CHANNEL_1, DMAD_USE_DEFAULT_IT);
    startSourceAddr = IMAGE_BASE_ADDRESS + ATMEL_OFFSET_IN_IMAGE;
    startDestAddr = DISPLAY_ADDRESS + x * PICTURE_COLOR_CHANNEL + y * IMAGE_WIDTH;
    bufferSize = DMA_TRANSFER_SIZE(PICTURE_WIDTH);
    // Configure linker list item.
    configureLinkList(startSourceAddr, startDestAddr);
    /// Configure transfer size and width per transfer.
    DMAD_Configure_TransferController(DMA_CHANNEL_1, bufferSize, DMA_TRANSFER_WIDTH, DMA_TRANSFER_WIDTH, 0, 0);
    // Configure multi-buffer transfer with Linked List for both source and destination.
    DMAD_Configure_Buffer(DMA_CHANNEL_1, DMA_TRANSFER_LLI, DMA_TRANSFER_LLI, &dmaLinkList[0], 0);
    // Start channel 1 transfer.
    DMAD_BufferTransfer(DMA_CHANNEL_1, DMA_TRANSFER_SIZE(PICTURE_WIDTH), TestCallback, 1);
}

//------------------------------------------------------------------------------
/// Transfer part of image(black picture) using single buffer transfer with 
/// Picture-In-Picture mode enable.
/// \param x X-axis offset for destination.
/// \param y Y-axis offset for destination..
//------------------------------------------------------------------------------
static void transferImageWithSingleBufferPip(unsigned int x, unsigned int y)
{
    unsigned int bufferSize;
    unsigned int startSourceAddr;
    unsigned int startDestAddr;
    // Initialize DMA controller using channel 0.
    DMAD_Initialize(DMA_CHANNEL_0, DMAD_USE_DEFAULT_IT);
    bufferSize = DMA_TRANSFER_SIZE(PICTURE_WIDTH* PICTURE_HEIGHT);
    startSourceAddr = IMAGE_BASE_ADDRESS + BLACK__OFFSET_IN_IMAGE;
    startDestAddr = DISPLAY_ADDRESS + x * PICTURE_COLOR_CHANNEL + y * IMAGE_WIDTH;
    /// Configure transfer size and width per transfer.
    DMAD_Configure_TransferController(DMA_CHANNEL_0, bufferSize, DMA_TRANSFER_WIDTH, DMA_TRANSFER_WIDTH, startSourceAddr, startDestAddr);
    /// Configure hole and boundary size in Picture-in-Picture mode in both source and destination.
    pip.pipSourceHoleSize = DMA_TRANSFER_SIZE(IMAGE_WIDTH - PICTURE_WIDTH);
    pip.pipSourceBoundarySize = DMA_TRANSFER_SIZE(PICTURE_WIDTH);
    pip.pipDestHoleSize = pip.pipSourceHoleSize;
    pip.pipDestBoundarySize = pip.pipSourceBoundarySize;
    // Configure single buffer transfer with Picture-In-Picture mode enable.
    DMAD_Configure_Buffer(DMA_CHANNEL_0, DMA_TRANSFER_SINGLE, DMA_TRANSFER_SINGLE, 0, &pip);
    // Start channel 0 transfer.
    DMAD_BufferTransfer(DMA_CHANNEL_0, bufferSize, TestCallback , 0);
    while (!DMAD_IsFinished(DMA_CHANNEL_0));
}    

//------------------------------------------------------------------------------
/// Transfer part of image using Multi-buffer transfer with source address 
/// auto-reloaded and contiguous destination address.
//------------------------------------------------------------------------------
static void transferImageWithMultiBufferReload(void)
{
    unsigned int bufferSize;
    unsigned int startSourceAddr;
    unsigned int startDestAddr;
    // Initialize DMA controller using channel 1.
    DMAD_Initialize(DMA_CHANNEL_1, DMAD_USE_DEFAULT_IT);
    startSourceAddr = IMAGE_BASE_ADDRESS + PICTURE_ROW * IMAGE_WIDTH;
    startDestAddr = DISPLAY_ADDRESS + 20 *  IMAGE_WIDTH;;
    bufferSize = DMA_TRANSFER_SIZE(PICTURE_HEIGHT * IMAGE_WIDTH);
    /// Configure transfer size and width per transfer.
    DMAD_Configure_TransferController(DMA_CHANNEL_1, bufferSize, DMA_TRANSFER_WIDTH, DMA_TRANSFER_WIDTH, startSourceAddr, startDestAddr);
    // Configure multi-buffer transfer with source address auto-reloaded and contiguous destination address.
    DMAD_Configure_Buffer(DMA_CHANNEL_1, DMA_TRANSFER_RELOAD, DMA_TRANSFER_CONTIGUOUS, 0 , 0);
    // Start channel 1 transfer. Source image auto-reload 5 times and transfer to destination continguous.
    DMAD_BufferTransfer(DMA_CHANNEL_1, bufferSize * 5, TestCallback, 0);
    while (!DMAD_IsFinished(DMA_CHANNEL_1));
}

//------------------------------------------------------------------------------
/// Main function
//------------------------------------------------------------------------------
int main()
{   
    unsigned int time;
    unsigned char delay;
    /// Coordinate of point along the X-axis of the screen.
    unsigned int x = 0;
    /// Coordinate of point along the Y-axis of the screen.
    unsigned int y = 0;
        
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic DMA Screensaver Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

#if !defined(sdram) && defined(PINS_SDRAM)
    //configure SDRAM for use
    BOARD_ConfigureSdram(BOARD_SDRAM_BUSWIDTH);
#endif

#if !defined(ddram) && defined(PINS_DDRAM)
    //configure DDRAM for use
    BOARD_ConfigureDdram(0, BOARD_DDRAM_BUSWIDTH);
#endif

    // Decode bmp file.
    DecodeImage();
    
    // Initialize LCD
	LCDD_Initialize();
    LCDD_Fill((unsigned char*)DISPLAY_ADDRESS, COLOR_BLACK);
    LCDD_DisplayBuffer((unsigned char*)DISPLAY_ADDRESS);
    
    // Configure RTT for a 3 second tick.
    RTT_SetPrescaler(AT91C_BASE_RTTC, 32768);
     
    // Initialize seed of random generator.
    srand(100);
    
    while(1)
    {        
        // Transfer part of image (desktop image) using Multi-buffer transfer with source address auto-reloaded and contiguous destination address.
        transferImageWithMultiBufferReload();
        delay = 0;
        while(delay != DELAY){
            // Wait next second
            while (time == RTT_GetTime(AT91C_BASE_RTTC));
            time = RTT_GetTime(AT91C_BASE_RTTC);
            delay++;
        }            
        // Clear screen.
        LCDD_Fill((unsigned char*)DISPLAY_ADDRESS, COLOR_BLACK);
        // Start screensaver until key pressed.        
        while(1) {
            // Transfer part of image(black picture) using single buffer transfer with Picture-In-Picture mode enable.
            transferImageWithSingleBufferPip(x, y);
            generateImagePosition(&x, &y);
            // Transfer part of image(atmel logo) using multi-buffer transfer with Linked List for both source and destination.
            transferImageWithMultiBufferLli(x, y);
            // Back to desktop if user key detected.
            if(DBGU_IsRxReady()){
                 DBGU_GetChar();
                 break;
            }
            // Wait for next second.
            while (time == RTT_GetTime(AT91C_BASE_RTTC));
            time = RTT_GetTime(AT91C_BASE_RTTC);
        }
    }

    return 0;
}

