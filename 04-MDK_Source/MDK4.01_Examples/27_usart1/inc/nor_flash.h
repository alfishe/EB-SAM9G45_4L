/*********************************************************************************************
* File£º	Flash.H
* Author:	embest	
* Desc£º	AM29LV800B header file
* History:	
*********************************************************************************************/

#ifndef __FLASH_H__
#define __FLASH_H__

/* Define the specific for the flash memory system */
// AM29LV800 Manufacture ID and Device ID
#define MANU_ID				0x01
#define FLASH_ID			0x225b

/* command word */
#define FLASH_SEQ_ADD_1  	(0x5555 << 1)
#define FLASH_SEQ_ADD_2  	(0x2AAA << 1)
#define FLASH_WORD_COM_1 	((short)0xAAAA)
#define FLASH_WORD_COM_2 	((short)0x5555)
#define WRITE_WORD_COM    	((short)0xA0A0)
#define ID_IN_WORD_COM   	((short)0x9090)
#define ID_OUT_WORD_COM  	((short)0xF0F0)
#define ERASE_SECTOR_CODE1  ((short)0x8080)
#define CHIP_SECTOR_CODE1   ((short)0x1010)
#define ERASE_SECTOR_CODE2  ((short)0x3030)

/********************************/
/* DEFINE BANK 0 DATA BUS WIDTH */
/********************************/
#define B0SIZE_BYTE			0x1
#define B0SIZE_SHORT		0x2
#define B0SIZE_WORD			0x3

#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif

/*****************************************/
/* Functions for AMD AM29LV800B	     */
/*****************************************/
extern unsigned short Nor_ReadManuID(void);
extern unsigned short Nor_ReadDevID(void);
extern int Nor_EraseSector(unsigned int SectorAddr);
extern int Nor_EraseChip(void);
extern void Nor_WriteWord(unsigned int WriteAddr, unsigned short Data);
extern void Nor_WriteBuffer(unsigned short *pBuffer, unsigned int WriteAddr, unsigned int WriteNum);
extern unsigned short Nor_ReadWord(unsigned int ReadAddr);
extern void Nor_ReadBuffer(unsigned short *pBuffer, unsigned int ReadAddr, unsigned int ReadNum);
extern void Nor_Reset(void);
#endif /* __FLASH_H__ */
