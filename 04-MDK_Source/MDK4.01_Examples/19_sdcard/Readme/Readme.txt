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
The Basic-sdcard-project will help you to get familiar with sdmmc_mci interface on 
AT91SAM microcontrollers. It can also help you to get familiar with the SD operation flow
which can be used for fast implementation of your own sd drivers and other applications 
related.

You can find following information depends on your needs:
- Usage of auto detection of sdcard insert and sdcard write-protection detection
- MCI interface initialize sequence and interrupt installation
- Sdcard driver implementation based on MCI interface 
- Sdcard physical layer initialize sequence implementation
- Sample usage of sdcard write and read

Requirements
===========================
This package can be used with all Atmel evaluation kits that have MCI interface, the 
package runs at SRAM or SDRAM, so EBI interface and SDRAM device is needed if you
want to run this package in SDRAM

Description
===========================
Open HyperTerminal before running this program, use SAM-BA to download this program to 
SRAM or external RAM, make the program run, the HyperTerminal will give out the test results.

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
    -- Basic SD-Card MCI Mode Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    -I- Please connect a SD card ...
    -I- SD card connection detected
    -I- Cannot check if SD card is write-protected
    -I- SD/MMC card initialization successful
    -I- Card size: *** MB
    -I- Block size: *** Bytes
    -I- Testing block [  *** -   ***] ..."
    \endcode
//-------------------------------------------------------------------------------------

