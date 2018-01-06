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
This project demonstrates the TWI peripheral in slave mode. It mimics the
behavior of a serial memory, enabling the TWI master to read and write
data in its internal SRAM.

Requirements
===========================
In addition, another device will be needed to act as the TWI master. The
basic-twi-eeprom-project can be used for that, in which case a second kit
supported by that project is needed.
Note: For AT91SAM9261-EK, the TWI slave is available on AT91SAM9261S chip.

Description
===========================
After launching the program, the device will act as a simple TWI-enabled
serial memory containing 512 bytes. This enables this project to be used
with the basic-twi-eeprom-project as the master.

To write in the memory, the TWI master must address the device first, then
send two bytes containing the memory address to access. Additional bytes are
treated as the data to write.

Reading is done in the same fashion, except that after receiving the memory
address, the device will start outputting data until a STOP condition is
sent by the master.

Usage
===========================
-# The default address for the TWI slave is fixed to Ox50.
   If the board has a TWI component with this adress, you can change
   the define AT24C_ADDRESS in main.c of twi-eeprom project, and the define 
   SLAVE_ADDRESS in main.c of twi-slave project.

-# For AT91SAM9G45-EK board:
-# Connect TWD (SDA) for the 2 boards: pin 8 of connector J9
-# Connect TWCK(SCL) for the 2 boards: pin 7 of connector J9
-# Connect GND for the 2 boards: pin 30 of connector J9
-# Add a pull up of 2,2KOhms on TWD and TWCK (pin 1 of J17 is 3,3V)

-# For the TWI Slave board:
-# Compile the application. 
-# Connect the DBGU port of the evaluation board to the computer and open 
   it in a terminal.
   - Settings: 115200 bauds, 8 bits, 1 stop bit, no parity, no flow control. 
-# Download the program inside the evaluation board and run it.
-# Upon startup, the application will output the following line on the DBGU: 
   \code
    -- Basic TWI Slave Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    -I- Configuring the TWI in slave mode
   \endcode
-# For the TWI Master board, see the description inside his project
-# and the "Master" board will output:
   \code
    -- Basic TWI EEPROM Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    -I- Filling page #0 with zeroes ...
    -I- Filling page #1 with zeroes ...
    -I- Read/write on page #0 (polling mode)
    -I- 0 comparison error(s) found
    -I- Read/write on page #1 (IRQ mode)
    -I- Callback fired !
    -I- Callback fired !
    -I- 0 comparison error(s) found
   \endcode
//-------------------------------------------------------------------------------------

