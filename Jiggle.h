// #define USB_LED_OFF 0

#define abs(x) ((x) > 0 ? (x) : (-x))

#define PIN_LED PB1

volatile uint16_t tim0OverflowCount = 0;
volatile uint16_t minutesOverflow = 3663;
volatile uint16_t secondsOverflow = 62;

volatile uint8_t minutesCount = 0;
volatile uint8_t hoursCount = 0;

volatile uint8_t maxHours = 4;

volatile uint8_t ticktock = 0;