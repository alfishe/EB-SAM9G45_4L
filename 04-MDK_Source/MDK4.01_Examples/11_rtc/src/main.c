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
#include <rtc/rtc.h>
#include <utility/trace.h>

#include <stdio.h>
#include <stdarg.h>
//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------

/// Main menu is being displayed.
#define STATE_MENU              0
/// Time is being edited.
#define STATE_SET_TIME          1
/// Date is being edited.
#define STATE_SET_DATE          2
/// Time alarm is being edited.
#define STATE_SET_TIME_ALARM    3
/// Date alarm is being edited.
#define STATE_SET_DATE_ALARM    4

/// Maximum size of edited string
#define MAX_EDIT_SIZE       10

/// Macro for check digi character
#define IsDigitChar(c) ((c) >= '0' && (c) <='9')
/// Macro for converting char to digit
#define CharToDigit(c) ((c) - '0')

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

 /// Current state of application.
static unsigned int bState = STATE_MENU;

/// Edit index.
//static unsigned char editIndex;

/// Edited hour.
static unsigned char newHour;
/// Edited minute.
static unsigned char newMinute;
/// Edited second.
static unsigned char newSecond;

/// Edited year.
static unsigned short newYear;
/// Edited month.
static unsigned char newMonth;
/// Edited day.
static unsigned char newDay;
/// Edited day-of-the-week.
static unsigned char newWeek;

/// Edited alarm hour.
//static unsigned char newAlarmHour;
/// Edited alarm minute.
//static unsigned char newAlarmMinute;
/// Edited alarm second.
//static unsigned char newAlarmSecond;

/// Indicates if alarm has been trigerred and not yet cleared.
static unsigned char alarmTriggered = 0;

/// store time string
static char time[8+1] = {'0', '0', ':', '0', '0', ':', '0', '0','\0'};
/// store date string
static char date[10+1] = {'0', '0', '/', '0', '0', '/', '0', '0', '0', '0', '\0'};
/// week string
static char pDayNames[7][4] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
/// console erase sequence
static char pEraseSeq[] = "\b \b";
//output format string buffer
static char calendar[80];
//for idendify refreshing menu
static unsigned int bMenuShown = 0;

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// Display a string on the DBGU.
//------------------------------------------------------------------------------
static inline void DBGU_puts(const char *pStr)
{
  while(*pStr) {
    DBGU_PutChar(*pStr++);
  }
}

//------------------------------------------------------------------------------
/// print a formatted string into a buffer
//------------------------------------------------------------------------------
static signed int PrnToBuf(char *pBuf, const char *pFormat, ...)
{
    va_list    ap;
    signed int rc;

    va_start(ap, pFormat);
    rc = vsprintf(pBuf, pFormat, ap);
    va_end(ap);
    
    return rc;
}

//------------------------------------------------------------------------------
/// Get new time, successful value is put in newHour, newMinute, newSecond
//------------------------------------------------------------------------------
static int GetNewTime()
{
  char key;
  int i=0;
  
  /* clear setting variable */
  newHour = newMinute = newSecond = 0xFF;
  
  /* use time[] as a format template */
  while(1) {
    
    key = DBGU_GetChar();
    
    /* end input */
    if(key == 0x0d || key == 0x0a) {
      DBGU_puts("\n\r");
      break;
    }

    /* DEL or BACKSPACE */
    if(key == 0x7f || key == 0x08) {
      
      if(i>0) {
        /* end of time[], index then one more back */
        if(!time[i])
          --i;
        
        DBGU_puts(pEraseSeq);
        --i;
        
        /* delimitor ':' for time is uneditable */
        if(!IsDigitChar(time[i]) && i>0) {
          DBGU_puts(pEraseSeq);
          --i;
        }
      }
    }
    
    /* end of time[], no more input except above DEL/BS or enter to end */
    if(!time[i]) {
      continue;
    }
    
    if(!IsDigitChar(key)) {
      continue;
    }
   
    DBGU_PutChar(key);
    time[i++] = key;
    
    /* ignore non digi position if not end*/
    if(!IsDigitChar(time[i]) && i < 8) {
      DBGU_PutChar(time[i]);
      ++i;
    }
  }
  
  if(i == 0) {
    return 0;
  }
  
  if(i != 0 && time[i] != '\0') {
    return 1; /* failure input */
  }
  
  newHour = CharToDigit(time[0]) * 10 + CharToDigit(time[1]);
  newMinute = CharToDigit(time[3]) * 10 + CharToDigit(time[4]);
  newSecond = CharToDigit(time[6]) * 10 + CharToDigit(time[7]);
  
  /* success input. verification of data is left to RTC internal Error Checking */
  return 0;       
}

