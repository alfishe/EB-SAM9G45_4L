/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: clock-arch.c,v 1.2 2006/06/12 08:00:31 adam Exp $
 */

/**
 * \file
 *         Implementation of architecture-specific clock functionality
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

//-----------------------------------------------------------------------------
//         Headers
//-----------------------------------------------------------------------------

#include <board.h>
#include <tc/tc.h>
#include <irq/irq.h>
#include <utility/trace.h>
#include <utility/assert.h>
#include "clock-arch.h"

//-----------------------------------------------------------------------------
///        Internal variables
//-----------------------------------------------------------------------------

/// clock tick count
static unsigned int clockTick;

//-----------------------------------------------------------------------------
///        Local functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Timer Counter 0 interrupt handler.
//-----------------------------------------------------------------------------
static void ISR_Tc0(void)
{
    // Acknowledge interrupt
    AT91C_BASE_TC0->TC_SR;

    // Increase tick
    clockTick ++;
}

//-----------------------------------------------------------------------------
///        Global functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Initialize for clock time operation
//-----------------------------------------------------------------------------
void clock_init(void)
{
    unsigned int div, tcclks;

    printf("P: clock time initialize - TC0\n\r");

    // Enable TC0
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TC0;
    ASSERT(TC_FindMckDivisor(CLOCK_CONF_SECOND, BOARD_MCK, &div, &tcclks),
           "F: Fail to generate the desired frequency %d\n\r",
           CLOCK_CONF_SECOND);
    TC_Configure(AT91C_BASE_TC0, tcclks | AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO);
    AT91C_BASE_TC0->TC_RC = BOARD_MCK / (CLOCK_CONF_SECOND * div);

    // Configure TC0 interrupt
    IRQ_DisableIT(AT91C_ID_TC0);
    IRQ_ConfigureIT(AT91C_ID_TC0, 0, ISR_Tc0);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    IRQ_EnableIT(AT91C_ID_TC0);

    // Clear tick value
    clockTick = 0;

    // Start timer
    TC_Start(AT91C_BASE_TC0);
}

//-----------------------------------------------------------------------------
/// Read for clock time (ms)
//-----------------------------------------------------------------------------
clock_time_t
clock_time(void)
{
    return clockTick;
}
/*---------------------------------------------------------------------------*/
