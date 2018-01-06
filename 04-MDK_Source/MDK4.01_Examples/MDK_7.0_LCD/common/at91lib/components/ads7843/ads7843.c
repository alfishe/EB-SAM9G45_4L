/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
/// 
/// ADS7843 driver
/// 
/// !Usage
/// 
/// Explanation on the usage of the code made available through the header file.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <board.h>

#ifdef BOARD_TSC_ADS7843

#include <pio/pio.h>
#include <spi/spi.h>

#include "ads7843.h"

//------------------------------------------------------------------------------
//         Defines
//------------------------------------------------------------------------------

#define ADS_CTRL_PD0              (1 << 0)        // PD0
#define ADS_CTRL_PD1              (1 << 1)        // PD1
#define ADS_CTRL_DFR              (1 << 2)        // SER/DFR
#define ADS_CTRL_EIGHT_BITS_MOD   (1 << 3)        // Mode
#define ADS_CTRL_START            (1 << 7)        // Start Bit
#define ADS_CTRL_SWITCH_SHIFT     4               // Address setting

#if defined(cortexm3)
// Get X position command
#define CMD_Y_POSITION ((1 << ADS_CTRL_SWITCH_SHIFT) | ADS_CTRL_START | ADS_CTRL_PD0 | ADS_CTRL_PD1)
// Get Y position command
#define CMD_X_POSITION ((5 << ADS_CTRL_SWITCH_SHIFT) | ADS_CTRL_START | ADS_CTRL_PD0 | ADS_CTRL_PD1)
#else
// Get X position command
#define CMD_X_POSITION ((1 << ADS_CTRL_SWITCH_SHIFT) | ADS_CTRL_START | ADS_CTRL_PD0 | ADS_CTRL_PD1)
// Get Y position command
#define CMD_Y_POSITION ((5 << ADS_CTRL_SWITCH_SHIFT) | ADS_CTRL_START | ADS_CTRL_PD0 | ADS_CTRL_PD1)
#endif

// Enable penIRQ
#define CMD_ENABLE_PENIRQ  ((1 << ADS_CTRL_SWITCH_SHIFT) | ADS_CTRL_START)


#define AT91C_TOUCHSCREEN_TIMEOUT        5000000

#define DELAY_BEFORE_SPCK          (200 << 16) // 2us min (tCSS) <=> 200/100 000 000 = 2us
#define DELAY_BETWEEN_CONS_COM     (0xf << 24) // 5us min (tCSH) <=> (32 * 15) / (100 000 000) = 5us
  
//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

/// Pins used by SPI
static const Pin pinsSPI[]  = {BOARD_TSC_SPI_PINS, BOARD_TSC_NPCS_PIN};