//------------------------------------------------------------------------------
/// Calculate week from year, month,day
//------------------------------------------------------------------------------
static char CalcWeek(int year, int month, int day)
{
  char week;
  
  if(month == 1 || month == 2) {
    month += 12;
    --year;
  }
  
  week = (day+2*month+3*(month+1)/5+year+year/4-year/100+year/400)%7;
  
  ++week;
  
  return week;
}
//------------------------------------------------------------------------------
/// Get new time, successful value is put in newYear, newMonth, newDay, newWeek
//------------------------------------------------------------------------------
static int GetNewDate()
{
  char key;
  int i=0;
  
  /* clear setting variable */
  newYear = 0xFFFF;
  newMonth = newDay= newWeek = 0xFF;
  
  /* use time[] as a format template */
  while(1) {
    
    key = DBGU_GetChar();
    
    /* end input */
    if(key == 0x0d || key == 0x0a) {
      DBGU_puts("\n\r");
      break;
    }

    /* DEL or BACKSPACE */
    if(key == 0x7f || key == 0x08) {
      
      if(i>0) {
        /* end of date[], index then one more back */
        if(!date[i])
          --i;
        
        DBGU_puts(pEraseSeq);
        --i;
        
        /* delimitor '/' for date is uneditable */
        if(!IsDigitChar(date[i]) && i>0) {
          DBGU_puts(pEraseSeq);
          --i;
        }
      }
    }
    
    /* end of time[], no more input except above DEL/BS or enter to end */
    if(!date[i]) {
      continue;
    }
    
    if(!IsDigitChar(key)) {
      continue;
    }
   
    DBGU_PutChar(key);
    date[i++] = key;
    
    /* ignore non digi position */
    if(!IsDigitChar(date[i]) && i < 10) {
      DBGU_PutChar(date[i]);
      ++i;
    }
  }
  
  if(i == 0) {
    return 0;
  }
  
  if(i != 0 && date[i] != '\0' && i !=6) {
    return 1; /* failure input */
  }
  //MM-DD-YY, static char date[10+1] = {'0', '0', '/', '0', '0', '/', '0', '0', '0', '0', '\0'};
  newMonth = CharToDigit(date[0]) * 10 + CharToDigit(date[1]);
  newDay = CharToDigit(date[3]) * 10 + CharToDigit(date[4]);
  if(i != 6) {/* not scenario of getting mm/dd/ only for alarm */
    newYear = CharToDigit(date[6]) * 1000 + CharToDigit(date[7]) * 100 + \
                  CharToDigit(date[8]) * 10 + CharToDigit(date[9]);
    newWeek = CalcWeek(newYear, newMonth, newDay);
  }
  /* success input. verification of data is left to RTC internal Error Checking */
  return 0;       
}


