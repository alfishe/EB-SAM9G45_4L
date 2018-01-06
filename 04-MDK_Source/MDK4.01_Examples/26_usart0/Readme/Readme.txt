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
The Basic USART project will show you how to use USART0 or USART3. 

Requirements
===========================
If you use USART0, you should link USART0(J16) to the PC's COM; else if you use USART3, 
you should link USART3(j17) to the PC's COM; 

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
-# Upon startup, the application will output the following lines on the DBGU.
   \code
	TEST USART0...
	Please input:
   \endcode
-# Eventually, the test result (pass or fail) will be output on the DBGU.
//-------------------------------------------------------------------------------------