/// Touch screen BUSY pin
static    const Pin pinBusy[] = {PIN_TCS_BUSY};
/// Touch screen CS pin
static    const Pin pinNss[]  = {BOARD_TSC_NPCS_PIN};

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Generic function to send a command to the touchscreen controller
/// \param bCmd command to send
/// \return command result
//-----------------------------------------------------------------------------
static unsigned int SendCommand(unsigned char bCmd)
{
#if defined(cortexm3)
    int i;
    AT91S_SPI *spi = BOARD_TSC_SPI_BASE;
    unsigned int  uResult = 0;
    // (volatile declaration needed for code optimisation by compiler)
    volatile unsigned char bufferRX[3];
    volatile unsigned char bufferTX[3];

    bufferRX[0] = 0;
    bufferRX[1] = 0;
    bufferRX[2] = 0;

    bufferTX[0] = bCmd;
    bufferTX[1] = 0;
    bufferTX[2] = 0;

    // Send command
    i = 0;
    while ((spi->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
    spi->SPI_TDR = bufferTX[i] | SPI_PCS(BOARD_TSC_NPCS);
    while ((spi->SPI_SR & AT91C_SPI_TDRE) == 0);

    while (PIO_Get(pinBusy) == 1);
    while ((spi->SPI_SR & AT91C_SPI_RDRF) == 0);
    bufferRX[i] = spi->SPI_RDR & 0xFFFF;

    // Read data
    for (i = 1; i < 3; i++) {

        while ((spi->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
        spi->SPI_TDR = bufferTX[i] | SPI_PCS(BOARD_TSC_NPCS);
        while ((spi->SPI_SR & AT91C_SPI_TDRE) == 0);

        while ((spi->SPI_SR & AT91C_SPI_RDRF) == 0);
        bufferRX[i] = spi->SPI_RDR & 0xFFFF;
    }

    uResult = ((unsigned int)bufferRX[1] << 8) | (unsigned int)bufferRX[2];
    uResult = uResult >> 4;

    return uResult;

#else // AT91SAM7, SAM9

    unsigned int  uResult = 0;
    unsigned int  uTimeout = 0;
    // (volatile declaration needed for code optimisation by compiler)
    volatile unsigned char bufferRX[3];
    volatile unsigned char bufferTX[3];

    AT91PS_PDC pPdc = (AT91PS_PDC) &(AT91C_BASE_SPI0->SPI_RPR);
    unsigned int dStatus;

    bufferRX[0] = 0;
    bufferRX[1] = 0;
    bufferRX[2] = 0;

    bufferTX[0] = bCmd;
    bufferTX[1] = 0;
    bufferTX[2] = 0;//bCmd;

    // Send Command and data through the SPI
    pPdc->PDC_PTCR = AT91C_PDC_RXTDIS;
    pPdc->PDC_RPR = (unsigned int) bufferRX;
    pPdc->PDC_RCR = 3;

    pPdc->PDC_PTCR = AT91C_PDC_TXTDIS;
    pPdc->PDC_TPR = (unsigned int) bufferTX;
    pPdc->PDC_TCR = 3;

    pPdc->PDC_PTCR = AT91C_PDC_RXTEN;
    pPdc->PDC_PTCR = AT91C_PDC_TXTEN;

    do {
        dStatus = AT91C_BASE_SPI0->SPI_SR;
        uTimeout++;
    }
    while ((( dStatus & AT91C_SPI_RXBUFF) != AT91C_SPI_RXBUFF) && (uTimeout < AT91C_TOUCHSCREEN_TIMEOUT));

    pPdc->PDC_PTCR = AT91C_PDC_RXTDIS;
    pPdc->PDC_PTCR = AT91C_PDC_TXTDIS;

    uResult = ((unsigned int)bufferRX[1] << 8) | (unsigned int)bufferRX[2];
    uResult = uResult >> 4;

    return uResult;
#endif
}

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Get position of the pen by ask the ADS controller (SPI)
/// \param px_pos pointer to the horizontal position
/// \param py_pos pointer to the vertical position
//-----------------------------------------------------------------------------
void ADS7843_GetPosition(unsigned int* px_pos, unsigned int* py_pos)
{
    // Get X position
    *px_pos = SendCommand(CMD_X_POSITION);
    // Get Y position
    *py_pos = SendCommand(CMD_Y_POSITION);
    // Switch to full power mode
    SendCommand(CMD_ENABLE_PENIRQ);
}


//-----------------------------------------------------------------------------
/// Initialization of the SPI for communication with ADS7843 component
//-----------------------------------------------------------------------------
void ADS7843_Initialize(void)
{
    volatile unsigned int uDummy;
      
    // Configure pins
    PIO_Configure(pinsSPI, PIO_LISTSIZE(pinsSPI));

    PIO_Configure(pinBusy, PIO_LISTSIZE(pinBusy));

    SPI_Configure(AT91C_BASE_SPI0,
                  AT91C_ID_SPI0,
                  AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | SPI_PCS(BOARD_TSC_NPCS) // Value of the SPI configuration register.
    );

#if defined(cortexm3)
    SPI_ConfigureNPCS(AT91C_BASE_SPI0, BOARD_TSC_NPCS,
                      AT91C_SPI_NCPHA | (AT91C_SPI_DLYBS & DELAY_BEFORE_SPCK) | AT91C_SPI_CSAAT |
                      (AT91C_SPI_DLYBCT & DELAY_BETWEEN_CONS_COM) | (0xC8 << 8) );
#else // AT91SAM7, SAM9    
    SPI_ConfigureNPCS(AT91C_BASE_SPI0, BOARD_TSC_NPCS,
                      AT91C_SPI_NCPHA | (AT91C_SPI_DLYBS & DELAY_BEFORE_SPCK) |
                      (AT91C_SPI_DLYBCT & DELAY_BETWEEN_CONS_COM) | (0xC8 << 8) );
#endif    

    SPI_Enable(AT91C_BASE_SPI0);

    for (uDummy=0; uDummy<100000; uDummy++);

    uDummy = AT91C_BASE_SPI0->SPI_SR;
    uDummy = AT91C_BASE_SPI0->SPI_RDR;

    SendCommand(CMD_ENABLE_PENIRQ);
}

//-----------------------------------------------------------------------------
/// Reset the ADS7843
//-----------------------------------------------------------------------------
void ADS7843_Reset(void)
{
    // Disable SPI 0
    SPI_Disable(AT91C_BASE_SPI0);
}

#endif //#ifdef BOARD_TSC_ADS7843
