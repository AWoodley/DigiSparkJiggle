/* Name: Jiggle.h
 * Project: DigiSparkJiggle; a mouse jiggler for a DigiSpark
 * Author: Adrian Woodley
 * Creation Date: 12-03-2020
 * Tabsize: 4
 * License: GNU GPL v3
 */

#define abs(x) ((x) > 0 ? (x) : (-x))

#define PIN_LED PB1

volatile uint16_t tim0OverflowCount = 0;
volatile uint16_t minutesOverflow = 3663;
volatile uint16_t secondsOverflow = 62;

volatile uint8_t minutesCount = 0;
volatile uint8_t hoursCount = 0;

volatile uint8_t maxHours = 4;

volatile uint8_t ticktock = 0;