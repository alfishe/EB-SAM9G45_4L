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
The USB HID Keyboard Project will help you to get familiar with the
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
This package can be used with all Atmel evaluation kits that has UDP
interface and have push buttons on it.

Description
===========================
When an EK running this program connected to a host (PC for example), with
USB cable, the EK appears as a HID Keyboard for the host. Then you can use
the push buttons on the EK to input letter to the host. E.g, to open a
editor and input a letter 'a' or '9'.

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
    -- USB Device HID Keyboard Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    \endcode
-# When connecting USB cable to windows, the LED blinks. 
   Then new "HID Keyboard Device" appears in the
   hardware %device list.
-# Once the device is connected and configured, pressing any of the board buttons 
   should send characters to the host PC. Pressing num. lock should also make the third 
   LED toggle its state (on/off).
//-------------------------------------------------------------------------------------

