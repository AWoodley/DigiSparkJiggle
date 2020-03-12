/* Name: main.c
 * Project: DigiSparkJiggle; a mouse jiggler for a DigiSpark
 * Author: Adrian Woodley
 * Creation Date: 12-03-2020
 * Tabsize: 4
 * License: GNU GPL v3
 */

#include <avr/io.h>
#include <avr/interrupt.h>  /* for sei() */
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "Jiggle.h"

#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

PROGMEM const char usbHidReportDescriptor[] = { /* USB report descriptor, size must match usbconfig.h */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xA1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM
    0x29, 0x03,                    //     USAGE_MAXIMUM
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Const,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xC0,                          //   END_COLLECTION
    0xC0,                          // END COLLECTION
};

/* This is the same report descriptor as seen in a Logitech mouse. The data
 * described by this descriptor consists of 4 bytes:
 *      .  .  .  .  . B2 B1 B0 .... one byte with mouse button states
 *     X7 X6 X5 X4 X3 X2 X1 X0 .... 8 bit signed relative coordinate x
 *     Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 .... 8 bit signed relative coordinate y
 *     W7 W6 W5 W4 W3 W2 W1 W0 .... 8 bit signed relative coordinate wheel
 */
typedef struct{
    uchar   buttonMask;
    char    dx;
    char    dy;
    char    dWheel;
}report_t;

static report_t reportBuffer;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

ISR(TIMER0_OVF_vect)
{   
    if (tim0OverflowCount % secondsOverflow == 0)
    {
        PORTB ^= (1 << PIN_LED);
    }
    if (tim0OverflowCount == minutesOverflow)
    {
        reportBuffer.dx = 1;
    }
    else if (tim0OverflowCount > minutesOverflow)
    {
        reportBuffer.dx = -1;
        tim0OverflowCount = 0;
        minutesCount++;
    }
    else
    {
        reportBuffer.dx = 0; 
    }

    if (minutesCount >= 60) {
        hoursCount++;
        minutesCount = 0;
    }

    if (hoursCount >= maxHours) {
        PORTB |= (1 << PIN_LED);
        usbDeviceDisconnect();
        cli();
        sleep_enable();
        sleep_cpu();
    }

    if(usbInterruptIsReady())
    {
        usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
    }
    tim0OverflowCount++;
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
    cli();
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

    DDRB |= (1 << PIN_LED);  //LED Pin as output

	/* Hardware Initialization */
    reportBuffer.dx = 0; //centre mouse
    reportBuffer.dy = 0;
    reportBuffer.dWheel = 0;
    reportBuffer.buttonMask = 0;

    usbInit();
    /* enforce re-enumeration, do this while interrupts are disabled! */
    usbDeviceDisconnect();  
    for (int i = 0; i<250; i++)
    {             /* fake USB disconnect for > 250 ms */
        _delay_ms(2);
    }
    usbDeviceConnect();

    // TCCR0A = (1<<WGM01); // Clear time on compare
    TCCR0B = (1<<CS02)|(1<<CS00);  // /1024 prescaler 

    TCNT0 = 0; // initialise counter

    // OCR0A = 241;

    TIMSK |= (1<<TOIE0); // TIMER0 overflow interrupt

    tim0OverflowCount = 0; // reset overflow count

    sei();
    // wdt_enable(WDTO_1S);
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (void *)data;

    /* The following requests are never used. But since they are required by
     * the specification, we implement them in this example.
     */
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            usbMsgPtr = (void *)&reportBuffer;
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

// Called by V-USB after device reset
void hadUsbReset()
{
    cli();
    int frameLength, targetLength = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
    int bestDeviation = 9999;
    uchar trialCal, bestCal, step, region;

    bestCal = OSCCAL;

    // do a binary search in regions 0-127 and 128-255 to get optimum OSCCAL
    for(region = 0; region <= 1; region++) {
        frameLength = 0;
        trialCal = (region == 0) ? 0 : 128;
        
        for(step = 64; step > 0; step >>= 1) { 
            if(frameLength < targetLength) // true for initial iteration
                trialCal += step; // frequency too low
            else
                trialCal -= step; // frequency too high
                
            OSCCAL = trialCal;
            frameLength = usbMeasureFrameLength();
            
            if(abs(frameLength-targetLength) < bestDeviation) {
                bestCal = trialCal; // new optimum found
                bestDeviation = abs(frameLength -targetLength);
            }
        }
    }

    OSCCAL = bestCal;
    sei();
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    SetupHardware();

    for(;;)
    {
        if (USBIN&USBMASK)
        {
            sleep_cpu();	// sleep, except at SE0, until SOF
        }
        
        usbPoll();
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
