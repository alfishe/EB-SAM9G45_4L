/***************************************************************************************
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
#include <irq/irq.h>
#include <rtt/rtt.h>
#include <utility/trace.h>

#include <stdio.h>

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------

/// Device is in the main menu.
#define STATE_MAINMENU      0
/// User is setting an alarm time
#define STATE_SETALARM      1

/// Define AT91C_BASE_RTTC if not already done
#if !defined(AT91C_BASE_RTTC)
    #define AT91C_BASE_RTTC     AT91C_BASE_RTTC0
#endif

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

 /// Current device state.
volatile unsigned char state;

/// New alarm time being currently entered.
volatile unsigned int newAlarm;

/// Indicates if an alarm has occured but has not been cleared.
volatile unsigned char alarmed;

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Updates the DBGU display to show the current menu and the current time
/// depending on the device state.
//------------------------------------------------------------------------------
void RefreshDisplay(void)
{
    printf("%c[2J\r", 27);
    printf("Time: %u\n\r", RTT_GetTime(AT91C_BASE_RTTC));

    // Display alarm
    if (alarmed) {

        printf("!!! ALARM !!!\n\r");
    }

    // Main menu
    if (state == STATE_MAINMENU) {

        printf("Menu:\n\r");
        printf(" r - Reset timer\n\r");
        printf(" s - Set alarm\n\r");
        if (alarmed) {

            printf(" c - Clear alarm notification\n\r");
        }
        printf("\n\rChoice? ");
    }
    // Set alarm
    else if (state == STATE_SETALARM) {

        printf("Enter alarm time: ");
        if (newAlarm != 0) {

            printf("%u", newAlarm);
        }
    }
}

//------------------------------------------------------------------------------
/// Interrupt handler for the RTT. Displays the current time on the DBGU.
//------------------------------------------------------------------------------
void RTT_IrqHandler(void)
{
    unsigned int status;

    // Get RTT status
    status = RTT_GetStatus(AT91C_BASE_RTTC);

    // Time has changed, refresh display
    if ((status & AT91C_RTTC_RTTINC) == AT91C_RTTC_RTTINC) {

        RefreshDisplay();
    }

    // Alarm
    if ((status & AT91C_RTTC_ALMS) == AT91C_RTTC_ALMS) {

        alarmed = 1;
        RefreshDisplay();
    }
}

//------------------------------------------------------------------------------
/// Configures the RTT to generate a one second tick, which triggers the RTTINC
/// interrupt.
//------------------------------------------------------------------------------
void ConfigureRtt(void)
{
    unsigned int previousTime;

    // Configure RTT for a 1 second tick interrupt
    RTT_SetPrescaler(AT91C_BASE_RTTC, 32768);
    previousTime = RTT_GetTime(AT91C_BASE_RTTC);
    while (previousTime == RTT_GetTime(AT91C_BASE_RTTC));

    // Enable RTT interrupt
    #if !defined(cortexm3)
    IRQ_ConfigureIT(AT91C_ID_SYS, 0, RTT_IrqHandler);
    IRQ_EnableIT(AT91C_ID_SYS);
    #else
    IRQ_ConfigureIT(AT91C_ID_RTT, 0, RTT_IrqHandler);
    IRQ_EnableIT(AT91C_ID_RTT);
    #endif
    RTT_EnableIT(AT91C_BASE_RTTC, AT91C_RTTC_RTTINCIEN);
}

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Initializes the RTT, displays the current time and allows the user to
/// perform several actions: clear the timer, set an alarm, etc.
//------------------------------------------------------------------------------
int main(void)
{
    unsigned char c;
	unsigned char length;

    // Enable DBGU
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("-- Basic RTT Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    // Configure RTT
    ConfigureRtt();

    // Initialize state machine
    state = STATE_MAINMENU;
    alarmed = 0;
    RefreshDisplay();

    // User input loop
    while (1) { 

        // Wait for user input
        c = DBGU_GetChar();

        // Main menu mode
        if (state == STATE_MAINMENU) {

            // Reset timer
            if (c == 'r') {

                ConfigureRtt();
                RefreshDisplay();
            }
            // Set alarm
            else if (c == 's') {

                state = STATE_SETALARM;
                newAlarm = 0;
                RefreshDisplay();
            }
            // Clear alarm
            else if ((c == 'c') && alarmed) {

                alarmed = 0;
                RefreshDisplay();
            }
        }
        // Set alarm mode
        if (state == STATE_SETALARM) {
			// Disable RTT Interrupt
			AT91C_BASE_RTTC->RTTC_RTMR &= ~(AT91C_RTTC_RTTINCIEN | AT91C_RTTC_ALMIEN);

			length = 0;
			while (1)
			{
				c = DBGU_GetChar();

	            // Number
	            if ((c >= '0') && (c <= '9')) {
					printf("%c", c);
	
	                newAlarm = newAlarm * 10 + c - '0';
					length++;
	            }
	            // Backspace
	            else if (c == 8) {
					printf("\b \b");
	
	                newAlarm /= 10;
					length--;
	            }
	            // Enter key
	            else if (c == 13) {
					// The number is too large
					if (length > 10)
					{
						printf("\nThe input number is invaild!\n");
						// Enable RTT Interrupt
						AT91C_BASE_RTTC->RTTC_RTMR |= AT91C_RTTC_RTTINCIEN;
					}
	                else if (newAlarm != 0) { // Avoid newAlarm = 0 case
						// Enable RTT Interrupt
						AT91C_BASE_RTTC->RTTC_RTMR |= (AT91C_RTTC_RTTRST | AT91C_RTTC_RTTINCIEN | AT91C_RTTC_ALMIEN);
	                    RTT_SetAlarm(AT91C_BASE_RTTC, newAlarm);
	                }
					else
					{
						// Enable RTT Interrupt
						AT91C_BASE_RTTC->RTTC_RTMR |= AT91C_RTTC_RTTINCIEN;
					}
	               
	                state = STATE_MAINMENU;
	                RefreshDisplay();
					break;
	            }
			}
        }
    }
}

