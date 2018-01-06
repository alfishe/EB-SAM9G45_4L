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
 
//-----------------------------------------------------------------------------
//         Headers
//-----------------------------------------------------------------------------
#include <drivers/twi/twid.h>
#include <omnivision/ov9655/ov9655.h>
#include <omnivision/omnivision.h>
#include <utility/trace.h>

/// Slave address of OMNIVISION chips.
#define OV_CAPTOR_ADDRESS   (0x60>>1)

/// terminating list entry for register in configuration file
#define OV9650_REG_TERM 0xFF
/// terminating list entry for value in configuration file
#define OV9650_VAL_TERM 0xFF 

//-----------------------------------------------------------------------------
//         Local Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Read PID and VER
/// \param pTwid TWI interface
/// \return VER | (PID<<8)
//-----------------------------------------------------------------------------
static unsigned short ov95x_id(Twid *pTwid)
{
    unsigned char id=0;
    unsigned char ver=0;

    // OV9650_PID
    ov965x_read_reg(pTwid, 0x0A, &id);
    TRACE_INFO("PID  = 0x%X\n\r", id);

    // OV9650_VER
    ov965x_read_reg(pTwid, 0x0B, &ver);
    TRACE_INFO("VER  = 0x%X\n\r", ver);

    return((unsigned short)(id <<8) | ver);
}

//-----------------------------------------------------------------------------
/// Read Manufacturer
/// \param pTwid TWI interface
/// \return 0 if error; 1 if the good captor is present
//-----------------------------------------------------------------------------
static unsigned char ov95x_Manufacturer(Twid *pTwid)
{
    unsigned char midh=0;
    unsigned char midl=0;
    unsigned status = 0;

    // OV9650_MIDH
    ov965x_read_reg(pTwid, 0x1C, &midh);
    TRACE_DEBUG("MIDH = 0x%X\n\r", midh);

    // OV9650_MIDL
    ov965x_read_reg(pTwid, 0x1D, &midl);
    TRACE_DEBUG("MIDL = 0x%X\n\r", midl);

    if(( midh == 0x7F) && (midl == 0xA2)) {
        status = 1;
    }
    return(status);
}
//-----------------------------------------------------------------------------
/// ov95x_TestWrite
/// \param pTwid TWI interface
/// \return 1 if  the write is correct; 0 otherwise
//-----------------------------------------------------------------------------
static unsigned char ov95x_TestWrite(Twid *pTwid)
{
    unsigned char value=0;
    unsigned char oldvalue=0;

    // OV9650_BLUE
    ov965x_read_reg(pTwid, 0x01, &oldvalue);
    ov965x_write_reg(pTwid, 0x01, 0xAD);
    ov965x_read_reg(pTwid, 0x01, &value);
    if( value != 0xAD ) {
        return(0);
    }
    // return old value
    ov965x_write_reg(pTwid, 0x01, oldvalue);
    ov965x_read_reg(pTwid, 0x01, &value);
    if( value != oldvalue ) {
        return(0);
    }
    return(1);
}

//-----------------------------------------------------------------------------
//         Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Read a value from a register in an OV9650 sensor device.
/// \param pTwid TWI interface
/// \param reg Register to be read
/// \param pData Data read
/// \return 0 if no error, otherwize TWID_ERROR_BUSY
//-----------------------------------------------------------------------------
unsigned char ov965x_read_reg(Twid *pTwid, unsigned char reg, unsigned char *pData)
{
    unsigned char status;

    status = TWID_Write( pTwid, OV_CAPTOR_ADDRESS, 0, 0, &reg, 1, 0);
    status |= TWID_Read( pTwid, OV_CAPTOR_ADDRESS, 0, 0, pData, 1, 0);
    //status = TWID_Read(pTwid, OV_CAPTOR_ADDRESS, reg, 1, pData, 1, 0);
    if( status != 0 ) {
        TRACE_ERROR("ov965x_read_reg pb");
    }
    return status;
}

//-----------------------------------------------------------------------------
/// Write a value to a register in an OV9650 sensor device.
/// \param pTwid TWI interface
/// \param reg Register to be write
/// \param val Value to be writte
/// \return 0 if no error, otherwize TWID_ERROR_BUSY
//-----------------------------------------------------------------------------
unsigned char ov965x_write_reg(Twid *pTwid, unsigned char reg, unsigned char val)
{
    unsigned char status;

    status = TWID_Write(pTwid, OV_CAPTOR_ADDRESS, reg, 1, &val, 1, 0);
    if( status != 0 ) {
        TRACE_ERROR("ov965x_write_reg pb");
    }

    return status;
}

//-----------------------------------------------------------------------------
/// Initialize a list of OV9650 registers.
/// The list of registers is terminated by the pair of values
/// { OV9650_REG_TERM, OV9650_VAL_TERM }.
/// Returns zero if successful, or non-zero otherwise.
/// \param pTwid TWI interface
/// \param pReglist Register list to be written
/// \return 0 if no error, otherwize TWID_ERROR_BUSY
//-----------------------------------------------------------------------------
int ov965x_write_regs(Twid *pTwid, const struct ov965x_reg* pReglist)
{
    int err;
    int size=0;
    const struct ov965x_reg *pNext = pReglist;
    unsigned int i=0;

    TRACE_DEBUG("ov965x_write_regs:");
    while (!((pNext->reg == OV9650_REG_TERM) && (pNext->val == OV9650_VAL_TERM))) {
        err = ov965x_write_reg(pTwid, pNext->reg, pNext->val);
        TRACE_DEBUG_WP("+(%d) ", size);
        size++;

        //delay(1);
        for(i=0; i<6000; i++ ) {
            *(unsigned int*)0x20400000 = 0;
        }

        if (err == TWID_ERROR_BUSY){
            TRACE_ERROR("ov965x_write_regs: TWI ERROR\n\r");
            return err;
        }
        pNext++;
    }
    TRACE_DEBUG_WP("\n\r");
    return 0;
}

//-----------------------------------------------------------------------------
/// Dump all register
/// \param pTwid TWI interface
//-----------------------------------------------------------------------------
void ov95x_DumpRegisters(Twid *pTwid)
{
    int i;
    unsigned char value;

    TRACE_INFO_WP("Dump all camera register\n\r");
    for(i = 0; i <= 0xDA; i++) {
        value = 0;
        ov965x_read_reg(pTwid, i, &value);
        TRACE_INFO_WP("[0x%02x]=0x%02x ", i, value);
        if( ((i+1)%5) == 0 ) {
            TRACE_INFO_WP("\n\r");
        }        
    }
    TRACE_INFO_WP("\n\r");
}

//-----------------------------------------------------------------------------
/// Sequence For correct operation of the sensor
/// \param pTwid TWI interface
/// \return 1 if initialization ok, otherwise 0
//-----------------------------------------------------------------------------
unsigned char ov965x_init(Twid *pTwid)
{
    unsigned short id=0;

    id = ov95x_id(pTwid);
    if( (id>>8) == 0x96 ) {
        TRACE_DEBUG("ID and PID OK\n\r");
        if( ov95x_Manufacturer(pTwid) == 1 ) {
            TRACE_DEBUG("Manufacturer OK\n\r");
            if( ov95x_TestWrite(pTwid) == 1 ) {
                return 1;
            }
            else {
                TRACE_ERROR("Problem captor: bad write\n\r");
            }
        }
        else {
            TRACE_ERROR("Problem captor: bad Manufacturer\n\r");
        }
    }
    else {
        TRACE_ERROR("Problem captor: bad PID\n\r");
    }
    TRACE_INFO("Problem: captor not responding\n\r");
    return 0;
}


