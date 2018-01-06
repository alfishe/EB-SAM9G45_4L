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
The USB CDC Serial Project will help you to get familiar with the
USB Device Port(UDP) and USART interface on AT91SAM microcontrollers. Also
it can help you to be familiar with the USB Framework that is used for
rapid development of USB-compliant class drivers such as USB Communication
Device class (CDC).

You can find following information depends on your needs:
- Sample usage of USB CDC driver and USART driver.
- USB CDC driver development based on the AT91 USB Framework.
- USB enumerate sequence, the standard and class-specific descriptors and
  requests handling.
- The initialize sequence and usage of UDP interface.
- The initialize sequence and usage of USART interface with PDC.

Requirements
===========================
This package can be used with all Atmel evaluation kits that have both
UDP and USART interface.

Description
===========================
When an EK running this program connected to a host (PC for example), with
USB cable, the EK appears as a Seriao COM port for the host, after driver
installation with the offered 6119.inf. Then the host can send or receive
data through the port with host software. The data stream from the host is
then sent to the EK, and forward to USART port of AT91SAM chips. The USART
port of the EK is monitored by the timer and the incoming data will be sent
to the host.

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
    -- USB Device CDC Serial Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    \endcode
-# When connecting USB cable to windows, the LED blinks, and the host
   reports a new USB %device attachment (if it's the first time you connect
   an %audio speaker demo board to your host). You can use the inf file
   at91lib\\usb\\device\\cdc-serial\\drv\\6119.inf to install the serial
   port. Then new "AT91 USB to Serial Converter (COMx)" appears in the
   hardware %device list.
-# You can run hyperterminal to send data to the port. And it can be seen
   at the other hyperterminal connected to the USART port of the EK.
//-------------------------------------------------------------------------------------



