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
The Basic Norflash project will show you how to program NorFlash. It will read the 
NorFlash' ID, and read or write data to NorFlash.

You can find following information depends on your needs:
- Configures the EBI for NorFlash access.
- Low-level driver implement procedures to program basic operations
  described in the datasheets for norflash devices.
- Sample code for accessing norflash device.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have the 
External Bus Interfac (EBI) and an external NorFlash chip connecteda.
the package runs at SRAM or SDRAM, so SDRAM device is needed if you want 
to run this package in SDRAM.

Description
===========================
At startup, the program configures the SMC to access the NorFlash and tries 
to identify it. If it succeed, Each block is first erased, then all its pages 
are written and verified. 

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
-# Upon startup, the application will output the following lines on the DBGU.
   \code
    -- Basic NorFlash Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
	-- Basic NorFlash Project 1.7 --
	-- AT91SAM9G45-EK
	-- Compiled: Jan 28 2010 08:46:14 --
	NorFlash Manu ID = 0x1, Device ID = 0x225b
	Nor Flash is erasing...
	Nor Flash is writing...
	Nor Flash is reading...
	Nor Flash operation success!
   \endcode
-# Eventually, the test result (pass or fail) will be output on the DBGU.
//-------------------------------------------------------------------------------------


