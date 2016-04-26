#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <TinyWireM.h>
#include <LiquidCrystal_attiny.h>


#define LED 3
// #define LED2 1
#define BUTTON 4
#define SIGNAL_PIN 1

#define GPIO_ADDR     0x27
LiquidCrystal_I2C lcd(GPIO_ADDR, 16, 2); // set address & 16 chars / 2 lines


#define set(x) |= (1<<x)
#define clr(x) &=~(1<<x) 
#define inv(x) ^=(1<<x)
#define chkBit(x) & (1<<(x))


volatile uint32_t mills;
volatile uint32_t pOptick = 0;
volatile uint8_t pOpPulse = 0;
volatile uint16_t pOpBpmValue = 261;
volatile uint32_t buttonOffTill = 0;
volatile uint32_t lastClickTime;
volatile uint8_t consecutiveClicks = 0;



void setupTimers() {

  // timer 0 - used for ticks...
  TCCR0A set(WGM01); // CTC mode. set WGM01
  
  // divide by 64
  TCCR0B set(CS00);
  TCCR0B set(CS01);
  TCCR0B clr(CS02);
  
  OCR0A = 125; // 8,000,000 / 64 / 125 = 1ms
  TIMSK set(OCIE0A); // set timer to interrupt

  // calibration...
  // OSCCAL = 0x00; // lowest...
  OSCCAL -= 1;
  // OSCCAL = 0x7f; // highest?...

}

void setupButton() {
  DDRB clr(BUTTON); // button is input...
  PORTB set(BUTTON); // turn on pull-up...
  PCMSK set(BUTTON); // turn interrupt on...
  GIMSK set(PCIE);
}

void setupSignal() {
  DDRB set(SIGNAL_PIN);  
}

void setupLed() {
  // led is output...
  DDRB set(LED);
  // DDRB set(LED2);
}

void setup(void) {
  // stop interrupts...
  cli();
  
  setupLed();
  setupTimers();
  setupButton();

  // start interrupts...
  sei();
  
  lcd.init();                           // initialize the lcd
  lcd.backlight();                      // Print a message to the LCD.
  lcd.print("Starting up...");
  

}

void loop() {
  lcd.clear();
  for(;;) {
    lcd.setCursor(0, 0);
    lcd.print("taps: ");
    lcd.print(consecutiveClicks);
    lcd.print(" - ");
    lcd.print(OSCCAL);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("bpm: ");
    lcd.print(60000 / (pOpBpmValue * 2));
    lcd.print("   ");
    
  }
}

int main() {
  setup();
  loop();
}

ISR (TIMER0_COMPA_vect) {
  PORTB inv(LED);
  mills++;
  pOptick++;
  // timer for Pocket Operator synce signal
  if (pOptick >= pOpBpmValue) {
    pOptick = 0;
    pOpPulse = 5; // length of Pocket Operator sync signal - just something that works....
    PORTB set(SIGNAL_PIN);
  }
  // timer for length of signal 
  if (pOpPulse == 0) {
    PORTB clr(SIGNAL_PIN);
  }
  else {
    pOpPulse--;
  }
  
  // timer for button debounce...
  if (buttonOffTill < mills) {
    PCMSK set(BUTTON); // turn the button interrupt on after debounce...
    buttonOffTill = 0;
  }
  
  // timer for wait period between bpm clicks - wait (about?) 2 seconds and reset bpm clicker
  if (mills > lastClickTime + 2000) {
    consecutiveClicks = 0;
    // PORTB set(LED);
  }
}

volatile uint32_t firstClickTime;
volatile uint16_t totalTime;

// // PIN 2 is INT0
// ISR (INT0_vect) {
//   PORTB clr(LED);
//   lastClickTime = mills;
//   if (consecutiveClicks > 0) {
//     totalTime = mills - firstClickTime;
//     pOpBpmValue = round((totalTime / consecutiveClicks) / 2); // Pocket Operator seems to sync on 2 beats per measure...
//   }
//   else {
//     firstClickTime = mills;
//   }
//   consecutiveClicks++;
//   GIMSK clr(INT0); // turn the button interrupt off for debounce...
//   buttonOffTill = mills + 200; // time to debounce...
// }

// activates on both rising and falling edge...
ISR (PCINT0_vect) {
  PCMSK clr(BUTTON);
  if( PINB chkBit(BUTTON) ) {
    // PORTB clr(LED);
    lastClickTime = mills;
    if (consecutiveClicks > 0) {
      totalTime = mills - firstClickTime;
      pOpBpmValue = (totalTime / (consecutiveClicks)) / 2; // Pocket Operator seems to sync on 2 beats per measure...
    }
    else {
      firstClickTime = mills;
    }
    consecutiveClicks++;
    GIMSK clr(INT0); // turn the button interrupt off for debounce...
    buttonOffTill = mills + 180; // time to debounce...
  }
  else {
    return;
  }
}





