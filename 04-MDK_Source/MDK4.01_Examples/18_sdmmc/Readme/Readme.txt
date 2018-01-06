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
The Basic-sdmmc-project will help you to get familiar with sdmmc_mci
interface on AT91SAM microcontrollers. It can also help you to get familiar
with the SD & MMC operation flow which can be used for fast implementation
of your own SD drivers and other applications related.

You can find following information depends on your needs:
- Usage of auto detection of sdcard insert and sdcard write-protection
- MCI interface initialize sequence and interrupt installation
- SD/MMC card driver implementation based on MCI interface 
- SD card physical layer initialize sequence implementation
- MMC card physical layer initialize sequence implementation
- Sample usage of SD/MMC card write and read

Requirements
===========================
This package can be used with all Atmel evaluation kits that have MCI/MCI2
interface, the package runs at SDRAM or DDRAM, so EBI interface and SDRAM
device is needed.

Description
===========================
Open HyperTerminal before running this program, use SAM-BA to download this
program to SDRAM or DDRAM, make the program run, the HyperTerminal will
give out the test hints, you can run different tests on a inserted card.

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
-# In HyperTerminal, it will show something like on start up
    \code
    -- Basic SD/MMC MCI Mode Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    -I- Cannot check if SD card is write-protected
    -I- DMAD_Initialize channel 0  
    TC Start ... OK

    ==========================================
    -I- Card Type 1, CSD_STRUCTURE 0
    -I- SD 4-BITS BUS
    -I- CMD6(1) arg 0x80FFFF01
    -I- SD HS Not Supported
    -I- SD/MMC TRANS SPEED 25000 KBit/s
    -I- SD/MMC card initialization successful
    -I- Card size: 483 MB, 990976 * 512B
    ...
    \endcode
-# Test function menu is like this
    \code
    # 0,1,2 : Block read test
    # w,W   : Write block test(With data or 0)
    # b,B   : eMMC boot mode or access boot partition change
    # i,I   : Re-initialize card
    # t     : Disk R/W/Verify test
    # T     : Disk performance test
    # p     : Change number of blocks in one access for test
    # s     : Change MCI Clock for general test
    \endcode
//-------------------------------------------------------------------------------------

