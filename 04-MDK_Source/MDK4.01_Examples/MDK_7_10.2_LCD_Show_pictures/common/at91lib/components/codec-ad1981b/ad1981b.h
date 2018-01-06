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


#ifndef AC97_AD1981B_H
#define AC97_AD1981B_H

//------------------------------------------------------------------------------
//         External constants
//------------------------------------------------------------------------------
#define AD1981B_CMD(cmd, data) (((cmd << 16) & AT91C_AC97C_CADDR) | data)

#define AD1981B_RESET 0x00    // Reset 
    #define AD1981B_RESET_ID0 (1 << 0)
    #define AD1981B_RESET_ID1 (1 << 1)
    #define AD1981B_RESET_ID2 (1 << 2)
    #define AD1981B_RESET_ID3 (1 << 3)
    #define AD1981B_RESET_ID4 (1 << 4)
    #define AD1981B_RESET_ID5 (1 << 5)
    #define AD1981B_RESET_ID6 (1 << 6)
    #define AD1981B_RESET_ID7 (1 << 7)
    #define AD1981B_RESET_ID8 (1 << 8)                        
    #define AD1981B_RESET_ID9 (1 << 9)

#define AD1981B_MASTER 0x02    // Master Volume 
    #define AD1981B_MASTER_RMV0 (1 << 0)
    #define AD1981B_MASTER_RMV1 (1 << 1)
    #define AD1981B_MASTER_RMV2 (1 << 2)
    #define AD1981B_MASTER_RMV3 (1 << 3)           
    #define AD1981B_MASTER_RMV4 (1 << 4)
    #define AD1981B_MASTER_RM (1 << 7)
    #define AD1981B_MASTER_LMV0 (1 << 8)
    #define AD1981B_MASTER_LMV1 (1 << 9)
    #define AD1981B_MASTER_LMV2 (1 << 10)
    #define AD1981B_MASTER_LMV3 (1 << 11)           
    #define AD1981B_MASTER_LMV4 (1 << 12)
    #define AD1981B_MASTER_MM (1 << 15)

#define AD1981B_HEADPHONE 0x04    // Headphone Volume (optional) 
    #define AD1981B_HEADPHONE_RHV0 (1 << 0)
    #define AD1981B_HEADPHONE_RHV1 (1 << 1)
    #define AD1981B_HEADPHONE_RHV2 (1 << 2)
    #define AD1981B_HEADPHONE_RHV3 (1 << 3)
    #define AD1981B_HEADPHONE_RHV4 (1 << 4)                
    #define AD1981B_HEADPHONE_RM (1 << 7)
    #define AD1981B_HEADPHONE_LHV0 (1 << 8)
    #define AD1981B_HEADPHONE_LHV1 (1 << 9)
    #define AD1981B_HEADPHONE_LHV2 (1 << 10)
    #define AD1981B_HEADPHONE_LHV3 (1 << 11)
    #define AD1981B_HEADPHONE_LHV4 (1 << 12)   
    #define AD1981B_HEADPHONE_HPM (1 << 15)

#define AD1981B_MASTER_MONO        0x06    // Master Volume Mono (optional) 

#define AD1981B_MASTER_TONE        0x08    // Master Tone (Bass & Treble) (optional) 
#define AD1981B_PC_BEEP            0x0a    // PC Beep Volume (optinal) 
#define AD1981B_PHONE              0x0c    // Phone Volume (optional) 
#define AD1981B_MIC                0x0e    // MIC Volume 
#define AD1981B_LINE               0x10    // Line In Volume 
#define AD1981B_CD                 0x12    // CD Volume 
#define AD1981B_VIDEO              0x14    // Video Volume (optional) 
#define AD1981B_AUX                0x16    // AUX Volume (optional) 
#define AD1981B_PCM                0x18    // PCM Volume 
#define AD1981B_REC_SEL            0x1a    // Record Select 
#define AD1981B_REC_GAIN           0x1c    // Record Gain 
#define AD1981B_REC_GAIN_MIC       0x1e    // Record Gain MIC (optional) 
#define AD1981B_GENERAL_PURPOSE    0x20    // General Purpose (optional) 
#define AD1981B_3D_CONTROL         0x22    // 3D Control (optional) 
#define AD1981B_INT_PAGING         0x24    // Audio Interrupt & Paging (AC'97 2.3) 
#define AD1981B_POWERDOWN          0x26    // Powerdown control / status 
#define AD1981B_GENPURPOSE_LPBK (1 << 7)
#define AD1981B_GENPURPOSE_LPBK_NO (0 << 7)
#define AD1981B_GENPURPOSE_LPBK_YES (1 << 7)
#define AD1981B_GENPURPOSE_MS (1 << 8)
#define AD1981B_GENPURPOSE_MS_MIC1 (0 << 8)
#define AD1981B_GENPURPOSE_MS_MIC2 (1 << 8)
#define AD1981B_GENPURPOSE_MIX (1 << 9)
#define AD1981B_GENPURPOSE_MIX_MONO (0 << 9)
#define AD1981B_GENPURPOSE_MIX_MIC1 (1 << 9)

#define AD1981B_EXTAUDIO 0x2A
    #define AD1981B_EXTAUDIO_VRA (1 << 0)       // Variable Rate Audio
    #define AD1981B_EXTAUDIO_SPDIF (1 << 2)     // Subsystem Enable/Disable
    #define AD1981B_EXTAUDIO_SPSA (3 << 4)      // SPDIF Slot Assignment
    #define AD1981B_EXTAUDIO_SPCV (1 << 10)     // SPDIF Configuration Valid
    #define AD1981B_EXTAUDIO_VFORCE (1 << 15)   // Validity Force Bit

#define AD1981B_PMC_DAC 0x2C
#define AD1981B_PMC_ADC 0x32

#define AD1981B_SERIAL_CONFIG      0x74		    // Serial configuration
    #define AD1981B_SERIAL_CONFIG_SPLNK (1 << 0)    // SPDIF Link
    #define AD1981B_SERIAL_CONFIG_REGM0 (1 << 12)   // Master Codec REG Mask
    #define AD1981B_SERIAL_CONFIG_REGM1 (1 << 13)   // Slave1 Codec REG Mask
    #define AD1981B_SERIAL_CONFIG_REGM2 (1 << 14)   // Slave2 Codec REG Mask

#define AD1981B_MISC_CONTROL       0x76         // Miscellaneous control
    #define AD1981B_MISC_CONTROL_FMXE (1 << 9)  // Front DAC into Mixer Enable

#define AD1981B_SAMPLE_RATE_0      0x78		// set up the equalizer coefficients.
#define AD1981B_SAMPLE_RATE_1      0x7A		// set up the equalizer coefficients.

#define AD1981B_VENDOR_ID1         0x7C		// Vendor 1
#define AD1981B_VENDOR_ID2         0x7E     // Vendor 2

#define AD1981B_ID_1	0x4144
#define AD1981B_ID_2	0x5374

#endif //#ifndef AC97_AD1981B_H
