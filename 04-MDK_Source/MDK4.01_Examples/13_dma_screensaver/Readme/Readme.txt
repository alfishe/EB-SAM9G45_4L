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
This example shows how to use the DMA controller of AT91 microcontrollers
to transfer picture from source memory to dispaly memory.

Description
===========================
This example project should enable the user to perform the
following tasks: 
- Display desktop picture on LCD
   - Display 5 of Embest logo at LCD vertically as a desktop 
     picture using DMA Multi-buffer with source address 
     auto-reloaded and contiguous destination address.
- Screensaver
  - Display Embest logo at random location on LCD using 
    DMA Multi-buffer transfer with Linked List for both 
    source and destination.
  - Display black picture at previous location using DMA 
    single buffer transfer with Picture-In-Picture mode enable.

The example project transfer picture from an image which preload 
in memory using DMAC. The image(in BMP format) must be loaded in 
SDRAM or DDRAM before the program execution, at address offset 
0x0100000. 
Two already processed image are provided in the project folder.
Bitmap Image320x240.bmp use for LCD 320X240, and image480x272.bmp 
use for LCD 480x272.

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
   -- Basic DMA Screensaver Project xxx --
   -- AT91xxxxxx-xx
   -- Compiled: xxx xx xxxx xx:xx:xx --
   \endcode
When launched, this program displays desktop picture on LCD and start to 
play screensaver after 3 seconds, User can press any key to stop screensaver
and back to desktop.
//-------------------------------------------------------------------------------------

