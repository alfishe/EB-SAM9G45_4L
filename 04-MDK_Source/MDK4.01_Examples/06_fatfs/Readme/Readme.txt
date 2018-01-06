/***************************************************************************************
* File Name          : readme.txt
* Date First Issued  : 01/10/2010
* Description        : Read Me
***************************************************************************************
***************************************************************************************
* History:
* 01/10/2010:          V1.0
**************************************************************************************/

Purpose
===========================
The Basic-fatfs-project will help you to get familiar with filesystem on AT91SAM microcontrollers. 
It supplies sample code of common operations of a filesystem through a External RAM based filesystem.
It can also help you to configure the filesystem according to your own needs, which can be used
for fast implementation of your own filesystem and other applications related.
FatFs is a generic file system module to implement the FAT file system to small embedded systems.

You can find following information depends on your needs:
- External RAM initialize sample code
- Supply a set of file system interface for user to do file related work, e.g. mount a disk, initialize 
   a disk, create/open a file, write/read a file e.t.c, this can be used on different physical medias, 
   such as External RAM, SD card. 
- Supply a set of disk level interface for user's reference, which is called by filesystem level
- Teaches user how to use these interfaces through sample code
- Configuration of a filesystem, e.g. user can build up a tiny filesystem which consume low memory,
   or a filesystem with read-only functions

Requirements
===========================
This package can be used with all Atmel evaluation kits that have EBI interface and External RAM device

Description
===========================
Open HyperTerminal before running this program, use SAM-BA to download this program to 
SRAM or External RAM, make the program run, the HyperTerminal will give out the test results.

Usage
===========================
-# Build the program and download it inside the evaluation board. 
-# On the computer, open and configure a terminal application
   (e.g. HyperTerminal on Microsoft Windows) with these settings:
  - 115200 bauds
  - 8 bits of data
  - No parity
  - 1 stop bit
  - No flow control
-# Start the application
-# In HyperTerminal, it will show something like
   \code
   -- Basic FatFS Full Version with SDRAM Project xxx --
   -- AT91SAMxxx
   -- Compiled: xxx --
   -I- xxx init (xxx is based on physical medias)
   -I- Mount disk 0
   -I- Format disk 0
   -I- Please wait a moment during formating...
   -I- Format disk finished !
   -I- Create a file : "0:Basic.bin"
   -I- Write file
   -I- ByteWritten=512
   -I- f_write ok: ByteWritten=512
   -I- Close file
   -I- Open file : 0:Basic.bin
   -I- Read file
   -I- Close file
   -I- File data Ok !
   -I- Test passed !
   \endcode
//-------------------------------------------------------------------------------------




