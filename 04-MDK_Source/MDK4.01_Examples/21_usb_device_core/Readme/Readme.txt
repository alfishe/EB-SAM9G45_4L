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
The USB Enumeration Project will help you to get familiar with the
USB Device Port(UDP)interface on AT91SAM microcontrollers. Also
it can help you to be familiar with the USB Framework that is used for
rapid development of USB-compliant class drivers such as USB Communication
Device class (CDC).

You can find following information depends on your needs:
- Sample usage of USB Device Framework.
- USB enumerate sequence, the standard and class-specific descriptors and
  requests handling.
- The initialize sequence and usage of UDP interface.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have UDP
interface.

Description
===========================
When an EK running this program connected to a host (PC for example), with
USB cable, host will notice the attachment of a USB %device. No %device
driver offered for the %device now.

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
    -- USB Device Core Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    \endcode
-# When connecting USB cable to windows, the LED blinks, and the host
   reports a new USB %device attachment.
//-------------------------------------------------------------------------------------



