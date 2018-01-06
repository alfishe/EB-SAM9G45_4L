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
#include <board_memories.h>
#include <pio/pio.h>
#include <lcd/lcd.h>
#include <rtt/rtt.h>
#include <irq/irq.h>
#include <utility/trace.h>
#include <lcd/color.h>

#include <string.h>
#include <stdio.h>


extern unsigned char Bg_16bpp_b_fh_fv[];
//------------------------------------------------------------------------------
//         Internal constants
//------------------------------------------------------------------------------

/// Addresses of the two image buffers
#if defined(PINS_DDRAM)
unsigned char *images[2] = {
    (unsigned char *) (AT91C_DDR2 + 0x00300000),
    (unsigned char *) (AT91C_DDR2 + 0x00700000)
};
#elif defined(PINS_SDRAM)
unsigned char *images[2] = {
    (unsigned char *) (AT91C_EBI_SDRAM + 0x00100000),
    (unsigned char *) (AT91C_EBI_SDRAM + 0x00200000)
};
#else
    #error Not define PINS_SDRAM or PINS_DDRAM
#endif

#define DISPLAY_ADDRESS           (AT91C_DDR2 + 0x0300000)
//------------------------------------------------------------------------------
//         Internal functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Configures the PIO needed by the application.
//------------------------------------------------------------------------------
static void ConfigurePins()
{
    static const Pin pins[] = {PINS_LCD};
    PIO_Configure(pins, PIO_LISTSIZE(pins));
}

//------------------------------------------------------------------------------
/// Initializes the LCD controller with the board parameters.
//------------------------------------------------------------------------------
void InitializeLcd()
{
    // Enable peripheral clock
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_LCDC;

#if defined(at91sam9g10)||defined(at91sam9261)
    AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_HCK1;
#endif

    // Disable the LCD and the DMA
    LCD_DisableDma();
    LCD_Disable(0);

    // Configure the LCD controller
    //LCD_SetPixelClock(BOARD_MCK, BOARD_LCD_PIXELCLOCK);
		AT91C_BASE_LCDC->LCDC_LCDCON1 = 10 << 12;

    LCD_SetDisplayType(BOARD_LCD_DISPLAYTYPE);
    LCD_SetScanMode(AT91C_LCDC_SCANMOD_SINGLESCAN);
    LCD_SetBitsPerPixel(BOARD_LCD_BPP);
    LCD_SetPolarities(BOARD_LCD_POLARITY_INVVD,
                      BOARD_LCD_POLARITY_INVFRAME,
                      BOARD_LCD_POLARITY_INVLINE,
                      BOARD_LCD_POLARITY_INVCLK,
                      BOARD_LCD_POLARITY_INVDVAL);
    LCD_SetClockMode(BOARD_LCD_CLOCKMODE);
    LCD_SetMemoryFormat((unsigned int) AT91C_LCDC_MEMOR_LITTLEIND);
    LCD_SetSize(BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);

    // Configure timings
    LCD_SetVerticalTimings(BOARD_LCD_TIMING_VFP,
                           BOARD_LCD_TIMING_VBP,
                           BOARD_LCD_TIMING_VPW,
                           BOARD_LCD_TIMING_VHDLY);
    LCD_SetHorizontalTimings(BOARD_LCD_TIMING_HBP,
                             BOARD_LCD_TIMING_HPW,
                             BOARD_LCD_TIMING_HFP);

    // Configure contrast (TODO functions)
    LCD_SetContrastPrescaler(AT91C_LCDC_PS_NOTDIVIDED);
    LCD_SetContrastPolarity(AT91C_LCDC_POL_POSITIVEPULSE);
    //LCD_SetContrastValue(0xE8);
	LCD_SetContrastValue(0xE0);
    LCD_EnableContrast();

    // Configure DMA
    LCD_SetFrameSize(BOARD_LCD_FRAMESIZE);
    LCD_SetBurstLength(4);

    // Set frame buffer
    LCD_SetFrameBufferAddress(images[0]);

    // Enable DMA and LCD
    LCD_EnableDma();
    LCD_Enable(0x0C);
}

//------------------------------------------------------------------------------
/// Main function
//------------------------------------------------------------------------------
int main()
{   
    unsigned int time;
    unsigned int nextImage = 0;

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic LCD Project %s --\n\r", SOFTPACK_VERSION);
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
    
//	LCD_DecodeRGB((unsigned char*)Bg_16bpp_b_fh_fv,images[0],400,200,8);
#define LCD_Width 800
#define LCD_High 480

#if 0
#if defined(BOARD_LCD_RGB565)
    unsigned int i, image;
    unsigned char r, g, b;

    // RGB 565 fix
//    for (image = 0; image < 2; image++) {
    
        for (i = 0; i < (LCD_Width * LCD_High * 3); i += 3) {
    
            r = Bg_16bpp_b_fh_fv[i];
            g = Bg_16bpp_b_fh_fv[i+1];
            b = Bg_16bpp_b_fh_fv[i+2];
    
            // Interlacing
            r = ((r << 1) & 0xF0) | ((g & 0x80) >> 4) | ((r & 0x80) >> 5);
            g = (g << 1) & 0xF8;
            b = b & 0xF8;
    
            // R & B inversion
            Bg_16bpp_b_fh_fv[i+2] = r;
            Bg_16bpp_b_fh_fv[i+1] = g;
            Bg_16bpp_b_fh_fv[i] = b;
        }
//    }
#elif defined (BOARD_LCD_BGR565)
    unsigned int i, image;
    unsigned char r, g, b;

    // RGB 565 fix
    for (image = 0; image < 2; image++) {
    
        for (i = 0; i < (LCD_Width * LCD_High * 3); i += 3) {
    
            r = images[image][i];
            g = images[image][i+1];
            b = images[image][i+2];
    
            // R & B inversion
            images[image][i+2] = r;
            images[image][i+1] = g;
            images[image][i] = b;
        }
    }
#endif //#if defined(BOARD_LCD_RGB565)
#endif

    // Configure pins
    ConfigurePins();

    // Initialize LCD
    InitializeLcd();

    // Configure RTT for a 2s tick
    RTT_SetPrescaler(AT91C_BASE_RTTC, 0);

    // Infinite loop
    time = RTT_GetTime(AT91C_BASE_RTTC); 


    while (1) {

        // Wait next second
        while (time == RTT_GetTime(AT91C_BASE_RTTC));
        time = RTT_GetTime(AT91C_BASE_RTTC);

        // Display image
        LCD_SetFrameBufferAddress((unsigned char *)images[nextImage]);
        nextImage = (nextImage + 1) % 2;
    }

    return 0;
}

