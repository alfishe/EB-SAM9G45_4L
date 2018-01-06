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
The USB HID Transfer Project will help you to get familiar with the
USB Device Port(UDP) and PIO interface on AT91SAM microcontrollers. Also
it can help you to be familiar with the USB Framework that is used for
rapid development of USB-compliant class drivers such as USB Humen
Interface Device class (HID).

You can find following information depends on your needs:
- Sample usage of USB HID driver and PIO driver.
- USB HID driver development based on the AT91 USB Framework.
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
USB cable, the EK appears as a "USB Human Interface Device" for the host.
Then you can use the client application to read/write on it.

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
    -- USB Device HID Transfer Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    \endcode
-# When connecting USB cable to windows, the LED blinks. 
   Then new "HID Transfer Device" appears in the
   hardware %device list.
-# Then you can use the PC program !hidTest.exe to check the !device
   information and run tests.
-# Find the HID Device whose VID is 03EB and PID is 6201, select item type
   and item to see its attributes.
-# Type what you want to send in output edit box, use buttons on the right
   side to send. You can see data information in debug terminal.
-# You can use the buttons above the input edit box to read data from
   !device of monitor the data, then the data and the status of the buttons
   on the board is read and the gray buttons is up or down based on the
   buttons status on the board.
//-------------------------------------------------------------------------------------

