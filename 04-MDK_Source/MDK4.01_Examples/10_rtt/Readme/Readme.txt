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
This example demonstrates the Real-Time Timer (RTT) provided on 
AT91 microcontrollers. It enables the user to set an alarm and watch 
it being triggered when the timer reaches the corresponding value. 

You can configure the RTT by following steps
- SetPrescaler the RTT to 1s (32768)
- Initialize the ISR routine which refesh it when 1s is counted down
- Enable the RTT Interrtup of the vector 

Description
===========================
When launched, this program displays a timer count and a menu on the DBGU, 
enabling the user to choose between several options: 

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
    -- Basic RTT Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    Time: 0
    Menu:
    r - Reset timer
    s - Set alarm
    Choice?
   \endcode

The user can then choose any of the available options to perform the described action. 
//-------------------------------------------------------------------------------------








