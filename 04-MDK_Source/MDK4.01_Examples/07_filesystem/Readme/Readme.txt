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
The basic-fs-project will help you to get familiar with filesystem
on AT91SAM microcontrollers.

This demo is based on Open Source FAT filesystem and EFSL(Embedded
Filesystem Library). It supplies sample code of common operations of a
filesystem through a RAM Disk based filesystem. Also the accessing speed
is calculated for your comparation and reference.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have EBI 
interface, SDRAM device and USB interface.

Description
===========================
Open HyperTerminal before running this program, use SAM-BA to download this
program to SRAM or SDRAM, make the program run, the HyperTerminal will give
out the test results and operation hints.

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
-# Start the application.
-# In the terminal window, the following text should appear:
    \code
    -- Basic File System Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    *** Using EFSL ***
    
    --- File System Test (EFSL) ---
    1. FS Mount:PASS
    2. Creat file test.bin:OK
    3. Writ xxxxxxx bytes:Done, Speed xxxxx KB/s
    4. Copy file test.bin to copy.bin:Done, Speed xxxx KB/s
    5. Verify file copy.bin:OK, Speed xxxx KB/s
    6. Read file test.bin:OK, Speed xxxx KB/s
    -------------------------------
     F to change File System Type
     R to run the test again
    -------------------------------
    \endcode
-# The following input is used for the test
  - 'F' to toggle the File System between EFSL and FAT FS
  - 'R' to run/re-start the File System test sequence
//-------------------------------------------------------------------------------------


