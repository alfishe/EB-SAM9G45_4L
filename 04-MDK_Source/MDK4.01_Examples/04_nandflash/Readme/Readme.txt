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
The Basic-nandflash-project will help you to get familiar with nandflash interface on AT91SAM 
microcontrollers. It can also help you to get familiar with the nandflash operation flow
which can be used for fast implementation of your own nandflash drivers and other 
applications related.

You can find following information depends on your needs:
  - Nandflash Initialize, this include a find model procedure, the program will try to find 
    the device model with the readout Nandflash id.
  - Basic operations of Nandflash, such as erase blocks, write blocks, read blocks.
  - Get Nandflash parameters after find the Nandflash model. User can refer to NandflashModel
    for those parameters.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have EBI interface, the 
package runs at SRAM or SDRAM, so EBI interface and SDRAM device is needed if you
want to run this package in SDRAM.

Description
===========================
Open HyperTerminal before running this program, use SAM-BA to download this program to 
SRAM or SDRAM, make the program run, the HyperTerminal will give out the test results.

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
    -- Basic NandFlash Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    Nandflash ID is 0x1580DA2C
    -I-     Nandflash driver initialized
    -I- Size of the whole device in bytes : 0x10000000
    -I- Size in bytes of one single block of a device : 0x20000
    -I- Number of blocks in the entire device : 0x800
    -I- Size of the data area of a page in bytes : 0x800
    -I- Number of pages in the entire device : 0x40
    -I- Bus width : 0x8
    -I- Test in progress on block:     22
    \endcode
//-------------------------------------------------------------------------------------






