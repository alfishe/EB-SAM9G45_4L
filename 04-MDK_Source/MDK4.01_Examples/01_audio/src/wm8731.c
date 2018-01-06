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
/// WM8731 driver
/// 
/// !Usage
/// 
/// Explanation on the usage of the code made available through the header file.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include "wm8731.h"
#include <board.h>

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Internal functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Read data from WM8731 Register.
/// \param pTwid   pointer to twi driver structure
/// \param device   twi slave address
/// \param regAddr       register address to read.
//------------------------------------------------------------------------------
unsigned short WM8731_Read(Twid *pTwid,
                            unsigned int device,
                            unsigned int regAddr)
{
    unsigned short bitsDataRegister;
    unsigned char Tdata[2]={0,0};

    TWID_Read(pTwid, device, regAddr, 0, Tdata, 2, 0);
    bitsDataRegister = (Tdata[0] << 8) | Tdata[1];
    return bitsDataRegister;
}

//------------------------------------------------------------------------------
/// Write data to WM8731 Register.
/// \param pTwid   pointer to twi driver structure
/// \param device   twi slave address
/// \param regAddr       register address to read.
/// \param data    data to write
//------------------------------------------------------------------------------
void WM8731_Write(Twid *pTwid,
                   unsigned int device,
                   unsigned int regAddr,
                   unsigned short data)
{
    unsigned char tmpData[2];
    unsigned short tmp;
    tmp = ((regAddr & 0x7f) << 9) | (data & 0x1ff);

    tmpData[0] = (tmp & 0xff00) >> 8;
    tmpData[1] = tmp & 0xff;
    TWID_Write(pTwid, device, regAddr, 0, tmpData, 2, 0);
}

//------------------------------------------------------------------------------
/// Init WM8731 to DAC mode.
/// \param pTwid   pointer to twi driver structure
/// \param device   twi slave address
//------------------------------------------------------------------------------
unsigned char WM8731_DAC_Init(Twid *pTwid,
                   unsigned int device)
{
    // reset
    WM8731_Write(pTwid, device, WM8731_REG_RESET, 0);

    // analogue audio path control
    WM8731_Write(pTwid, device, WM8731_REG_ANALOGUE_PATH_CTRL, 0x12);

    // digital audio path control
    WM8731_Write(pTwid, device, WM8731_REG_DIGITAL_PATH_CTRL, 0x00);

    // power down control
    WM8731_Write(pTwid, device, WM8731_REG_PWDOWN_CTRL, 0x7);

    // Active control
    WM8731_Write(pTwid, device, WM8731_REG_ACTIVE_CTRL, 0x01);

    return 0;
}
