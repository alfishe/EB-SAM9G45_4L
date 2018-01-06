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
This example shows how to use the WM8731 controller to play sound.

Description
===========================
When launched, this program displays a menu on the DBGU,
enabling the user to choose between several options
- Play a loaded WAV file in Micro SD Card 
- Display the WAV file information (sample rate, etc.) 

To be able to play a WAV file, it must first be loaded into the root directory of a 
Micro SD Card, then insert this SD Card into the board.  

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
    -- Basic AC97 Project xxx --
    -- AT91xxxxxx-xx
    -- Compiled: xxx xx xxxx xx:xx:xx --
    -I- Codec probed correctly
	-I- Please connect a SD card ...
	-I- SD card connection detected
	-I- Init media Sdcard
	-I- MEDSdcard init
	-I- DMAD_Initialize channel 0  
	-I- Card Type 1, CSD_STRUCTURE 0
	-I- SD/MMC TRANS SPEED 25000 KBit/s
	-I- SD 4-BITS BUS
	-I- SD/MMC TRANS SPEED 25000 KBit/s
	-I- SD/MMC card initialization successful
	-I- Card size: 121 MB
	-I- Mount disk 0
	-I- File Found!
	
	  Wave file header information
	--------------------------------
	  - Chunk ID        = 0x46464952
	  - Chunk Size      = 6801444
	  - Format          = 0x45564157
	  - SubChunk ID     = 0x20746D66
	  - Subchunk1 Size  = 16
	  - Audio Format    = 0x0001
	  - Num. Channels   = 2
	  - Sample Rate     = 24000
	  - Byte Rate       = 96000
	  - Block Align     = 4
	  - Bits Per Sample = 16
	  - Subchunk2 ID    = 0x61746164
	  - Subchunk2 Size  = 6801408
	Press a key to return to the menu ..
   \endcode
   After press any key, it will display:
   \code
	-I- PCM Load to 70100100, size 6801408
	Menu :
	------
	  P: Play the WAV file
	  D: Display the information of the WAV file
	\endcode
	If you press 'P', you will listen to music from the microphone.

User can choose any of the available options(capital) to perform the described action,
connecting earphone to the board to hear the sound of WAV file.
//-------------------------------------------------------------------------------------