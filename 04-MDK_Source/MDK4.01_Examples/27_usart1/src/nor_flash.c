
#include <board.h>
#include <stdio.h>
#include "nor_flash.h"

#define M16(adr) (*((volatile unsigned short *)(adr)))

union fsreg {                  // Flash Status Register
  struct b  {
    unsigned int q0:1;
    unsigned int q1:1;
    unsigned int q2:1;
    unsigned int q3:1;
    unsigned int q4:1;
    unsigned int q5:1;
    unsigned int q6:1;
    unsigned int q7:1;
  } b;
  unsigned int v;
} fsr;

/*
 * Check if Program/Erase completed
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int Polling (unsigned long adr) {
  unsigned int q6;

  fsr.v = M16(adr);
  q6 = fsr.b.q6;
  do {
    fsr.v = M16(adr);
    if (fsr.b.q6 == q6) return (0);  // Done
    q6 = fsr.b.q6;
  } while (fsr.b.q5 == 0);           // Check for Timeout
  fsr.v = M16(adr);
  q6 = fsr.b.q6;
  fsr.v = M16(adr);
  if (fsr.b.q6 == q6) return (0);    // Done
  M16(adr) = 0xF0;                   // Reset Device
  return (1);                        // Failed
}

/******************************************************************************
* Function Name  : Nor_ReadManuID
* Description    : Reads NOR memory's Manufacturer ID.
* Input          : None.
* Output         : None
* Return         : Manufacturer ID
*******************************************************************************/
unsigned short Nor_ReadManuID(void)
{
	unsigned short ManuID;

    /* Enter Software Product Identification Mode  */
    M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
    M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
    M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = ID_IN_WORD_COM;

	ManuID = M16(BOARD_NORFLASH_ADDR + 0) & 0x00FF;
	return ManuID;
}

/******************************************************************************
* Function Name  : Nor_ReadDevID
* Description    : Reads NOR memory's Device ID.
* Input          : None.
* Output         : None
* Return         : Device ID
*******************************************************************************/
unsigned short Nor_ReadDevID(void)
{
	unsigned short DeviceID;

    /* Enter Software Product Identification Mode  */
    M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
    M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
    M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = ID_IN_WORD_COM;

	DeviceID = M16(BOARD_NORFLASH_ADDR + 2) & 0xFFFF;
	return DeviceID;
}

/*******************************************************************************
* Function Name  : Nor_EraseSector
* Description    : Erases the specified Nor memory Sector.
* Input          : - SectorAddr: address of the Sector to erase.
* Output         : None
* Return         : NOR_Status:The returned value can be: TRUE, FALSE
*******************************************************************************/
int Nor_EraseSector(unsigned int SectorAddr)
{
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = ERASE_SECTOR_CODE1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
	M16(BOARD_NORFLASH_ADDR + SectorAddr) = ERASE_SECTOR_CODE2;

  	return (Polling(BOARD_NORFLASH_ADDR + SectorAddr));       // Wait until Erase completed
}

/*******************************************************************************
* Function Name  : Nor_EraseChip
* Description    : Erases the entire chip.
* Input          : None                      
* Output         : None
* Return         : NOR_Status:The returned value can be: TRUE, FALSE
*******************************************************************************/
int Nor_EraseChip(void)
{
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = ERASE_SECTOR_CODE1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = CHIP_SECTOR_CODE1;

	return (Polling(BOARD_NORFLASH_ADDR));  // Wait until Erase completed
}

/******************************************************************************
* Function Name  : Nor_WriteWord
* Description    : Writes a half-word to the NOR memory. 
* Input          : - WriteAddr : NOR memory internal address to write to.
*                  - Data : Data to write. 
* Output         : None
* Return         : None
*******************************************************************************/
void Nor_WriteWord(unsigned int WriteAddr, unsigned short Data)
{
	int i;

	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = FLASH_WORD_COM_1;
	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_2) = FLASH_WORD_COM_2;
 	M16(BOARD_NORFLASH_ADDR + FLASH_SEQ_ADD_1) = WRITE_WORD_COM;
	M16(BOARD_NORFLASH_ADDR + WriteAddr) = Data;

	Polling(BOARD_NORFLASH_ADDR + WriteAddr);
}

/*******************************************************************************
* Function Name  : Nor_WriteBuffer
* Description    : Writes a half-word buffer to the NOR memory. 
* Input          : - pBuffer : pointer to buffer. 
*                  - WriteAddr : NOR memory internal address from which the data 
*                    will be written.
*                  - WriteNum : number of Half words to write. 
* Output         : None
* Return         : None
*******************************************************************************/
void Nor_WriteBuffer(unsigned short *pBuffer, unsigned int WriteAddr, unsigned int WriteNum)
{
	int i;
	
	for (i = 0; i < WriteNum; i++)
	{
		Nor_WriteWord(WriteAddr, *pBuffer++);
		WriteAddr = WriteAddr + 2;
	}
}

/******************************************************************************
* Function Name  : Nor_ReadWord
* Description    : Reads a half-word from the NOR memory. 
* Input          : - ReadAddr : NOR memory internal address to read from.
* Output         : None
* Return         : Half-word read from the NOR memory
*******************************************************************************/
unsigned short Nor_ReadWord(unsigned int ReadAddr)
{
	return M16(BOARD_NORFLASH_ADDR + ReadAddr); 
}

/*******************************************************************************
* Function Name  : Nor_ReadBuffer
* Description    : Reads a block of data from the NOR memory.
* Input          : - pBuffer : pointer to the buffer that receives the data read 
*                    from the NOR memory.
*                  - ReadAddr : NOR memory internal address to read from.
*                  - NumHalfwordToRead : number of Half word to read.
* Output         : None
* Return         : None
*******************************************************************************/
void Nor_ReadBuffer(unsigned short *pBuffer, unsigned int ReadAddr, unsigned int ReadNum)
{
	int i;

	for (i = 0; i < ReadNum; i++)
	{
		*pBuffer++ = M16(BOARD_NORFLASH_ADDR+ReadAddr);
		ReadAddr = ReadAddr + 2;
	}
}

/******************************************************************************
* Function Name  : Nor_Reset
* Description    : Returns the NOR memory to Read mode and resets the errors in
*                  the NOR memory Status Register.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Nor_Reset(void)
{
	M16(BOARD_NORFLASH_ADDR) = ID_OUT_WORD_COM;
}
