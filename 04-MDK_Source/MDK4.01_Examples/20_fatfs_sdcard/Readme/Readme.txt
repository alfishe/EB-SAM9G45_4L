/**************************************************************************************
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
The Basic-fatfs-sdcard-project is the concatenation of 2 basic projects, 
basic-fatfs-project and basic-sdcard-project.
It will help you to get familiar with filesystem and sdmmc_mci interface on
AT91 microcontrollers. 
It supplies sample code of common operations of a filesystem through a SD
Card based filesystem.
It can also help you to configure the filesystem according to your own
needs, which can be used for fast implementation of your own filesystem and
other applications related.
FatFs is a generic file system module to implement the FAT file system to
small embedded systems.

You can find following information depends on your needs:
- Supply a set of file system interface for user to do file related work,
  e.g. mount a disk, initialize a disk, create/open a file, write/read a
  file e.t.c.
- Supply a set of disk level interface for user's reference, which is called
  by filesystem level
- Teaches user how to use these interfaces through sample code
- Configuration of a filesystem, e.g. user can build up a tiny filesystem
  which consume low memory, or a filesystem with read-only functions.
  Configuration is set in the fatfs_config.h file.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have MCI
interface, the package runs at SRAM or external RAM, so EBI interface and
external RAM device is needed if you want to run this package in external
RAM

Description
===========================
When launched, this program asks user to plug a SD card in the MCI
connector.
Once the SD card is plugged, there are 2 cases. 
- The SD card is not yet formated, the program formats it with FAT file
  system. 
- The SD card is already formated. All the files contained on the SD Card
  are displayed. Then, depending on the user answer, the SD card is erased
  and re-formated or nothing is done.

Finally a Basic.bin file is created and filled with a special pattern. Then
the file is closed, reopened and its data are verified.

For the FATFS TINY project version, copy previously the Basic.bin file on
the SD Card in the root directory.
This file can be found in the project.
The FAT system is compiled with only the reading functions (see _FS_READONLY
option in fatfs_config.h). 
This option can be modified.

!!!Note-----------
The project basic-sdcard-project can be used to "unformat" the SD card. By
writing dummy values in the first blocks, the FAT file system data are
erased.

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
    -- Basic FatFS Full Version with SDCard Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    -I- Init media Sdcard
    -I- MEDSdcard init
    -I- Please connect a SD card ...
    \endcode
//-------------------------------------------------------------------------------------



