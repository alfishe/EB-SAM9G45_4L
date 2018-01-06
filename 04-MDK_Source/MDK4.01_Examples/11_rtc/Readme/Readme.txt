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
This basic example shows how to use the Real-Time Clock (RTC) peripheral 
available on the newest Atmel AT91 microcontrollers. The RTC enables easy 
time and date management and allows the user to monitor events like a 
configurable alarm, second change, calendar change, and so on.

Description
===========================
Upon startup, the program displays the currently set time and date 
(00:00:00, 01/01/2007 at reset) and a menu to perform the following: 
    Menu:
       t - Set time
       d - Set date
       i - Set time alarm
       m - Set date alarm
       c - Clear the alarm notification (only if it has been triggered)

Setting the time, date and time alarm is done by using Menu option "t", "d",
the display is updated accordingly. 

The time alarm is triggered only when the second, minute and hour match the preset
values; the date alarm is triggered only when the month and date match the preset
values. If both time alarm and date alarm are set, only when the second, minute,
hour, month and date match the preset values, the alarm will be triggered.

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
    -- Basic RTC Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --

    Menu:
    t - Set time
    d - Set date
    i - Set time alarm
    m - Set date alarm
    q - Quit
   \endcode
-# Press one of the keys listed in the menu to perform the corresponding action.
//-------------------------------------------------------------------------------------


