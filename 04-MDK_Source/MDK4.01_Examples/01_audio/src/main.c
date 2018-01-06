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
#include <irq/irq.h>
#include <dbgu/dbgu.h>
#include <utility/wav.h>
#include <utility/assert.h>
#include <utility/trace.h>
#include <utility/math.h>
#include <memories/MEDSdcard.h>
#include <twi/twi.h>
#include <twi/twid.h>
#include <stdio.h>
#include <string.h>
#include "wm8731.h"
#include "ssc.h"

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

// TWI clock
#define 	TWI_CLOCK                 100000
#define     SLOT_BY_FRAME             (2)
#define     BITS_BY_SLOT              (16)
#define 	SAMPLE_RATE               (48000)
/// Address at which the WAV file is located
#define 	WAV_FILE_ADDRESS          (0x70000000 + 0x100000 + 0x100)

#define AT91C_I2S_MASTER_TX_SETTING(nb_bit_by_slot, nb_slot_by_frame)( +\
                       AT91C_SSC_CKS_DIV   +\
                       AT91C_SSC_CKO_CONTINOUS      +\
                        AT91C_SSC_START_FALL_RF +\
                       ((1<<16) & AT91C_SSC_STTDLY) +\
                       ((((nb_bit_by_slot*nb_slot_by_frame)/2)-1) <<24))

#define AT91C_I2S_TX_FRAME_SETTING(nb_bit_by_slot, nb_slot_by_frame)( +\
                        (nb_bit_by_slot-1)  +\
                        AT91C_SSC_MSBF   +\
                        (((nb_slot_by_frame-1)<<8) & AT91C_SSC_DATNB)  +\
                        (((nb_bit_by_slot-1)<<16) & AT91C_SSC_FSLEN) +\
                        AT91C_SSC_FSOS_NEGATIVE)

/// Maximum size in bytes of the WAV file.
#define MAX_WAV_SIZE            0x2000000

#define PINS_SSC_CODEC {(1 << 10) | (1 << 12) | (1 << 14), \
                      AT91C_BASE_PIOD, AT91C_ID_PIOD_E, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PCK0      {(1 << 27), \
                      AT91C_BASE_PIOD, AT91C_ID_PIOD_E, PIO_PERIPH_A, PIO_DEFAULT}
//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
/// Available medias.
Media medias[MAX_LUNS];
/// List of pins to configure.
static const Pin pinsAudio[] = {PINS_TWI0, PINS_SSC_CODEC, PIN_PCK0};
static Twid twid;
volatile unsigned int SSCBitRate;

/// Pointer to the playback WAV file header.
static const WavHeader *userWav = (WavHeader *)(0x70000000 + 0x100000);
const char* FileName = STR_ROOT_DIRECTORY "sample.wav";

/// Indicates if the user-provided WAV file is valid.
static unsigned char isWavValid;

