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
This example program demonstrates how to use the TWI peripheral of an AT91
microcontroller to access an external serial EEPROM chip. 

Requirements
===========================
An external serial EEPROM must be connected to the TWI bus of the 
microcontroller.

Pay particular attention to the fact that on some boards, such as the
AT91SAM7S-EK, there is no pull-up on the TWI bus: they must be 
added externally. 

Description
===========================
This software performs simple tests on the first and second page of the EEPROM: 
- Sets both pages to all zeroes 
- Writes pattern in page #0 (polling) 
- Reads back data in page #0 and compare with original pattern (polling) 
- Writes pattern in page #1 (interrupts) 
- Reads back data in page #1 and compare with original pattern (interrupts) 

Usage
===========================
-# Compile the application. 
-# Connect the DBGU port of the evaluation board to the computer and open 
it in a terminal.
   - Settings: 115200 bauds, 8 bits, 1 stop bit, no parity, no flow control. 
-# Download the program inside the evaluation board and run it. 
-# Upon startup, the application will output the following line on the DBGU: 
   \code
    -- Basic TWI EEPROM Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
   \endcode
-# The following traces detail operations on the EEPROM, displaying success 
   or error messages depending on the results of the commands. 
//-------------------------------------------------------------------------------------
