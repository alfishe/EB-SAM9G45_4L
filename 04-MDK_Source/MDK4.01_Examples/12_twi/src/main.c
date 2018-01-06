/**************************************************************************************
* File Name          : main.c
* Date First Issued  : 01/10/2010
* Description        : Main program body
***************************************************************************************
***************************************************************************************
* History:
* 01/10/2010:          V1.0
**************************************************************************************/

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include <board.h>
#include <pio/pio.h>
#include <dbgu/dbgu.h>
#include <utility/trace.h>
#include <utility/assert.h>
#include <pmc/pmc.h>
#include <irq/irq.h>
#include <twi/twi.h>
#include <stdio.h>

//------------------------------------------------------------------------------
//         Local constants
//------------------------------------------------------------------------------
/// Slave address of the device on the TWI bus.
#define SLAVE_ADDRESS       0x50
/// Memory size in bytes (example AT24C1024)
#define MEMORY_SIZE         512
/// Page size in bytes (example AT24C1024)
#define PAGE_SIZE           256

/// Memory size in bytes (example AT24C512)
//#define MEMORY_SIZE         512
/// Page size in bytes (example AT24C512)
//#define PAGE_SIZE           128

#ifndef AT91C_BASE_TWI
#define AT91C_BASE_TWI AT91C_BASE_TWI0
#define PINS_TWI       PINS_TWI0
#define AT91C_ID_TWI   AT91C_ID_TWI0
#endif

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------
/// Pio pins to configure.
const Pin pins[] = {PINS_DBGU, PINS_TWI};

/// The slave device instance
typedef struct {

    /// PageAddress of the slave device
    unsigned short pageAddress;
    /// Offset of the memory access
    unsigned short offsetMemory;
    //unsigned int status;
    /// Read address of the request
    unsigned char acquireAddress;
    /// Memory buffer
    unsigned char pMemory[MEMORY_SIZE];

} SlaveDeviceDriver;

SlaveDeviceDriver EmulateDriver;

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
void TWI0_IrqHandler(void)
{
    unsigned int status;

    status = TWI_GetStatus(AT91C_BASE_TWI);

    if( ((status & AT91C_TWI_SVACC) == AT91C_TWI_SVACC)
     && (EmulateDriver.acquireAddress == 0) ) {
        TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_SVACC);
        TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_RXRDY
                                   | AT91C_TWI_GACC
                                   | AT91C_TWI_NACK_SLAVE 
                                   | AT91C_TWI_EOSACC 
                                   | AT91C_TWI_SCLWS );
        EmulateDriver.acquireAddress++;
        EmulateDriver.pageAddress = 0;
        EmulateDriver.offsetMemory = 0;
    }

    if( (status & AT91C_TWI_GACC) == AT91C_TWI_GACC ) {
        printf("General Call Treatment\n\r");
        printf("not treated");
    }

    if( ((status & AT91C_TWI_SVACC) == AT91C_TWI_SVACC )
       && ((status & AT91C_TWI_GACC) == 0 )
       && ((status & AT91C_TWI_RXRDY) == AT91C_TWI_RXRDY ) ){

        if( EmulateDriver.acquireAddress == 1 ) {
            // Acquire LSB address
            EmulateDriver.pageAddress = (TWI_ReadByte(AT91C_BASE_TWI) & 0xFF);
            EmulateDriver.acquireAddress++;
        }
        else if( EmulateDriver.acquireAddress == 2 ) {
            // Acquire MSB address
            EmulateDriver.pageAddress |= (TWI_ReadByte(AT91C_BASE_TWI) & 0xFF)<<8;
            EmulateDriver.acquireAddress++;
        }
        else {
            // Read one byte of data from master to slave device
            EmulateDriver.pMemory[(PAGE_SIZE*EmulateDriver.pageAddress)+EmulateDriver.offsetMemory] = (TWI_ReadByte(AT91C_BASE_TWI) & 0xFF);
            EmulateDriver.offsetMemory++;
        }
    }
    else if( ((status & AT91C_TWI_TXRDY_SLAVE) == AT91C_TWI_TXRDY_SLAVE )
          && ((status & AT91C_TWI_TXCOMP_SLAVE) == AT91C_TWI_TXCOMP_SLAVE )
          && ((status & AT91C_TWI_EOSACC) == AT91C_TWI_EOSACC ) ) {
        // End of transfert, end of slave access
        EmulateDriver.offsetMemory = 0;
        EmulateDriver.acquireAddress = 0;
        EmulateDriver.pageAddress = 0;
        TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_SVACC);
        TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_RXRDY
                                    | AT91C_TWI_GACC
                                    | AT91C_TWI_NACK_SLAVE 
                                    | AT91C_TWI_EOSACC 
                                    | AT91C_TWI_SCLWS );
    }
    else if( ((status & AT91C_TWI_SVACC) == AT91C_TWI_SVACC )
       && ((status & AT91C_TWI_GACC) == 0 )
       && ( EmulateDriver.acquireAddress == 3 )
       && ((status & AT91C_TWI_SVREAD) == AT91C_TWI_SVREAD )
       && ((status & AT91C_TWI_NACK_SLAVE) == 0 ) ) {
        // Write one byte of data from slave to master device
        TWI_WriteByte(AT91C_BASE_TWI, EmulateDriver.pMemory[(PAGE_SIZE*EmulateDriver.pageAddress)+EmulateDriver.offsetMemory]);
        EmulateDriver.offsetMemory++;
    }
}

//------------------------------------------------------------------------------
/// Default main() function. Initializes the DBGU and writes a string on the
/// DBGU.
//------------------------------------------------------------------------------
int main(void)
{
    unsigned int i;

    PIO_Configure(pins, PIO_LISTSIZE(pins));
    DBGU_Configure(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic TWI Slave Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    PMC_EnablePeripheral(AT91C_ID_TWI);

    for (i=0; i<MEMORY_SIZE; i++) {
        EmulateDriver.pMemory[i] = 0;
    }
    EmulateDriver.offsetMemory = 0;
    EmulateDriver.acquireAddress = 0;
    EmulateDriver.pageAddress = 0;

    // Configure TWI as slave
    printf("-I- Configuring the TWI in slave mode\n\r");
    TWI_ConfigureSlave(AT91C_BASE_TWI, SLAVE_ADDRESS);

    // Clear receipt buffer
    TWI_ReadByte(AT91C_BASE_TWI);

    TRACE_DEBUG("TWI is in slave mode\n\r");

    IRQ_ConfigureIT(AT91C_ID_TWI, 0, TWI0_IrqHandler);
    IRQ_EnableIT(AT91C_ID_TWI);

    TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_SVACC );

    while (1) {
    }
}


