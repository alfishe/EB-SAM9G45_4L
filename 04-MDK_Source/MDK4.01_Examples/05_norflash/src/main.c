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
#include <utility/trace.h>
#include <utility/math.h>
#include <string.h>
#include "nor_flash.h"

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
#define BUFFER_SIZE        0x400
#define WRITE_READ_ADDR    0x8000
unsigned short TxBuffer[BUFFER_SIZE];
unsigned short RxBuffer[BUFFER_SIZE];
/// Pins to configure for the application
#ifdef PINS_NORFLASH
static const Pin pPins[] = {
   PINS_NORFLASH
};
#endif

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Tests the norflash connected to the board by performing several command
/// on each of its pages.
//------------------------------------------------------------------------------
int main(void)
{
	unsigned short manID, devID;
	int i, status;
		
    // Configure pins and DBGU
#ifdef PINS_NORFLASH
    PIO_Configure(pPins, PIO_LISTSIZE(pPins));		  
#endif
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic NorFlash Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

	BOARD_ConfigureNorFlash(BOARD_NORFLASH_DFT_BUS_SIZE);

	manID = Nor_ReadManuID();
	devID = Nor_ReadDevID();
	printf("NorFlash Manu ID = 0x%x, Device ID = 0x%x\r\n", manID, devID);

	printf("Nor Flash is erasing...\r\n");
	/* Erase the NOR memory Sector to write on */
  	Nor_EraseSector(WRITE_READ_ADDR);

	/* Fill the buffer to send */
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		TxBuffer[i] = 0x1234 + i;
	}

	printf("Nor Flash is writing...\r\n");
	Nor_WriteBuffer(TxBuffer, WRITE_READ_ADDR, BUFFER_SIZE);

 	printf("Nor Flash is reading...\r\n");
	Nor_ReadBuffer(RxBuffer, WRITE_READ_ADDR, BUFFER_SIZE);

	status = 0;
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		if (RxBuffer[i] != TxBuffer[i])
		{
			status = 1;
			break;
		}
	}

	if (status == 0)
	{
		printf("Nor Flash operation success!\r\n");
	}
	else
	{
		printf("Nor Flash operation error!\r\n");
	}
	
	while (1);
}

