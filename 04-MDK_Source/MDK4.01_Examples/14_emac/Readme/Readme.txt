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
This project uses the Ethernet MAC (EMAC) and the on-board Ethernet transceiver available
on Atmel evaluation kits. It enables the device to respond to a ping command sent by a host computer. 

Hardware requirements
===========================
The system reset signal NRST is connected to both the Ethernet PHY and the JTAG/ICE interface. 
This prevents the project from working properly when a JTAG probe is connected. 
To disconnect NRST from the ICE interface, the following modifications must be performed: 
 - AT91SAM7X/XC-EK: cut strap S2. 
 - AT91SAM9XE-EK: remove resistor R14 
 - AT91SAM9260-EK: remove resistor R14 
 - AT91SAM9263-EK: remove resistor R18 

Description
===========================
Upon startup, the program will configure the EMAC with a default IP and MAC addresses 
and then ask the transceiver to auto-negotiate the best mode of operation. Once this is done, 
it will start monitoring incoming packets and processing them whenever appropriate. 
The basic will only answer to two kinds of packets: 

- It will reply to ARP requests with its MAC address, 
- and to ICMP ECHO request so the device can be PING'ed. 

To test that the board responds correctly to ping requests, type the following 
command-line on a computer connected to the same network as the board: 
\code 
  ping 192.168.2.19
\endcode

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
-# Connect an Ethernet cable between the evaluation board and the network. 
    The board may be connected directly to a computer; in this case, 
    make sure to use a cross/twisted wired cable such as the one provided with the evaluation kit. 
-# Start the application. It will display the following message on the DBGU: 
   \code   
    -- Basic EMAC Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    MAC 00:45:56:78:9a:ac
    IP 192.168.2.19
   \endcode
-# The program will then auto-negotiate the mode of operation and start receiving packets, 
     displaying feedback on the DBGU. To display additional information, press any key in the terminal application. 
-# To test that the board responds to ICMP ECHO requests, type the following command line in a shell: 
    \code 
     ping 192.168.2.19
    \endcode
//-------------------------------------------------------------------------------------