//------------------------------------------------------------------------------
/// Displays the user interface on the DBGU.
//------------------------------------------------------------------------------
static void RefreshDisplay(void)
{

    unsigned char hour, minute, second;
    unsigned short year;
    unsigned char month, day, week;

    if(bState != STATE_MENU) { //not in menu display mode, in set mode

    } else {
      // Retrieve date and time
      RTC_GetTime(&hour, &minute, &second);
      RTC_GetDate(&year, &month, &day, &week);
      
      //display
      if(!bMenuShown ) { 
        printf("\n\rMenu:\n\r");
        printf("  t - Set time\n\r");
        printf("  d - Set date\n\r");
        printf("  i - Set time alarm\n\r");
        printf("  m - Set date alarm\n\r");
        
        if (alarmTriggered) {
            printf("  c - Clear alarm notification\n\r");
        }
        
        printf("  q - Quit!\n\r");
        
        printf("\n\r");
        
        bMenuShown = 1;
      }
      
      /* update current date and time*/
      PrnToBuf(time, "%02d:%02d:%02d",hour,minute,second);
      PrnToBuf(date, "%02d/%02d/%04d",month,day,year);
      PrnToBuf(calendar, " [Time/Date: %s, %s %s ][Alarm status:%s]", time, date, pDayNames[week-1],\
        alarmTriggered?"Triggered!":"");
      
      printf("\r%s", calendar);
    }
}

//------------------------------------------------------------------------------
/// Interrupt handler for the RTC. Refreshes the display.
//------------------------------------------------------------------------------
void RTC_IrqHandler(void)
{
    unsigned int status = AT91C_BASE_RTC->RTC_SR;

    // Second increment interrupt
    if ((status & AT91C_RTC_SECEV) == AT91C_RTC_SECEV) {
        // Disable RTC interrupt
        RTC_DisableIt(AT91C_RTC_SECEV);
        //AIC_DisableIT(AT91C_ID_SYS);
        
        RefreshDisplay();
        
        AT91C_BASE_RTC->RTC_SCCR = AT91C_RTC_SECEV;
        
        RTC_EnableIt(AT91C_RTC_SECEV);
        //AIC_EnableIT(AT91C_ID_SYS);
    }
    // Time or date alarm
    else if ((status & AT91C_RTC_ALARM) == AT91C_RTC_ALARM) {
        // Disable RTC interrupt
        RTC_DisableIt(AT91C_RTC_ALARM);
        //AIC_DisableIT(AT91C_ID_SYS);
        
        alarmTriggered = 1;
        RefreshDisplay();
        bMenuShown = 0;// shown additional menu item for clear notification
        AT91C_BASE_RTC->RTC_SCCR = AT91C_RTC_ALARM;
        
        //AIC_EnableIT(AT91C_ID_SYS);
        RTC_EnableIt(AT91C_RTC_ALARM);
    }
}

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Default main() function. Initializes the DBGU and writes a string on the
/// DBGU.
//------------------------------------------------------------------------------
int main(void)
{
    unsigned char key;
//    unsigned char hour,minute,second;

    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("\n\r\n\r\n\r");//create display distance from history
    printf("-- Basic RTC Project %s --\n\r", SOFTPACK_VERSION);
    printf("-- %s\n\r", BOARD_NAME);
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
    
//    RTC_SetHourMode(1);//12 hour mode
//    hour = minute = second = 13;
//    printf("\n\r Seting 13:13:13 %s", RTC_SetTime(hour, minute, second)? "fail": "OK");
//    hour = minute = second = 22;
//    printf("\n\r Setting 22:22:22 %s", RTC_SetTime(hour, minute, second)? "fail": "OK");
//    
//    RTC_GetTime(&hour, &minute, &second);
//    printf("\n\r Now time is %02d:%02d:%02d",hour, minute, second);
//    
//    printf("\n\r Press any key to set time");
//    key=DBGU_GetChar();
//    printf("\n\r Setting %s", RTC_SetTime(hour, minute, second)? "fail": "OK");
    
#if defined(at91sam7l64) || defined(at91sam7l128)
    // Enable RTC in the supply controller
    AT91C_BASE_SUPC->SUPC_MR |= (0xA5 << 24) | AT91C_SUPC_RTCON;
    while ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_RTS) != AT91C_SUPC_RTS);