/// Indicates if the WAV file is currently being played.
static unsigned char isWavPlaying;

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Display the information of the WAV file (sample rate, stereo/mono and frame
/// size) on the DBGU.
//------------------------------------------------------------------------------
static void DisplayWavInfo(void)
{   
    printf( "\n\r  Wave file header information\n\r");
    printf( "--------------------------------\n\r");
    printf( "  - Chunk ID        = 0x%08X\n\r", userWav->chunkID);
    printf( "  - Chunk Size      = %u\n\r", userWav->chunkSize);
    printf( "  - Format          = 0x%08X\n\r", userWav->format);
    printf( "  - SubChunk ID     = 0x%08X\n\r", userWav->subchunk1ID);
    printf( "  - Subchunk1 Size  = %u\n\r", userWav->subchunk1Size);
    printf( "  - Audio Format    = 0x%04X\n\r", userWav->audioFormat);
    printf( "  - Num. Channels   = %d\n\r", userWav->numChannels);
    printf( "  - Sample Rate     = %u\n\r", userWav->sampleRate);
    printf( "  - Byte Rate       = %u\n\r", userWav->byteRate);
    printf( "  - Block Align     = %d\n\r", userWav->blockAlign);
    printf( "  - Bits Per Sample = %d\n\r", userWav->bitsPerSample);
    printf( "  - Subchunk2 ID    = 0x%08X\n\r", userWav->subchunk2ID);
    printf( "  - Subchunk2 Size  = %u\n\r", userWav->subchunk2Size);
    printf("Press a key to return to the menu ...\n\r");
    DBGU_GetChar();
}

//------------------------------------------------------------------------------
/// Displays the user menu on the DBGU.
//------------------------------------------------------------------------------
static void DisplayMenu(void)
{
    printf("Menu :\n\r");
    printf("------\n\r");

    // Play a WAV file pre-loaded in SDRAM using SAM-BA
    if (isWavValid && !isWavPlaying) {
        printf("  P: Play the WAV file\n\r");
    }
    else if (!isWavValid) {
        printf("  P: Not available, invalid WAV file provided.\n\r");
    }

    // Display the information of the WAV file (sample rate, stereo/mono and frame size)
    if (isWavValid) {
        printf("  D: Display the information of the WAV file\n\r");
    }

	// stop the current playback
	if (isWavPlaying) {
		printf("  S: Stop playback\n");
	}
}

//------------------------------------------------------------------------------
/// Play a WAV file pre-loaded in SDCARD.
//------------------------------------------------------------------------------
void PlayLoop(unsigned short *pExtMem, unsigned int numSamples)
{
    unsigned int i;

    for (i = 0; i < numSamples; i++) {

        SSC_Write(AT91C_BASE_SSC1, pExtMem[i]);
    }
}

//------------------------------------------------------------------------------
/// Play a WAV file
//------------------------------------------------------------------------------
static void PlayWavFile(void)
{
#if !defined(AUDIO_USING_DMA)
    unsigned int size;

    size = userWav->subchunk2Size > MAX_WAV_SIZE ? MAX_WAV_SIZE : userWav->subchunk2Size;
    SSCBitRate = userWav->sampleRate;
	AT91C_BASE_SSC1->SSC_CMR = (BOARD_MCK) / (2 * (((unsigned int)(BITS_BY_SLOT * 2)) * SSCBitRate));
	SSC_EnableTransmitter(AT91C_BASE_SSC1);
    PlayLoop((unsigned short *)WAV_FILE_ADDRESS, size >> 1);
#else
    unsigned int size;
    unsigned int intFlag = 0;

    size = userWav->subchunk2Size > MAX_WAV_SIZE ? MAX_WAV_SIZE : userWav->subchunk2Size;
    SSC_EnableTransmitter(AT91C_BASE_SSC1);

    // Start transmitting WAV file to SSC
    remainingSamples = userWav->subchunk2Size;
    transmittedSamples = 0;
    
    intFlag = 1 << (BOARD_SSC_DMA_CHANNEL + 8) ;
    DMA_DisableIt(intFlag);
    DMA_DisableChannel(BOARD_SSC_DMA_CHANNEL);
    
    // Fill DMA buffer
    size = min(remainingSamples / (userWav->bitsPerSample / 8), BOARD_SSC_DMA_FIFO_SIZE * MAX_SSC_LLI_SIZE/2);
	SSC_WriteBuffer(AT91C_BASE_SSC1, (void *) (WAV_FILE_ADDRESS + transmittedSamples), size);
    remainingSamples -= size * (userWav->bitsPerSample / 8);
    transmittedSamples += size * (userWav->bitsPerSample / 8);

    intFlag = 1 << (BOARD_SSC_DMA_CHANNEL + 8) ;
    DMA_EnableIt(intFlag);
    DMA_EnableChannel(BOARD_SSC_DMA_CHANNEL);

#endif
}   

//------------------------------------------------------------------------------
/// Stop the current playback (if any).
//------------------------------------------------------------------------------
static void StopPlayback(void)
{
    SSC_DisableTransmitter(AT91C_BASE_SSC1);
}
 
//------------------------------------------------------------------------------
/// Check wav file from sdcard
//------------------------------------------------------------------------------
unsigned char CheckWavFile()
{
    FRESULT res;
    FATFS fs;             // File system object
    FIL FileObject;

    unsigned int numRead, pcmSize;

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
    memset(&fs, 0, sizeof(FATFS));		// Clear file system object
    res = f_mount(ID_DRV, &fs);
    if( res != FR_OK ) {
        printf("-E- f_mount pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
    }

	// open sample.wav file
    res = f_open(&FileObject, FileName, FA_OPEN_EXISTING|FA_READ);
    if (res == FR_OK) {
        printf("-I- File Found!\n\r");
        //f_close(&FileObject);
        //    FilePlay();
    }
    else {
        printf("-E- File Not Found!\n\r");
        return 0;
    }

    // Read header
    res = f_read(&FileObject, (void*)userWav, sizeof(WavHeader), &numRead);
    if (res != FR_OK) {
	    printf("-E- f_read pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
	}
	else {
		DisplayWavInfo();
	}

    // Load PCM
    pcmSize = userWav->subchunk2Size;
    if (pcmSize > MAX_WAV_SIZE) {
        pcmSize = MAX_WAV_SIZE;
    } 

    res = f_read(&FileObject, (void*)WAV_FILE_ADDRESS, pcmSize, &numRead);
    if (res != FR_OK) {
	    printf("-E- f_read pb: 0x%X (%s)\n\r", res, FF_GetStrResult(res));
        return 0;
	}
	else
	{
    	printf("-I- PCM Load to %x, size %d\n\r", WAV_FILE_ADDRESS, numRead);
    	f_close(&FileObject);  
	}

    return 1;
}
     
//------------------------------------------------------------------------------
/// Main function
//------------------------------------------------------------------------------
int main(void)
{
    unsigned char key;

    // Initialize the DBGU
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic Audio Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    
    // Initialize the Audio controller
    PIO_Configure(pinsAudio, PIO_LISTSIZE(pinsAudio));

	// Configure and enable the TWI (required for accessing the DAC)
	*AT91C_PMC_PCER = (1<< AT91C_ID_TWI0);
	TWI_ConfigureMaster(AT91C_BASE_TWI0, TWI_CLOCK, BOARD_MCK);
	TWID_Initialize(&twid, AT91C_BASE_TWI0);

	// Enable the DAC master clock, Use PCK1
    AT91C_BASE_PMC->PMC_PCKR[1] = AT91C_PMC_CSS_PLLA_CLK | AT91C_PMC_PRES_CLK_16;
    AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_PCK1;
    while ((AT91C_BASE_PMC->PMC_SR & AT91C_PMC_PCK1RDY) == 0);

	// Initialize the audio DAC
	WM8731_DAC_Init(&twid, WM8731_SLAVE_ADDRESS);

	// Configure SSC
	SSCBitRate = (unsigned int)(SAMPLE_RATE);
	SSC_Configure(AT91C_BASE_SSC1,
				  AT91C_ID_SSC1,
				  SAMPLE_RATE * BITS_BY_SLOT * 2,
				  BOARD_MCK);
	SSC_ConfigureReceiver(AT91C_BASE_SSC1, 0, 0);
	SSC_ConfigureTransmitter(AT91C_BASE_SSC1,
		AT91C_I2S_MASTER_TX_SETTING(BITS_BY_SLOT, SLOT_BY_FRAME),
		AT91C_I2S_TX_FRAME_SETTING(BITS_BY_SLOT, SLOT_BY_FRAME));
	SSC_DisableTransmitter(AT91C_BASE_SSC1);

    // Check and load wav file from sdcard
    isWavValid = CheckWavFile();
    if(!isWavValid) {
        printf("-E- Open wav file fail!\r\n");
        return 1;
    }

    isWavPlaying = 0;
    
    // Enter menu loop
    while (1) {

        // Display menu
        DisplayMenu();

        // Process user input
        key = DBGU_GetChar();

        // Play WAV file
        if ((key == 'P') || (key == 'p') && isWavValid && !isWavPlaying) {
            PlayWavFile();
            isWavPlaying = 1;
        }
        // Display WAV information
        else if ((key == 'D') || (key == 'd') && isWavValid) {
              DisplayWavInfo();
        }
		// Stop playback
		else if ((key == 'S') || (key == 's') && isWavValid && isWavPlaying) {
            StopPlayback();
            isWavPlaying = 0;
		}
    }

	return 0;
}

