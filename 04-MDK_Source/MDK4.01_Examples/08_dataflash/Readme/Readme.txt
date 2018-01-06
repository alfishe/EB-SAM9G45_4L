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
The Basic Dataflash project will help new users get familiar with SPI interface 
on Atmel's AT91 family of microcontrollers. This project gives you an AT45 
Dataflash programming code so that can help develop your own SPI devices
applications with maximum efficiency.

You can find following information depends on your needs:
- A Spi low level driver performs SPI device Initializes, data transfer and 
receive. It can be used by upper SPI driver such as AT45 %dataflash.
- A Dataflash driver is based on top of the corresponding Spi driver.
It allow user to do operations with %dataflash in a unified way.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have SPI
interface and on-board or external Serialflash connected. The package runs at 
SRAM or SDRAM, so SDRAM device is needed if you want to run this package in SDRAM.

Description
===========================
The demonstration program tests the dataflash present on the evaluation kit by 
erasing and writing each one of its pages.

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
   -- Basic Dataflash Project xxx --
   -- AT91xxxxxx-xx
   -- Compiled: xxx xx xxxx xx:xx:xx --
   -I- Initializing the SPI and AT45 drivers
   -I- At45 enabled
   -I- SPI interrupt enabled
   -I- Waiting for a dataflash to be connected ...
   \endcode
-# As soon as a dataflash is connected, the tests will start. Eventually, 
   the test result (pass or fail) will be output on the DBGU.
//-------------------------------------------------------------------------------------