#endif

    // Default RTC configuration
    RTC_SetHourMode(0); // 24-hour mode
    if(RTC_SetTimeAlarm(0, 0, 0))
      printf("\n\r Disable time alarm fail!");
    
    if(RTC_SetDateAlarm(0, 0))
      printf("\n\r Disable date alarm fail!");

    // Configure RTC interrupts
    IRQ_ConfigureIT(BOARD_RTC_ID, 0, RTC_IrqHandler);
    RTC_EnableIt(AT91C_RTC_SECEV | AT91C_RTC_ALARM);
    IRQ_EnableIT(BOARD_RTC_ID);

    // Refresh display once
    RefreshDisplay();

    // Handle keypresses
    while (1) {

      key = DBGU_GetChar();
      
      /* set time */
      if(key == 't') {
        bState = STATE_SET_TIME;
        
        do {
          printf("\n\r\n\r Set time(hh:mm:ss): ");
        } while(GetNewTime());
        
        // if valid input, none of variable for time is 0xff
        if(newHour != 0xFF) {
          if(RTC_SetTime(newHour, newMinute, newSecond)) {
            printf("\n\r Time not set, invalid input!\n\r");
          }
        }
        
          bState = STATE_MENU;
          bMenuShown = 0;
          RefreshDisplay();
      }
      
      
      /* set date */
      if(key == 'd') {
        bState = STATE_SET_DATE;
        
        do {
          printf("\n\r\n\r Set date(mm/dd/yyyy): ");
        }while(GetNewDate());
        
        //if valid input, none of variable for date is 0xff(ff)
        if(newYear !=0xFFFF) {
          if(RTC_SetDate(newYear, newMonth, newDay, newWeek)) {
            printf("\n\r Date not set, invalid input!\n\r");
          }
        }
        
        //only 'mm/dd' inputed
        if(newMonth != 0xFF && newYear == 0xFFFF) {
          printf("\n\r Not Set for no year field!\n\r");
        }
        
        bState = STATE_MENU;
        bMenuShown = 0;
        RefreshDisplay();
      }
      
      
      /*set time alarm */
      if(key == 'i') {
        bState = STATE_SET_TIME_ALARM;
        
        do {
          printf("\n\r\n\r Set time alarm(hh:mm:ss): ");
        }while(GetNewTime());
        
        if(newHour != 0xFF) {
          if(RTC_SetTimeAlarm(&newHour, &newMinute, &newSecond)) {
            printf("\n\r Time alarm not set, invalid input!\n\r");
          } else {
            printf("\n\r Time alarm is set at %02d:%02d:%02d!", newHour,newMinute,newSecond);
          }
        }
        bState = STATE_MENU;
        bMenuShown = 0;
        alarmTriggered = 0;
        RefreshDisplay();        
      }
      
      /* set date alarm */
      if(key == 'm') {
        bState = STATE_SET_DATE_ALARM;
        
        do {
          printf("\n\r\n\r Set date alarm(mm/dd/): ");
        }while(GetNewDate());
        
        if(newYear != 0xFFFF && newMonth != 0xFF) {
          if(RTC_SetDateAlarm(&newMonth, &newDay)) {
            printf("\n\r Date alarm not set, invalid input!\n\r");
          } else {
            printf("\n\r Date alarm is set on %02d/%02d!",newMonth, newDay);
          }
          
        }
        bState = STATE_MENU;
        bMenuShown = 0;
        alarmTriggered = 0;
        RefreshDisplay();
      }
      
      /* clear trigger flag */
      if(key == 'c') {
          alarmTriggered = 0;
          bMenuShown = 0;
          RefreshDisplay();
      }
      
      /* quit */
      if(key == 'q') {
        break;
      }

    }
    
    return 0;
}

